#include "videocapturer.h"
#include "dlog.h"
#include "timesutil.h"

VideoCapturer::~VideoCapturer() {}

bool VideoCapturer::Init(uint32_t video_width, uint32_t video_height,
                         uint32_t video_fps) {
  LogInfo("VideoCapturer::Init(..) begin");
  video_width_ = video_width;
  video_height_ = video_height;
  video_fps_ = video_fps;
  return true;
}

void VideoCapturer::Work() {
  LogInfo("VideoCapturer::Work() begin");
  const std::string input_yuv_name = "./media_files/count_s.yuv";
  FILE *yvu_fp = fopen(input_yuv_name.c_str(), "rb");
  if (yvu_fp == nullptr) {
    LogInfo("VideoCapturer::Work(), open file error, end");
    if (yuv_callback_ != nullptr) {
      RawDataBufferInfo raw_data_buffer_info{RawDataState::RAW_DATA_STATE_ERROR,
                                             nullptr, -1, -1};
      yuv_callback_(raw_data_buffer_info);
    }
    return;
  }

  bool reset_to_head = true;
  const auto y_frame_size = video_width_ * video_height_;
  const auto u_frame_size = y_frame_size / 4;
  const auto v_frame_size = y_frame_size / 4;
  const auto yuv_frame_size = y_frame_size + u_frame_size + v_frame_size;
  uint8_t *yuv_frame_buf = new uint8_t[yuv_frame_size];

  auto frame_duration = 1.0 / video_fps_ * 1000.0; // ms
  int64_t yuv_record_start_time = TimesUtil::GetTimeMillisecond();
  int64_t yuv_frame_start_time = TimesUtil::GetTimeMillisecond();
  int64_t yuv_frame_dst_time = yuv_frame_start_time + frame_duration;
  if (yuv_callback_ != nullptr) {
    RawDataBufferInfo raw_data_buffer_info{RawDataState::RAW_DATA_STATE_BEGIN,
                                           nullptr, -1, -1};
    yuv_callback_(raw_data_buffer_info);
  }

  while (true) {
    {
      std::lock_guard<std::mutex> lock(is_running_mutex_);
      if (!is_running_) {
        LogInfo("VideoCapturer::Work break!");
        break;
      }
    }
    if (reset_to_head) {
      fseek(yvu_fp, 0, SEEK_SET);
      reset_to_head = false;
    }
    auto current_time = TimesUtil::GetTimeMillisecond();
    if (current_time < yuv_frame_dst_time) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    // read a frame yuv data from file.
    int32_t size = fread(yuv_frame_buf, 1, yuv_frame_size, yvu_fp);
    if (size > 0) {
      if (yuv_callback_ != nullptr) {
        auto raw_data = std::make_unique<uint8_t[]>(size);
        std::copy(yuv_frame_buf, yuv_frame_buf + size, raw_data.get());
        RawDataBufferInfo raw_data_buffer_info{
            RawDataState::RAW_DATA_STATE_SENDING, std::move(raw_data), size,
            yuv_frame_start_time - yuv_record_start_time};
        yuv_callback_(raw_data_buffer_info);
      }
      yuv_frame_start_time = yuv_frame_dst_time;
      yuv_frame_dst_time += frame_duration;
    } else {
      LogInfo("VideoCapturer::Work(), capture with no data!");
      reset_to_head = true;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  fclose(yvu_fp);
  yvu_fp = nullptr;
  delete[] yuv_frame_buf;
  yuv_frame_buf = nullptr;
  if (yuv_callback_ != nullptr) {
    RawDataBufferInfo raw_data_buffer_info{RawDataState::RAW_DATA_STATE_END,
                                           nullptr, -1, -1};
    yuv_callback_(raw_data_buffer_info);
  }
  LogInfo("VideoCapturer::Work() end");
}
