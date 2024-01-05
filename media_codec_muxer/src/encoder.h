#ifndef ENCODER_H_
#define ENCODER_H_

#include "encodeddatabufferinfo.h"
#include "mediaformat.h"
#include "rawdatabufferinfo.h"
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <stdint.h>
#include <thread>

class AVFrame;
class AVCodec;
class AVCodecContext;

class Encoder {
public:
  Encoder();
  virtual ~Encoder();
  virtual bool Init(std::shared_ptr<MediaFormat> input_format) = 0;

  bool Start();
  void Work();
  bool Stop();
  bool QueueDataToEncode(RawDataBufferInfo &raw_data_buffer_info);

  AVCodecContext *GetCodecContext() { return codec_ctx_; }
  std::shared_ptr<MediaFormat> GetInputFormat() { return input_format_; }

  void
  AddCallback(std::function<void(EncodedDataBufferInfo &)> encoded_callback) {
    encoded_callback_ = encoded_callback;
  }

protected:
  std::function<void(EncodedDataBufferInfo &encoded_data_buffer_info)>
      encoded_callback_;
  std::shared_ptr<MediaFormat> input_format_;
  AVCodecContext *codec_ctx_;
  AVFrame *av_frame_;
  AVPacket *av_packet_;

private:
  virtual void
  consume_queue(std::queue<RawDataBufferInfo> &temp_raw_data_queue) = 0;

  virtual const char *GetClassName() = 0;

  bool is_consumer_running_;
  std::unique_ptr<std::thread> consumer_thread_;
  std::queue<RawDataBufferInfo> raw_data_queue_;
  std::mutex raw_data_queue_mutex_;
  std::condition_variable raw_data_queue_cv_;
};

#endif // ENCODER_H_
