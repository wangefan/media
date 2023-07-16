#ifndef PUSHWORKER_H
#define PUSHWORKER_H

#include "audiocapturer.h"
#include "aacencoder.h"
#include "resampler.h"
#include "mediabase.h"
#include "utility.h"
#include <memory>
class PushWorker {
public:
  PushWorker();
  ~PushWorker();

  RET_CODE Init(const Properties &properties);
  RET_CODE Start();
  void PcmCallback(uint8_t *pcm, int32_t size);

private:
  std::unique_ptr<AudioCapturer> audio_capturer_;
  std::unique_ptr<AACEncoder> audio_encoder_;
  std::unique_ptr<Resampler> resampler_;
  AVFrame *a_frame_;
};

#endif // PUSHWORKER_H
