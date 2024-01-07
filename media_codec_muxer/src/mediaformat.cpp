#include "mediaformat.h"

MediaFormat::MediaFormat(Format video_format, uint32_t video_width,
                         uint32_t video_height, uint32_t video_fps) {
  format_ = video_format;
  video_width_ = video_width;
  video_height_ = video_height;
  video_fps_ = video_fps;
}

MediaFormat::MediaFormat(Format audio_format, int32_t sample_rate,
                         int32_t channels) {
  format_ = audio_format;
  sample_rate_ = sample_rate;
  channels_ = channels;
}
