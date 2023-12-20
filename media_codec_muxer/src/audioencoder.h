#ifndef AUDIOENCODER_H_
#define AUDIOENCODER_H_

#include "mediaformat.h"
#include <memory>
#include "worker.h"
#include <functional>

class AVPacket;
class AVFrame;
class AVCodec;
class AVCodecContext;

namespace aue {

class AudioEncoder : public Worker {
public:
  AudioEncoder();
  virtual ~AudioEncoder();

  bool Init(std::shared_ptr<MediaFormat> input_format,
            std::shared_ptr<MediaFormat> output_format);
  void AddCallback(std::function<void(AVPacket *packet)> encoded_callback) {
    encoded_callback_ = encoded_callback;
  }
  bool Start() override;
  void Work() override;
  bool QueueDataToEncode(uint8_t *pcm, int32_t size, int64_t time_stamp);

  std::shared_ptr<MediaFormat> GetOutputFormat() { return output_format_; }

private:
  AVCodec *codec_;
  AVCodecContext *codec_ctx_;
  std::function<void(AVPacket *packet)> encoded_callback_;
  std::shared_ptr<MediaFormat> input_format_;
  std::shared_ptr<MediaFormat> output_format_;
};

} // namespace aue
#endif // AUDIOENCODER_H_
