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
      std::bind(&MediaWorker::PcmCallback, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));

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
  if (!media_muxer_->Start()) {
    LogError("MediaWorker::Start(), media_muxer_->Start() failed");
    return false;
  }
  
  // Todo: video_encoder_->Start()
  if (!audio_encoder_->Start()) {
    LogError("MediaWorker::Start(), audio_encoder_->Start() failed");
    return false;
  }

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
void MediaWorker::PcmCallback(uint8_t *pcm, int32_t size, int64_t time_stamp) {
  LogInfo("MediaWorker::PcmCallback(..) called: size:%d, %ld", size,
          time_stamp);
  if (pcm != nullptr && size != -1 && time_stamp != -1) {
    audio_encoder_->QueueDataToEncode(pcm, size, time_stamp);
  } else {
    LogInfo("MediaWorker::PcmCallback(..) called: will call "
            "audio_encoder_->Stop()");
    audio_encoder_->Stop();
    media_muxer_->Stop();
  }
}

void MediaWorker::EncodedAudioCallback(AVPacket *audio_packet) {
  LogInfo("MediaWorker::EncodedAudioCallback(..)");
  media_muxer_->WriteSampleData(audio_track_index_, audio_packet);
}