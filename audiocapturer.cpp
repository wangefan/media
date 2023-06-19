#include "audiocapturer.h"
#include "dlog.h"
#include "timesutil.h"

AudioCapturer::AudioCapturer() {
  LogInfo("AudioCapturer ctr called\n");
  pcm_start_time_ = 0;
  pcm_fp_ = nullptr;
  pcm_per_frame_size_ = 0;
  pcm_per_frame_ = nullptr;
  frame_duration_ = 23.2;
}

AudioCapturer::~AudioCapturer() {
  closePcmFile();
  pcm_per_frame_size_ = 0;
  if (pcm_per_frame_ != nullptr) {
    delete[] pcm_per_frame_;
    pcm_per_frame_ = nullptr;
  }
  frame_duration_ = 0.0;
}

void AudioCapturer::Work() {
  LogInfo("AudioCapturer::Work() called\n");
  bool reset_to_head = true;
  
  while (is_running_) {
    //LogInfo("AudioCapturer::Work() is running..\n");
    if(reset_to_head) {
      fseek(pcm_fp_, 0, SEEK_SET);
      pcm_start_time_ = TimesUtil::GetTimeMillisecond(); // begin time
      pcm_dst_time_ = pcm_start_time_ + frame_duration_; // dest time
      reset_to_head = false;
    }
    auto current_time = TimesUtil::GetTimeMillisecond();
    if (current_time < pcm_dst_time_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    // read a frame pcm data from file.
    int32_t size = fread(pcm_per_frame_, 1, pcm_per_frame_size_, pcm_fp_);
    if (size > 0) {
      if (pcm_callback_ != nullptr) {
        pcm_callback_(pcm_per_frame_, size);
      }
      pcm_dst_time_ += frame_duration_;
    } else {
      LogInfo("AudioCapturer::Work(), capture with no data!\n");
      reset_to_head = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  LogInfo("AudioCapturer::Work() end\n");
}

int AudioCapturer::openPcmFile(const char *file_name) {
  if (pcm_fp_ == nullptr) {
    pcm_fp_ = fopen(file_name, "rb");
    if (pcm_fp_ == nullptr) {
      return RET_ERR_OPEN_FILE;
    }
    return RET_OK;
  }
  return RET_ERR_OPEN_FILE;
}

int AudioCapturer::closePcmFile() {
  if (pcm_fp_) {
    fclose(pcm_fp_);
    pcm_fp_ = nullptr;
  }
  return RET_OK;
}

RET_CODE AudioCapturer::Init(std::unique_ptr<Properties> properties) {
  LogInfo("AudioCapturer::Init(..) called\n");
  properties_ = std::move(properties);
  std::string input_pcm_name = properties_->GetProperty("input_pcm_name");
  uint32_t nb_samples = properties_->GetProperty("nb_samples", 1024);
  uint32_t sample_rate = properties_->GetProperty("sample_rate", 44100);
  pcm_per_frame_size_ = nb_samples *
                        properties_->GetProperty("byte_per_sample", 2) *
                        properties_->GetProperty("channels", 2);
  pcm_per_frame_ = new uint8_t[pcm_per_frame_size_];
  if (!pcm_per_frame_) {
    return RET_ERR_OUTOFMEMORY;
  }
  if (openPcmFile(input_pcm_name.c_str()) < 0) {
    LogInfo("openPcmFile %s failed", input_pcm_name.c_str());
    return RET_FAIL;
  }
  frame_duration_ = 1.0 * nb_samples / sample_rate * 1000.0;
  return RET_OK;
}
