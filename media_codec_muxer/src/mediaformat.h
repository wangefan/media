#ifndef MEDIAFORMAT_H
#define MEDIAFORMAT_H

#include <cstdint>

// Todo: separate VideoFormat/AudioFormat to child class
class MediaFormat {
public:
  enum Format { RAW_VIDEO_YUV, RAW_AUDIO_PCM };
  Format GetFormat() { return format_; }

  // video
  MediaFormat(Format video_format, uint32_t video_width, uint32_t video_height,
              uint32_t video_fps);
  int32_t GetWidth() { return video_width_; }
  int32_t GetHeight() { return video_height_; }
  int32_t GetFps() { return video_fps_; }

  // audio
  MediaFormat(Format audio_format, int32_t sample_rate, int32_t channels);
  int32_t GetSampleRate() { return sample_rate_; }
  int32_t GetChannels() { return channels_; }

private:
  Format format_;
  // video settings
  uint32_t video_width_;
  uint32_t video_height_;
  uint32_t video_fps_;

  // audio settings
  int32_t sample_rate_;
  int32_t channels_;
};

#endif // MEDIAFORMAT_H
