#ifndef AUDIOENCODER_H_
#define AUDIOENCODER_H_

#include "encoder.h"

class AudioEncoder : public Encoder {
public:
  AudioEncoder();
  virtual ~AudioEncoder();

  bool Init(std::shared_ptr<MediaFormat> input_format) override;

private:
  void
  consume_queue(std::queue<RawDataBufferInfo> &temp_raw_data_queue) override;

  const char *GetClassName() override { return "AudioEncoder"; }

private:
  uint8_t *resample_fltp_buf_;
};

#endif // AUDIOENCODER_H_