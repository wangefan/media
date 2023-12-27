#ifndef AUDIOENCODER_H_
#define AUDIOENCODER_H_

#include "encoder.h"
#include "mediaformat.h"
#include "worker.h"
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>

class AVPacket;
class AVFrame;
class AVCodec;
class AVCodecContext;

namespace aue {

struct RawDataBufferInfo {
  std::unique_ptr<uint8_t[]> raw_data;
  int32_t size;
  int64_t time_stamp;
};
class AudioEncoder : public Encoder {
public:
  AudioEncoder();
  virtual ~AudioEncoder();

  bool Init(std::shared_ptr<MediaFormat> input_format);
  void AddCallback(std::function<void(AVPacket *packet)> encoded_callback) {
    encoded_callback_ = encoded_callback;
  }
  // Todo: make it base function
  AVCodecContext *GetCodecContext() override { return codec_ctx_; }
  // Todo: make it base function
  bool Start() override;
  // Todo: make it base function
  void Work() override;
  // Todo: make it base function
  bool Stop() override;
  // Todo: make it base function
  bool QueueDataToEncode(uint8_t *pcm, int32_t size,
                         int64_t time_stamp) override;

  std::shared_ptr<MediaFormat> GetInputFormat() { return input_format_; }

private:
  void consume_queue(std::queue<RawDataBufferInfo> &temp_raw_data_queue);

private:
  AVCodecContext *codec_ctx_;
  uint8_t *resample_fltp_buf_;
  AVFrame *av_frame_;
  AVPacket *av_packet_;
  std::function<void(AVPacket *packet)> encoded_callback_;
  std::shared_ptr<MediaFormat> input_format_;

  bool is_consumer_running_;
  std::unique_ptr<std::thread> consumer_thread_;
  std::queue<RawDataBufferInfo> raw_data_queue_;
  std::mutex raw_data_queue_mutex_;
  std::condition_variable raw_data_queue_cv_;
};

} // namespace aue
#endif // AUDIOENCODER_H_
