#ifndef MEDIAMUXER_H_
#define MEDIAMUXER_H_

#include "encoder.h"
#include <memory>
extern "C" {
#include <libavcodec/avcodec.h>
}

struct AVPacket;
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
class MediaMuxer {
public:
  enum Format {
    MP4,
    FLV,
    MKV,
  };
  MediaMuxer(Format format, const std::string &url);
  virtual ~MediaMuxer();
  int AddTrack(AVCodecContext *);
  bool Start();
  void WriteSampleData(int track_index, AVPacket *audio_packet);
  bool Stop();

private:
  AVFormatContext *fmt_ctx_ = nullptr;
  AVRational video_codec_time_base_;
  AVRational audio_codec_time_base_;
  AVStream *video_stream_ = nullptr;
  AVStream *audio_stream_ = nullptr;
  std::string url_ = "";
  bool started_ = false;
  std::mutex write_data_mutex_;
};

#endif // MEDIAMUXER_H_
