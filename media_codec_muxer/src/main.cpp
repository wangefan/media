#include "dlog.h"
#include "mediaworker.h"
#include <chrono>
#include <thread>

int main(int argc, char *argv[]) {
  MediaWorker media_worker;
  if (!media_worker.Init("./record.mp4")) {
    LogError("MediaWorker Init failed");
    return -1;
  }
  if (!media_worker.Start()) {
    LogError("MediaWorker start failed");
    return -1;
  }
  LogInfo("MediaWorker Start ok");
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  char ch;
  scanf(" %c", &ch);
  LogInfo("getchar input, will call media_worker.Stop() to stop recording");

  media_worker.Stop();
  LogInfo("MediaWorker end");
  return 1;
}
