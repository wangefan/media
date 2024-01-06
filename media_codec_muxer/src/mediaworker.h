#ifndef MEDIAWORKER_H
#define MEDIAWORKER_H
#include "mediamuxer.h"
#include "audiocapturer.h"
#include "audioencoder.h"
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
  // callback from AudioCapturer, callback one frame pcm data with
  // nb_samples * channels * byte_per_sample
  void PcmCallback(RawDataBufferInfo &raw_data_buffer_info);
  void EncodedAudioCallback(EncodedDataBufferInfo &encoded_data_buffer_info);

private:
  // Todo: std::unique_ptr<VideoCapturer> video_capturer_;
  std::unique_ptr<auc::AudioCapturer> audio_capturer_;
  // Todo: std::unique_ptr<VideoEncoder> video_encoder_;
  std::unique_ptr<AudioEncoder> audio_encoder_;
  std::unique_ptr<MediaMuxer> media_muxer_;
  int video_track_index_;
  int audio_track_index_;
};

#endif // MEDIAWORKER_H