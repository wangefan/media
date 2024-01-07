#ifndef VIDEOENCODER_H_
#define VIDEOENCODER_H_

#include "encoder.h"

class VideoEncoder : public Encoder {
public:
  VideoEncoder();
  virtual ~VideoEncoder();

  bool Init(std::shared_ptr<MediaFormat> input_format) override;

private:
  void
  consume_queue(std::queue<RawDataBufferInfo> &temp_raw_data_queue) override;

  const char *GetClassName() override { return "VideoEncoder"; }

private:
  
};

#endif // VIDEOENCODER_H_
