#include "mediamuxer.h"
extern "C"
{
#include "libavcodec/avcodec.h"
}

namespace mm {

MediaMuxer::MediaMuxer(Format format) {}

int MediaMuxer::AddTrack(std::weak_ptr<Encoder> encoder) { return -1; }

void MediaMuxer::WriteSampleData(int track_index, AVPacket *audio_packet) {
  audio_packet->stream_index = track_index;
}

bool MediaMuxer::Start() { return true; }

} // namespace mm