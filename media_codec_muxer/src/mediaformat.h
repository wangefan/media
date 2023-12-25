#ifndef MEDIAFORMAT_H
#define MEDIAFORMAT_H

#include <cstdint>

class MediaFormat {
public:
  enum AudioFormat { RAW_FORMAT_PCM };
  MediaFormat(AudioFormat audio_format, int32_t sample_rate, int32_t channels);

  AudioFormat GetFormat() { return audio_format_; }
  int32_t GetSampleRate() { return sample_rate_; }
  int32_t GetChannels() { return channels_; }

private:
  // audio settings
  AudioFormat audio_format_;
  int32_t sample_rate_;
  int32_t channels_;
};

#endif // MEDIAFORMAT_H
