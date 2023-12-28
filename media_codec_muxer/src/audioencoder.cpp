#include "audioencoder.h"
#include "dlog.h"
#include "timesutil.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace aue {

AudioEncoder::AudioEncoder()
    : codec_ctx_(nullptr), resample_fltp_buf_(nullptr), av_frame_(nullptr),
      av_packet_(nullptr), is_consumer_running_(false) {}

AudioEncoder::~AudioEncoder() {
  if (codec_ctx_) {
    avcodec_free_context(&codec_ctx_);
  }
  if (resample_fltp_buf_ != nullptr) {
    av_freep(&resample_fltp_buf_);
  }
  if (av_frame_ != nullptr) {
    av_frame_free(&av_frame_);
  }
  if (av_packet_ != nullptr) {
    av_packet_free(&av_packet_);
  }

  while (!raw_data_queue_.empty()) {
    raw_data_queue_.pop();
  }
}

bool AudioEncoder::Init(std::shared_ptr<MediaFormat> input_format) {
  input_format_ = input_format;

  if (input_format_->GetFormat() != MediaFormat::AudioFormat::RAW_FORMAT_PCM) {
    LogInfo(
        "AudioEncoder::Init, given unsupported audio input or output format ");
    return false;
  }

  // Initialize AVCodecContext
  AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
  if (!codec) {
    LogInfo("AudioEncoder::Init, avcodec_find_encoder AV_CODEC_ID_AAC failed");
    return false;
  }
  codec_ctx_ = avcodec_alloc_context3(codec);
  if (!codec_ctx_) {
    LogInfo("avcodec_alloc_context3 AV_CODEC_ID_AAC failed");
    return false;
  }
  codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  codec_ctx_->sample_rate = input_format_->GetSampleRate();
  codec_ctx_->sample_fmt =
      AV_SAMPLE_FMT_FLTP; // Need to covert pcm raw from s16 to fltp first
  codec_ctx_->channels = input_format_->GetChannels();
  codec_ctx_->channel_layout =
      av_get_default_channel_layout(codec_ctx_->channels);
  auto bit_rate = 8 * av_get_bytes_per_sample(codec_ctx_->sample_fmt) *
                  codec_ctx_->channels * codec_ctx_->sample_rate;
  codec_ctx_->bit_rate = bit_rate;
  int ret = avcodec_open2(codec_ctx_, NULL, NULL);
  if (ret != 0) {
    avcodec_free_context(&codec_ctx_);
    LogInfo("avcodec_open2 failed");
    return false;
  }

  // Initialize AVFrame
  av_frame_ = av_frame_alloc();
  av_frame_->format =
      AV_SAMPLE_FMT_FLTP; // Need to covert pcm raw from s16 to fltp first
  av_frame_->nb_samples = codec_ctx_->frame_size;
  av_frame_->channels = codec_ctx_->channels;
  av_frame_->channel_layout =
      av_get_default_channel_layout(av_frame_->channels);
  if (av_frame_get_buffer(av_frame_, 0) < 0) {
    avcodec_free_context(&codec_ctx_);
    LogError("av_frame_get_buffer failed");
    return false;
  }

  // Initialize resample_fltp_buf_
  if (!resample_fltp_buf_) {
    // ex: channel =2, flt format, nb_samples = 1024 => supposed to be 4*2*1024
    // = 8192 for this case
    int resample_fltp_buf_size = av_samples_get_buffer_size(
        NULL, av_frame_->channels, av_frame_->nb_samples,
        (enum AVSampleFormat)av_frame_->format, 1);
    resample_fltp_buf_ = (uint8_t *)av_malloc(resample_fltp_buf_size);
  }

  return true;
}

bool AudioEncoder::Start() {
  LogInfo("AudioEncoder::Start called");
  {
    std::lock_guard<std::mutex> lock(raw_data_queue_mutex_);
    if (is_consumer_running_) {
      LogInfo("AudioEncoder::Start(), already running");
      return false;
    }
    is_consumer_running_ = true;
  }
  auto worker_fun = [this] {
    { Work(); }
  };
  consumer_thread_ = std::make_unique<std::thread>(worker_fun);
  return true;
}

