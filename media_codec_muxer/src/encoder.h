#ifndef ENCODER_H_
#define ENCODER_H_

#include <stdint.h>

struct AVCodecContext;
class Encoder {
public:
  Encoder() = default;
  virtual ~Encoder() = default;

  virtual AVCodecContext *GetCodecContext() = 0;

  // Todo: make it base function
  virtual bool Start() = 0;
  // Todo: make it base function
  virtual void Work() = 0;
  // Todo: make it base function
  virtual bool Stop() = 0;
  // Todo: make it base function
  virtual bool QueueDataToEncode(uint8_t *pcm, int32_t size,
                                 int64_t time_stamp) = 0;
};

#endif // ENCODER_H_
