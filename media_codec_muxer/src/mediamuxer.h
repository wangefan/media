#ifndef MEDIAMUXER_H_
#define MEDIAMUXER_H_

#include "encoder.h"
#include <memory>

struct AVPacket;
namespace mm {

class MediaMuxer {
public:
  enum Format {
    MP4,
    FLV,
    MKV,
  };
  MediaMuxer(Format format);
  virtual ~MediaMuxer() = default;
  int AddTrack(std::weak_ptr<Encoder> format);
  void WriteSampleData(int track_index, AVPacket *audio_packet);
  bool Start();
};

} // namespace mm
#endif // MEDIAMUXER_H_
