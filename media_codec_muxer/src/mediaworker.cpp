#include "mediaworker.h"
#include "audiocapturer.h"
#include "dlog.h"
#include <functional>
#include <memory>

// Audio Capturing settings
constexpr int kAudioCaptureSampleRate = 44100;
constexpr int kAudioCaptureChannelCount = 2;
constexpr int kAudioCaptureSampleFormat = auc::SampleFormat::PCM16Bit;

MediaWorker::~MediaWorker() {}

bool MediaWorker::Init(const std::string &output_file_name) {
  LogInfo("MediaWorker::Init() begin");
  // Todo: Init VideoCapturer
  // Todo: Init VideoEncoder

  // Init AudioCapturer
  audio_capturer_ = std::make_unique<auc::AudioCapturer>();
  audio_capturer_->Init(kAudioCaptureSampleRate, kAudioCaptureChannelCount,
                        kAudioCaptureSampleFormat);
  audio_capturer_->AddCallback(
      std::bind(&MediaWorker::PcmCallback, this, std::placeholders::_1));

  // Init AudioEncoder
  audio_encoder_ = std::make_unique<aue::AudioEncoder>();
  auto audio_input_format = std::make_shared<MediaFormat>(
      MediaFormat::AudioFormat::RAW_FORMAT_PCM, kAudioCaptureSampleRate,
      kAudioCaptureChannelCount);
  audio_encoder_->Init(audio_input_format);
  audio_encoder_->AddCallback(std::bind(&MediaWorker::EncodedAudioCallback,
                                        this, std::placeholders::_1));

  // Init MediaMuxer
  media_muxer_ = std::make_unique<mm::MediaMuxer>(mm::MediaMuxer::Format::MP4,
                                                  output_file_name);
  // Todo: video_track_index_ =
  // mediaMuxer.addTrack(video_encoder_.GetOutputFormat());
  audio_track_index_ =
      media_muxer_->AddTrack(audio_encoder_->GetCodecContext());
  return true;
}

bool MediaWorker::Start() {
  // Todo: video_capturer_->Start();
  if (!audio_capturer_->Start()) {
    LogError("MediaWorker::Start(), audio_capturer_->Start() failed");
    return false;
  }

  return Worker::Start();
}

/*
 * No need to stop VideoEncoder, AudioEncoder and MediaMuxer here since
 * stopping capturer will fire the end state to the corresponding callback
 * and then stop the encoder and muxer.
 */
void MediaWorker::Stop() {
  // Todo: video_capturer_->Stop();
  audio_capturer_->Stop();
  Worker::Stop();
}

void MediaWorker::Work() {
  LogInfo("MediaWorker::Work() begin");
  auto count = 0;
  while (true) {
    {
      std::lock_guard<std::mutex> lock(is_running_mutex_);
      if (!is_running_) {
        LogInfo("MediaWorker::Work break!");
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    count++;
    LogInfo("MediaWorker::Work in progress, %d seconds..", count);
  }
  LogInfo("MediaWorker::Work() end");
}

/*
 * callback from AudioCapturer, callback one frame pcm data with
 * nb_samples * channels * byte_per_sample.
 * Pass pcm with null means capture ended, encoder should stop its
 * process.
 */
void MediaWorker::PcmCallback(RawDataBufferInfo &raw_data_buffer_info) {
  if (raw_data_buffer_info.state == RawDataState::RAW_DATA_STATE_BEGIN) {
    LogDebug("MediaWorker::PcmCallback(..) called: begin to capture audio..");
    if (!audio_encoder_->Start()) {
      LogError("MediaWorker::PcmCallback(), audio_encoder_->Start() failed");
    }
  } else if (raw_data_buffer_info.state ==
             RawDataState::RAW_DATA_STATE_SENDING) {
    LogDebug("MediaWorker::PcmCallback(..) called: size:%d, %ld",
             raw_data_buffer_info.size, raw_data_buffer_info.time_stamp);
    audio_encoder_->QueueDataToEncode(raw_data_buffer_info);
  } else if (raw_data_buffer_info.state == RawDataState::RAW_DATA_STATE_END) {
    LogInfo("MediaWorker::PcmCallback(..) called: will call "
            "audio_encoder_->Stop()");
    audio_encoder_->Stop();
  } else if (raw_data_buffer_info.state == RawDataState::RAW_DATA_STATE_ERROR) {
    // Todo: send message to let mediaworker stop
    LogInfo("MediaWorker::PcmCallback(..) called: send message to let "
            "mediaworker stop");
  }
}

void MediaWorker::EncodedAudioCallback(
    EncodedDataBufferInfo &encoded_data_buffer_info) {
  LogDebug("MediaWorker::EncodedAudioCallback(..)");
  if (encoded_data_buffer_info.state == EncodedDataState::STATE_BEGIN) {
    LogDebug("MediaWorker::EncodedAudioCallback(..) called: begin to encode "
             "audio..");
    if (!media_muxer_->Start()) {
      LogError(
          "MediaWorker::EncodedAudioCallback(), media_muxer_->Start() failed");
    }
  } else if (encoded_data_buffer_info.state ==
             EncodedDataState::STATE_SENDING) {
    LogDebug("MediaWorker::EncodedAudioCallback(..) called: write encoded "
             "audio data to muxer..");
    media_muxer_->WriteSampleData(audio_track_index_,
                                  encoded_data_buffer_info.packet);
  } else if (encoded_data_buffer_info.state == EncodedDataState::STATE_END) {
    LogInfo("MediaWorker::EncodedAudioCallback(..) called: will call "
            "media_muxer_->Stop()");
    media_muxer_->Stop();
  } /*else if (encoded_data_buffer_info.state == EncodedDataState::STATE_ERROR)
  {
    // Todo: error handling
  }*/
}