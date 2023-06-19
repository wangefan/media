#ifndef PUSHWORKER_H
#define PUSHWORKER_H

#include "audiocapturer.h"
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
};

#endif // PUSHWORKER_H
