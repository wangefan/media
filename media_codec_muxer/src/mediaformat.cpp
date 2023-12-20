#include "mediaformat.h"

MediaFormat::MediaFormat(AudioFormat audio_format, int32_t sample_rate,
                         int32_t channels) {
  audio_format_ = audio_format;
  sample_rate_ = sample_rate;
  channels_ = channels;
}