void AudioEncoder::consume_queue(
    std::queue<RawDataBufferInfo> &temp_raw_data_queue) {
  while (!temp_raw_data_queue.empty()) {
    auto raw_data_buffer_info = std::move(temp_raw_data_queue.front());
    temp_raw_data_queue.pop();
    LogDebug("AudioEncoder::consume_queue, cur raw_data_buffer_info.time_stamp: "
            "%ld, av_frame_->nb_samples: %d",
            raw_data_buffer_info.time_stamp, av_frame_->nb_samples);
    // resample raw_data_buffer_info, sl0, sr0, sl1, sr1 => fl0, fl1, fr0, fr1
    short *pcm_short = (short *)raw_data_buffer_info.raw_data.get();
    float *resample_fltp_buf_float = (float *)resample_fltp_buf_;
    float *fltp_l = resample_fltp_buf_float; // -1~1
    float *fltp_r = resample_fltp_buf_float + av_frame_->nb_samples;
    for (int idx = 0; idx < av_frame_->nb_samples; idx++) {
      fltp_l[idx] = pcm_short[idx * 2] / 32768.0f;
      fltp_r[idx] = pcm_short[idx * 2 + 1] / 32768.0f;
    }
    av_frame_make_writable(av_frame_);
    av_samples_fill_arrays(av_frame_->data, av_frame_->linesize,
                           resample_fltp_buf_, av_frame_->channels,
                           av_frame_->nb_samples,
                           (AVSampleFormat)av_frame_->format, 0);

    // encode raw_data_buffer_info
    av_frame_->pts =
        av_rescale_q(raw_data_buffer_info.time_stamp,
                     AVRational{1, TimesUtil::GetTimeBaseMillisecond()},
                     codec_ctx_->time_base);
    std::vector<AVPacket *> packets;
    int ret = avcodec_send_frame(codec_ctx_, av_frame_);
    if (ret != 0) {
      LogDebug("AudioEncoder::consume_queue, avcodec_send_frame failed!");
      continue;
    }
    while (true) {
      AVPacket *packet = av_packet_alloc();
      ret = avcodec_receive_packet(codec_ctx_, packet);
      // New input data is required to return new output.
      if (ret == AVERROR(EAGAIN)) {
        av_packet_free(&packet);
        LogDebug("AudioEncoder::consume_queue, avcodec_receive_packet need more "
                "input to encode, continue to send frame..");
        break;
      }
      // When in draining mode(send NULL to the avcodec_send_packet() (decoding)
      // or avcodec_send_frame() (encoding) functions. , no new output and
      // AVERROR_EOF will be sent.
      if (ret == AVERROR_EOF) {
        av_packet_free(&packet);
        LogDebug("AudioEncoder::consume_queue, the encoder is flushed without "
                "no extra output, leave the encoder process!");
        break;
      }
      packets.push_back(packet);
    }
    for(auto* packet: packets) {
      encoded_callback_(packet);
      av_packet_free(&packet);
    }
    packets.clear();
  }
}

void AudioEncoder::Work() {
  LogInfo("AudioEncoder::Work begins");
  while (true) {
    std::queue<RawDataBufferInfo> temp_raw_data_queue;
    {
      std::unique_lock<std::mutex> lock(raw_data_queue_mutex_);
      while (raw_data_queue_.empty() && is_consumer_running_) {
        LogDebug("AudioEncoder::Work, has no raw data to encode, wait..");
        raw_data_queue_cv_.wait(lock);
      }
      while (!raw_data_queue_.empty()) {
        temp_raw_data_queue.push(std::move(raw_data_queue_.front()));
        raw_data_queue_.pop();
      }
    }

    consume_queue(temp_raw_data_queue);

    if (!is_consumer_running_) {
      LogInfo("AudioEncoder::Work, is_consumer_running_ == false, break!");
      break;
    }
  }
  LogInfo("AudioEncoder::Work end");
}

bool AudioEncoder::Stop() {
  LogInfo("AudioEncoder::Stop called");
  {
    std::lock_guard<std::mutex> lock(raw_data_queue_mutex_);
    if (!is_consumer_running_) {
      LogInfo("AudioEncoder::Stop(), already stopped");
      return true;
    }
    is_consumer_running_ = false;
  }
  raw_data_queue_cv_.notify_one();
  consumer_thread_->join();
  LogInfo("AudioEncoder::Stop() ok");
  return true;
}

bool AudioEncoder::QueueDataToEncode(uint8_t *pcm, int32_t size,
                                     int64_t time_stamp) {
  LogDebug("AudioEncoder::QueueDataToEncode called");
  {
    std::lock_guard<std::mutex> lock(raw_data_queue_mutex_);
    if (!is_consumer_running_) {
      return false;
    }
    auto raw_data = std::make_unique<uint8_t[]>(size);
    std::copy(pcm, pcm + size, raw_data.get());
    raw_data_queue_.push({std::move(raw_data), size, time_stamp});
  }
  raw_data_queue_cv_.notify_one();
  return true;
}

} // namespace aue