#ifndef MEDIAWORKER_H
#define MEDIAWORKER_H
#include "audiocapturer.h"
#include "worker.h"
#include <string>

class MediaWorker : public Worker {
public:
  MediaWorker() = default;
  ~MediaWorker();

  bool Init(const std::string &output_file_name);

  bool Start() override;
  void Work() override;

private:
  // callback from AudioCapturer, callback one frame pcm data with
  // nb_samples * channels * byte_per_sample
  void PcmCallback(uint8_t *pcm, int32_t size, int64_t time_stamp);

private:
  std::string output_file_name_;
  // std::unique_ptr<VideoCapturer> video_capturer_;
  std::unique_ptr<auc::AudioCapturer> audio_capturer_;
};

#endif // MEDIAWORKER_H