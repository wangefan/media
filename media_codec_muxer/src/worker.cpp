#include "worker.h"
#include "dlog.h"

Worker::Worker() { is_running_ = false; }

Worker::~Worker() {
  LogInfo("Worker::~Worker called");
  if (is_running_) {
    Stop();
  }
}

bool Worker::Start() {
  LogInfo("Worker::Start() called");
  {
    std::lock_guard<std::mutex> lock(is_running_mutex_);
    if (!is_running_) {
      is_running_ = true;
    } else {
      LogInfo("Worker::Start() already running");
      return false;
    }
  }
  auto worker_fun = [this] {
    { Work(); }
  };
  thread_ = std::make_unique<std::thread>(worker_fun);
  return true;
}

void Worker::Stop() {
  LogInfo("Worker::Stop() called");
  {
    std::lock_guard<std::mutex> lock(is_running_mutex_);
    if (is_running_) {
      is_running_ = false;
    }
  }
  thread_->join();
  LogInfo("Worker::Stop() ok");
}
