#include "audiocapturer.h"
#include "dlog.h"
#include "timesutil.h"
namespace auc {

AudioCapturer::~AudioCapturer() {}

bool AudioCapturer::Init(uint32_t sample_rate, uint32_t channel_count,
                         uint32_t sample_format) {
  LogInfo("AudioCapturer::Init(..) begin");
  sample_rate_ = sample_rate;
  channel_count_ = channel_count;
  sample_format_ = sample_format;
  return true;
}

void AudioCapturer::Work() {
  LogInfo("AudioCapturer::Work() begin");

  const std::string input_pcm_name = "./media_files/count_s.pcm";
  FILE *pcm_fp = fopen(input_pcm_name.c_str(), "rb");
  if (pcm_fp == nullptr) {
    LogInfo("AudioCapturer::Work(), open file error, end");
    if (pcm_callback_ != nullptr) {
      RawDataBufferInfo raw_data_buffer_info{RawDataState::RAW_DATA_STATE_ERROR,
                                             nullptr, -1, -1};
      pcm_callback_(raw_data_buffer_info);
    }
    return;
  }

  bool reset_to_head = true;
  const int32_t nb_samples = 1024;
  const int32_t byte_per_sample =
      2; // Todo: To see if sample_format_ is PCM16Bit.
  auto pcm_per_frame_size = nb_samples * byte_per_sample * channel_count_;
  uint8_t *pcm_per_frame = new uint8_t[pcm_per_frame_size];
  auto frame_duration = 1.0 * nb_samples / sample_rate_ *
                        (double)(TimesUtil::GetTimeBaseMicroSeconds()); // us
  int64_t pcm_record_start_time = TimesUtil::GetTimeMicroSeconds();
  int64_t pcm_frame_start_time = TimesUtil::GetTimeMicroSeconds();
  int64_t pcm_frame_dst_time = pcm_frame_start_time + frame_duration;
  if (pcm_callback_ != nullptr) {
    RawDataBufferInfo raw_data_buffer_info{RawDataState::RAW_DATA_STATE_BEGIN,
                                           nullptr, -1, -1};
    pcm_callback_(raw_data_buffer_info);
  }

  while (true) {
    {
      std::lock_guard<std::mutex> lock(is_running_mutex_);
      if (!is_running_) {
        LogInfo("AudioCapturer::Work break!");
        break;
      }
    }
    if (reset_to_head) {
      fseek(pcm_fp, 0, SEEK_SET);
      reset_to_head = false;
    }
    auto current_time = TimesUtil::GetTimeMicroSeconds();
    if (current_time < pcm_frame_dst_time) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    // read a frame pcm data from file.
    int32_t size = fread(pcm_per_frame, 1, pcm_per_frame_size, pcm_fp);
    if (size > 0) {
      if (pcm_callback_ != nullptr) {
        auto raw_data = std::make_unique<uint8_t[]>(size);
        std::copy(pcm_per_frame, pcm_per_frame + size, raw_data.get());
        RawDataBufferInfo raw_data_buffer_info{
            RawDataState::RAW_DATA_STATE_SENDING, std::move(raw_data), size,
            pcm_frame_start_time - pcm_record_start_time};
        pcm_callback_(raw_data_buffer_info);
      }
      pcm_frame_start_time = pcm_frame_dst_time;
      pcm_frame_dst_time += frame_duration;
    } else {
      LogInfo("AudioCapturer::Work(), capture with no data!");
      reset_to_head = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  fclose(pcm_fp);
  pcm_fp = nullptr;
  delete[] pcm_per_frame;
  pcm_per_frame = nullptr;
  if (pcm_callback_ != nullptr) {
    RawDataBufferInfo raw_data_buffer_info{RawDataState::RAW_DATA_STATE_END,
                                           nullptr, -1, -1};
    pcm_callback_(raw_data_buffer_info);
  }
  LogInfo("AudioCapturer::Work() end");
}

} // namespace auc
