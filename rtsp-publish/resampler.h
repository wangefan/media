#ifndef RESAMPLER_H_
#define RESAMPLER_H_
#include "mediabase.h"
#include <memory>

class AVFrame;

class Resampler {
public:
  Resampler();
  virtual ~Resampler();
  void convertToFlt(AVFrame *frame, uint8_t *pcm, int32_t size);

private:
  uint8_t *resample_fltp_buf_;
};

#endif // RESAMPLER_H_
