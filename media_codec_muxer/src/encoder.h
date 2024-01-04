#ifndef ENCODER_H_
#define ENCODER_H_

#include <stdint.h>

struct AVCodecContext;
struct RawDataBufferInfo;
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
  virtual bool QueueDataToEncode(RawDataBufferInfo &raw_data_buffer_info) = 0;
};

#endif // ENCODER_H_
