#include "mediaworker.h"
#include "dlog.h"
#include <functional>
#include <memory>

// Video Capturing settings
/*constexpr int kVideoCaptureWidth = 854;
constexpr int kVideoCaptureHeight = 480;
constexpr int kVideoCaptureFps = 24;*/

const auto kVideoCaptureWidth = 1920;
const auto kVideoCaptureHeight = 1080;
const auto kVideoCaptureFps = 25;

// Audio Capturing settings
constexpr int kAudioCaptureSampleRate = 44100;
constexpr int kAudioCaptureChannelCount = 2;
constexpr int kAudioCaptureSampleFormat = SampleFormat::PCM16Bit;

MediaWorker::~MediaWorker() {}

bool MediaWorker::Init(const std::string &output_file_name) {
  LogInfo("MediaWorker::Init() begin");
  // Init Video Capture
  video_capturer_ = std::make_unique<DesktopCapturer>();
  video_capturer_->Init(kVideoCaptureWidth, kVideoCaptureHeight,
                        kVideoCaptureFps);
  video_capturer_->AddCallback(
      std::bind(&MediaWorker::YuvCallback, this, std::placeholders::_1));

  // Init VideoEncoder
  video_encoder_ = std::make_unique<VideoEncoder>();
  auto video_input_format = std::make_shared<MediaFormat>(
      MediaFormat::Format::RAW_VIDEO_YUV, kVideoCaptureWidth,
      kVideoCaptureHeight, kVideoCaptureFps);
  video_encoder_->Init(video_input_format);
  video_encoder_->AddCallback(std::bind(&MediaWorker::EncodedVideoCallback,
                                        this, std::placeholders::_1));
  // Init FakeAudioCapturer
  audio_capturer_ = std::make_unique<MicAudioCapturer>();
  audio_capturer_->Init(kAudioCaptureSampleRate, kAudioCaptureChannelCount,
                        kAudioCaptureSampleFormat);
  audio_capturer_->AddCallback(
      std::bind(&MediaWorker::PcmCallback, this, std::placeholders::_1));

  // Init AudioEncoder
  audio_encoder_ = std::make_unique<AudioEncoder>();
  auto audio_input_format = std::make_shared<MediaFormat>(
      MediaFormat::Format::RAW_AUDIO_PCM, kAudioCaptureSampleRate,
      kAudioCaptureChannelCount);
  audio_encoder_->Init(audio_input_format);
  audio_encoder_->AddCallback(std::bind(&MediaWorker::EncodedAudioCallback,
                                        this, std::placeholders::_1));

  // Init MediaMuxer
  media_muxer_ =
      std::make_unique<MediaMuxer>(MediaMuxer::Format::MP4, output_file_name);
  video_track_index_ =
      media_muxer_->AddTrack(video_encoder_->GetCodecContext());
  audio_track_index_ =
      media_muxer_->AddTrack(audio_encoder_->GetCodecContext());
  return true;
}

bool MediaWorker::Start() {
  if (!media_muxer_->Start()) {
    LogError("MediaWorker::Start() called, media_muxer_->Start() "
             "failed");
    return false;
  }
  if (!video_capturer_->Start()) {
    LogError("MediaWorker::Start(), video_capturer_->Start() failed");
    return false;
  }
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
  media_muxer_->Stop();
  video_capturer_->Stop();
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

void MediaWorker::YuvCallback(RawDataBufferInfo &raw_data_buffer_info) {
  if (raw_data_buffer_info.state == RawDataState::RAW_DATA_STATE_BEGIN) {
    LogDebug("MediaWorker::YuvCallback(..) called: begin to capture video..");
    if (!video_encoder_->Start()) {
      LogError("MediaWorker::YuvCallback(), video_encoder_->Start() failed");
    }
  } else if (raw_data_buffer_info.state ==
             RawDataState::RAW_DATA_STATE_SENDING) {
    LogDebug("MediaWorker::YuvCallback(..) called: size:%d, %ld",
             raw_data_buffer_info.size, raw_data_buffer_info.time_stamp);
    video_encoder_->QueueDataToEncode(raw_data_buffer_info);
  } else if (raw_data_buffer_info.state == RawDataState::RAW_DATA_STATE_END) {
    LogInfo("MediaWorker::YuvCallback(..) called: will call "
            "video_encoder_->Stop()");
    video_encoder_->Stop();
  } else if (raw_data_buffer_info.state == RawDataState::RAW_DATA_STATE_ERROR) {
    // Todo: send message to let mediaworker stop
    LogInfo("MediaWorker::YuvCallback(..) called: send message to let "
            "mediaworker stop");
  }
}

/*
 * callback from FakeAudioCapturer, callback one frame pcm data with
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

void MediaWorker::EncodedVideoCallback(
    EncodedDataBufferInfo &encoded_data_buffer_info) {
  LogDebug("MediaWorker::EncodedVideoCallback(..)");
  if (encoded_data_buffer_info.state == EncodedDataState::STATE_BEGIN) {
    LogInfo("MediaWorker::EncodedVideoCallback(..) called with "
            "EncodedDataState::STATE_BEGIN");
  } else if (encoded_data_buffer_info.state ==
             EncodedDataState::STATE_SENDING) {
    LogDebug("MediaWorker::EncodedVideoCallback(..) called: write encoded "
             "audio data to muxer..");
    media_muxer_->WriteSampleData(video_track_index_,
                                  encoded_data_buffer_info.packet);
  } else if (encoded_data_buffer_info.state == EncodedDataState::STATE_END) {
    LogInfo("MediaWorker::EncodedVideoCallback(..) called with "
            "EncodedDataState::STATE_END");
  } /*else if (encoded_data_buffer_info.state == EncodedDataState::STATE_ERROR)
  {
    // Todo: error handling
  }*/
}

void MediaWorker::EncodedAudioCallback(
    EncodedDataBufferInfo &encoded_data_buffer_info) {
  LogDebug("MediaWorker::EncodedAudioCallback(..)");
  if (encoded_data_buffer_info.state == EncodedDataState::STATE_BEGIN) {
    LogInfo("MediaWorker::EncodedAudioCallback(..) called with "
            "EncodedDataState::STATE_BEGIN");
  } else if (encoded_data_buffer_info.state ==
             EncodedDataState::STATE_SENDING) {
    LogDebug("MediaWorker::EncodedAudioCallback(..) called: write encoded "
             "audio data to muxer..");
    media_muxer_->WriteSampleData(audio_track_index_,
                                  encoded_data_buffer_info.packet);
  } else if (encoded_data_buffer_info.state == EncodedDataState::STATE_END) {
    LogInfo("MediaWorker::EncodedAudioCallback(..) called with "
            "EncodedDataState::STATE_END");
  } /*else if (encoded_data_buffer_info.state == EncodedDataState::STATE_ERROR)
  {
    // Todo: error handling
  }*/
}