#include "audioencoder.h"
#include "dlog.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace aue {

AudioEncoder::AudioEncoder() : codec_ctx_(nullptr), codec_(nullptr) {}

AudioEncoder::~AudioEncoder() {
  if (codec_ctx_) {
    avcodec_free_context(&codec_ctx_);
  }
}

bool AudioEncoder::Init(std::shared_ptr<MediaFormat> input_format,
                        std::shared_ptr<MediaFormat> output_format) {
  input_format_ = input_format;
  output_format_ = output_format;

  return true;
}

bool AudioEncoder::Start() {
  LogInfo("AudioEncoder::Start called");
  return true;
}

void AudioEncoder::Work() {
  LogInfo("AudioEncoder::Work called");
  LogInfo("AudioEncoder::Work end");
}

bool AudioEncoder::QueueDataToEncode(uint8_t *pcm, int32_t size,
                                     int64_t time_stamp) {
  LogInfo("AudioEncoder::QueueDataToEncode called");
  return true;
}

} // namespace aue