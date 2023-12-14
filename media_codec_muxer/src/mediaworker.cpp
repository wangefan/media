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
  output_file_name_ = output_file_name;

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

  // Todo: Turn on VideoEncoder
  // Todo: Turn on AudioEncoder
  // Todo: Turn on Mixer

  return true;
}

bool MediaWorker::Start() {
  // Todo: assert(video_capturer_->Start());
  if (!audio_capturer_->Start()) {
    LogError("MediaWorker::Start(), audio_capturer_->Start() failed");
    return false;
  }
  return Worker::Start();
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

// callback from AudioCapturer, callback one frame pcm data with
// nb_samples * channels * byte_per_sample
void MediaWorker::PcmCallback(uint8_t *pcm, int32_t size, int64_t time_stamp) {
  LogInfo("MediaWorker::PcmCallback(..) called: size:%d, %ld", size, time_stamp);
  /*int64_t pts = (int64_t)AVPublishTime::GetInstance()->get_audio_pts();
  resampler_->convertToFlt(a_frame_, pcm, size);
  RET_CODE encode_ret = audio_encoder_->Encode(a_frame_, a_packet_, pts);
  if (encode_ret != RET_OK) {
    LogError("PushWorker::PcmCallback, audio_encoder_->Encode failed");
    return;
  }
  LogInfo(
      "PushWorker::PcmCallback(..) audio_encoder_->Encode ok\n");
  if (encode_ret == RET_OK) {
    dumpToAACFile(a_packet_);
  }*/
}