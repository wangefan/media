#ifndef MEDIAFORMAT_H
#define MEDIAFORMAT_H

#include <cstdint>

class MediaFormat {
public:
  enum AudioFormat { RAW_FORMAT_PCM, ENC_FORMAT_AAC };
  MediaFormat(AudioFormat audio_format, int32_t sample_rate, int32_t channels);

private:
  // audio settings
  AudioFormat audio_format_;
  int32_t sample_rate_;
  int32_t channels_;
};

#endif // MEDIAFORMAT_H
