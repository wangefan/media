#ifndef MEDIAWORKER_H
#define MEDIAWORKER_H
#include "audioencoder.h"
#include "fakeaudiocapturer.h"
#include "fakevideocapturer.h"
#include "mediamuxer.h"
#include "videoencoder.h"
#include "worker.h"
#include <string>

class MediaWorker : public Worker {
public:
  MediaWorker() = default;
  ~MediaWorker();

  bool Init(const std::string &output_file_name);

  bool Start() override;
  void Stop() override;
  void Work() override;

private:
  // callback from FakeVideoCapturer, callback one frame yuv data with
  // yuv_frame_size = y_frame_size + u_frame_size + v_frame_size
  void YuvCallback(RawDataBufferInfo &raw_data_buffer_info);
  // callback from FakeAudioCapturer, callback one frame pcm data with
  // nb_samples * channels * byte_per_sample
  void PcmCallback(RawDataBufferInfo &raw_data_buffer_info);

  void EncodedVideoCallback(EncodedDataBufferInfo &encoded_data_buffer_info);
  void EncodedAudioCallback(EncodedDataBufferInfo &encoded_data_buffer_info);

private:
  std::unique_ptr<FakeVideoCapturer> video_capturer_;
  std::unique_ptr<FakeAudioCapturer> audio_capturer_;
  std::unique_ptr<VideoEncoder> video_encoder_;
  std::unique_ptr<AudioEncoder> audio_encoder_;
  std::unique_ptr<MediaMuxer> media_muxer_;
  int video_track_index_;
  int audio_track_index_;
};

#endif // MEDIAWORKER_H