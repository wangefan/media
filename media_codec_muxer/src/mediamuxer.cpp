#include "mediamuxer.h"

namespace mm {

MediaMuxer::MediaMuxer(Format format) {}

int MediaMuxer::AddTrack(std::shared_ptr<MediaFormat> format) { return -1; }

void MediaMuxer::WriteSampleData(int track_index, AVPacket *audio_packet) {}

bool MediaMuxer::Start() { return true; }

} // namespace mm