#include "worker.h"
#include "dlog.h"

Worker::Worker() { is_running_ = false; }

Worker::~Worker() {
  LogInfo("Worker::~Worker called");
  if (is_running_) {
    Stop();
  }
}

RET_CODE Worker::Start() {
  if (!is_running_) {
    is_running_ = true;
    auto worker_fun = [this] {
      { Work(); }
    };
    thread_ = std::make_unique<std::thread>(worker_fun);
    return RET_OK;
  }
  return RET_FAIL;
}

void Worker::Stop() {
  LogInfo("Worker::Stop() called");
  if (is_running_) {
    is_running_ = false;
    thread_->join();
  }
  LogInfo("Worker::Stop() ok");
}
