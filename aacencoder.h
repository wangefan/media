#ifndef AACENCODER_H_
#define AACENCODER_H_
#include "mediabase.h"
#include <memory>

class Properties;
class AVPacket;
class AVFrame;
class AVCodec;
class AVCodecContext;

#define AAC_ENCODER_PROP_CHANNELS "aac_encoder_prop_channels"
#define AAC_ENCODER_PROP_NB_SAMPLES "aac_encoder_prop_nb_samples"
#define AAC_ENCODER_PROP_SAMPLE_RATE "aac_encoder_prop_sample_rate"
#define AAC_ENCODER_PROP_SAMPLE_FMT "aac_encoder_prop_sample_fmt"

class AACEncoder {
public:
  AACEncoder();
  virtual ~AACEncoder();

  RET_CODE Init(std::unique_ptr<Properties> properties);

  virtual RET_CODE Encode(AVFrame *frame, AVPacket *packet, const int64_t pts);
  RET_CODE GetAdtsHeader(uint8_t *adts_header, int aac_length);
  int GetFrameSize();

private:
  AVCodec *codec_;
  AVCodecContext *codec_ctx_;
};

#endif // AACENCODER_H_
