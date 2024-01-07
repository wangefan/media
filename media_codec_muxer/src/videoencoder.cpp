#include "videoencoder.h"
#include "dlog.h"
#include "timesutil.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

#define VIDEO_BIT_RATE 500*1024

VideoEncoder::VideoEncoder() {}

VideoEncoder::~VideoEncoder() {}

bool VideoEncoder::Init(std::shared_ptr<MediaFormat> input_format) {
  input_format_ = input_format;

  if (input_format_->GetFormat() != MediaFormat::Format::RAW_VIDEO_YUV) {
    LogInfo(
        "VideoEncoder::Init, given unsupported video input or output format ");
    return false;
  }

  // Initialize AVCodecContext
  AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (!codec) {
    LogInfo("VideoEncoder::Init, avcodec_find_encoder AV_CODEC_ID_H264 failed");
    return false;
  }
  codec_ctx_ = avcodec_alloc_context3(codec);
  if (!codec_ctx_) {
    LogInfo("avcodec_alloc_context3 AV_CODEC_ID_H264 failed");
    return false;
  }
  codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  codec_ctx_->width = input_format_->GetWidth();
  codec_ctx_->height = input_format_->GetHeight();
  const auto fps = input_format_->GetFps();
  codec_ctx_->framerate = {fps, 1};
  codec_ctx_->bit_rate = VIDEO_BIT_RATE;
  codec_ctx_->time_base = {1, 1000000}; // 微妙
  codec_ctx_->gop_size = fps;
  codec_ctx_->max_b_frames = 0;
  codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;
  // av_dict_set(&dict_, "tune", "zerolatency", 0);
  // int ret = avcodec_open2(codec_ctx_, NULL, &dict_);
  int ret = avcodec_open2(codec_ctx_, nullptr, nullptr);
  if (ret != 0) {
    avcodec_free_context(&codec_ctx_);
    LogInfo("avcodec_open2 failed");
    return false;
  }

  // Initialize AVFrame
  av_frame_ = av_frame_alloc();
  av_frame_->format = codec_ctx_->pix_fmt;
  av_frame_->width = codec_ctx_->width;
  av_frame_->height = codec_ctx_->height;

  return true;
}

void VideoEncoder::consume_queue(
    std::queue<RawDataBufferInfo> &temp_raw_data_queue) {
  while (!temp_raw_data_queue.empty()) {
    auto raw_data_buffer_info = std::move(temp_raw_data_queue.front());
    temp_raw_data_queue.pop();

    int ret = 0;
    if (raw_data_buffer_info.raw_data.get() == nullptr) {
      LogDebug("VideoEncoder::consume_queue, raw_data_buffer_info is nullptr, "
               "flush the encoder");
      ret = avcodec_send_frame(codec_ctx_, nullptr);
    } else {
      LogDebug(
          "VideoEncoder::consume_queue, cur raw_data_buffer_info.time_stamp: "
          "%ld, av_frame_->nb_samples: %d",
          raw_data_buffer_info.time_stamp, av_frame_->nb_samples);
      av_image_fill_arrays(av_frame_->data, av_frame_->linesize,
                           raw_data_buffer_info.raw_data.get(),
                           (AVPixelFormat)av_frame_->format, av_frame_->width,
                           av_frame_->height, 1);
      // encode raw_data_buffer_info
      av_frame_->pts =
          av_rescale_q(raw_data_buffer_info.time_stamp,
                       AVRational{1, TimesUtil::GetTimeBaseMillisecond()},
                       codec_ctx_->time_base);
      ret = avcodec_send_frame(codec_ctx_, av_frame_);
    }

    if (ret != 0) {
      LogDebug("VideoEncoder::consume_queue, avcodec_send_frame failed!");
      continue;
    }

    // receive encoded data
    std::vector<AVPacket *> packets;
    while (true) {
      AVPacket *packet = av_packet_alloc();
      ret = avcodec_receive_packet(codec_ctx_, packet);
      // New input data is required to return new output.
      if (ret == AVERROR(EAGAIN)) {
        av_packet_free(&packet);
        LogDebug(
            "VideoEncoder::consume_queue, avcodec_receive_packet need more "
            "input to encode, continue to send frame..");
        break;
      }
      // When in draining mode(send NULL to the avcodec_send_packet() (decoding)
      // or avcodec_send_frame() (encoding) functions. , no new output and
      // AVERROR_EOF will be sent.
      if (ret == AVERROR_EOF) {
        av_packet_free(&packet);
        LogDebug("VideoEncoder::consume_queue, the encoder is flushed without "
                 "no extra output, leave the encoder process!");
        break;
      }
      packets.push_back(packet);
    }
    for (auto *packet : packets) {
      EncodedDataBufferInfo encoded_data_buffer_info{
          EncodedDataState::STATE_SENDING, packet};
      encoded_callback_(encoded_data_buffer_info);
      av_packet_free(&packet);
    }
    packets.clear();
  }
}