#include "dlog.h"
#include "mediabase.h"
#include "pushworker.h"
#include "utility.h"
#include <iostream>
extern "C" {
#include <libavcodec/avcodec.h>
}
using namespace std;

int main() {
  Properties properties;
  // Audio test properties
  properties.SetProperty("audio_test", 1); // 音频测试模式
  properties.SetProperty("input_pcm_name", "48000_2_s16le.pcm");
  // 麦克风采样属性
  properties.SetProperty("mic_sample_fmt", AV_SAMPLE_FMT_S16);
  properties.SetProperty("mic_sample_rate", 48000);
  properties.SetProperty("mic_channels", 2);
  PushWorker push_worker;
  if (push_worker.Init(properties) != RET_OK) {
    LogError("PushWorker Init failed");
    return -1;
  }
  if (push_worker.Start() != RET_OK) {
    LogError("PushWorker start failed");
    return -1;
  }
  cout << "PushWorker Start ok" << endl;
  getchar();
  return 0;
}
