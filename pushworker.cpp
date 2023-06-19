#include <functional>
#include "pushworker.h"
#include "dlog.h"

PushWorker::PushWorker() {}
PushWorker::~PushWorker() {}

void PushWorker::PcmCallback(uint8_t *pcm, int32_t size) {
  LogInfo("PushWorker::PcmCallback(..) called: size:%d\n", size);
}

RET_CODE PushWorker::Init(const Properties &properties) {
  // 设置音频捕获
  audio_capturer_ = std::make_unique<AudioCapturer>();
  auto aud_cap_properties = std::make_unique<Properties>();
  aud_cap_properties->SetProperty("audio_test",
                                  properties.GetProperty("audio_test", 1));
  aud_cap_properties->SetProperty("input_pcm_name",
                                  properties.GetProperty("input_pcm_name"));
  // 麦克风采样属性
  aud_cap_properties->SetProperty("format",
                                  properties.GetProperty("mic_sample_fmt"));
  aud_cap_properties->SetProperty("channels",
                                  properties.GetProperty("mic_channels"));

  aud_cap_properties->SetProperty("nb_samples", 1024); // 由编码器提供 // fix me
  aud_cap_properties->SetProperty("byte_per_sample", 2); // fix me
  if (audio_capturer_->Init(std::move(aud_cap_properties)) != RET_OK) {
    LogError("AudioCapturer Init failed");
    return RET_FAIL;
  }

  audio_capturer_->AddCallback(std::bind(&PushWorker::PcmCallback, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));

  return RET_OK;
}

RET_CODE PushWorker::Start() {
  if (audio_capturer_->Start() != RET_OK) {
    LogError("AudioCapturer Start failed");
    return RET_FAIL;
  }

  return RET_OK;
}
