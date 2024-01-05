#include "encoder.h"
#include "dlog.h"
#include "timesutil.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

Encoder::Encoder()
    : codec_ctx_(nullptr), av_frame_(nullptr), av_packet_(nullptr),
      is_consumer_running_(false) {}

Encoder::~Encoder() {
  if (codec_ctx_) {
    avcodec_free_context(&codec_ctx_);
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

bool Encoder::Start() {
  LogInfo("%s::Start called", GetClassName());
  {
    std::lock_guard<std::mutex> lock(raw_data_queue_mutex_);
    if (is_consumer_running_) {
      LogInfo("%s::Start(), already running", GetClassName());
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

void Encoder::Work() {
  LogInfo("%s::Work begins", GetClassName());
  EncodedDataBufferInfo encoded_data_buffer_info_begin{
      EncodedDataState::STATE_BEGIN, nullptr};
  encoded_callback_(encoded_data_buffer_info_begin);
  while (true) {
    std::queue<RawDataBufferInfo> temp_raw_data_queue;
    {
      std::unique_lock<std::mutex> lock(raw_data_queue_mutex_);
      while (raw_data_queue_.empty() && is_consumer_running_) {
        LogDebug("%s::Work, has no raw data to encode, wait..", GetClassName());
        raw_data_queue_cv_.wait(lock);
      }
      while (!raw_data_queue_.empty()) {
        temp_raw_data_queue.push(std::move(raw_data_queue_.front()));
        raw_data_queue_.pop();
      }
    }

    consume_queue(temp_raw_data_queue);

    if (!is_consumer_running_) {
      LogInfo("%s::Work, is_consumer_running_ == false, to flush "
              "encoded data and break!",
              GetClassName());
      std::queue<RawDataBufferInfo> temp_raw_data_queue_to_flush;
      temp_raw_data_queue_to_flush.push(
          {RawDataState::RAW_DATA_STATE_SENDING, nullptr, -1, -1});
      consume_queue(temp_raw_data_queue_to_flush);
      break;
    }
  }
  EncodedDataBufferInfo encoded_data_buffer_info_end{
      EncodedDataState::STATE_END, nullptr};
  encoded_callback_(encoded_data_buffer_info_end);
  LogInfo("%s::Work end", GetClassName());
}

bool Encoder::Stop() {
  LogInfo("%s::Stop called", GetClassName());
  {
    std::lock_guard<std::mutex> lock(raw_data_queue_mutex_);
    if (!is_consumer_running_) {
      LogInfo("Encoder::Stop(), already stopped");
      return true;
    }
    is_consumer_running_ = false;
  }
  raw_data_queue_cv_.notify_one();
  consumer_thread_->join();
  LogInfo("%s::Stop() ok", GetClassName());
  return true;
}

bool Encoder::QueueDataToEncode(RawDataBufferInfo &raw_data_buffer_info) {
  LogDebug("%s::QueueDataToEncode called", GetClassName());
  {
    std::lock_guard<std::mutex> lock(raw_data_queue_mutex_);
    if (!is_consumer_running_) {
      return false;
    }
    raw_data_queue_.push(std::move(raw_data_buffer_info));
  }
  raw_data_queue_cv_.notify_one();
  return true;
}