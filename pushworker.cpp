#include "pushworker.h"
#include "aacencoder.h"
#include "audiocapturer.h"
#include "avpublishtime.h"
#include "dlog.h"
#include <functional>
extern "C" {
#include <libavcodec/avcodec.h>
}

PushWorker::PushWorker() : a_frame_(nullptr), a_packet_(nullptr) {}
PushWorker::~PushWorker() {
  if (a_frame_ != nullptr) {
    av_frame_free(&a_frame_);
  }
  if (a_packet_ != nullptr) {
    av_packet_free(&a_packet_);
  }
}
static FILE *g_aac_fp_ = nullptr;

void PushWorker::dumpToAACFile(AVPacket *packet) {
  if (!g_aac_fp_) {
    g_aac_fp_ = fopen("push_dump.aac", "wb");
    if (!g_aac_fp_) {
      LogError("fopen push_dump.aac failed");
      return;
    }
  }
  if (g_aac_fp_) {
    uint8_t adts_header[7];
    if (audio_encoder_->GetAdtsHeader(adts_header, packet->size) != RET_OK) {
      LogError("GetAdtsHeader failed");
      return;
    }
    fwrite(adts_header, 1, 7, g_aac_fp_);
    fwrite(packet->data, 1, packet->size, g_aac_fp_);
  }
}

// callback from AudioCapturer, callback one frame pcm data with
// nb_samples * channels * byte_per_sample
void PushWorker::PcmCallback(uint8_t *pcm, int32_t size) {
  LogInfo("PushWorker::PcmCallback(..) called: size:%d\n", size);
  int64_t pts = (int64_t)AVPublishTime::GetInstance()->get_audio_pts();
  resampler_->convertToFlt(a_frame_, pcm, size);
  RET_CODE encode_ret = audio_encoder_->Encode(a_frame_, a_packet_, pts);
  if (encode_ret != RET_OK) {
    LogError("PushWorker::PcmCallback, audio_encoder_->Encode failed");
    return;
  }
  LogInfo(
      "PushWorker::PcmCallback(..) audio_encoder_->Encode ok\n");
  if (encode_ret == RET_OK) {
    dumpToAACFile(a_packet_);
  }
}

RET_CODE PushWorker::Init(const Properties &properties) {
  // 设置音频捕获
  const auto nb_samples = 1024;
  audio_capturer_ = std::make_unique<AudioCapturer>();
  auto aud_cap_properties = std::make_unique<Properties>();
  aud_cap_properties->SetProperty("audio_test",
                                  properties.GetProperty("audio_test", 1));
  aud_cap_properties->SetProperty("input_pcm_name",
                                  properties.GetProperty("input_pcm_name"));
  // 麦克风采样属性
  aud_cap_properties->SetProperty("format",
                                  properties.GetProperty("mic_sample_fmt"));
  aud_cap_properties->SetProperty("channels",
                                  properties.GetProperty("mic_channels"));

  aud_cap_properties->SetProperty("nb_samples", nb_samples);
  aud_cap_properties->SetProperty("byte_per_sample", 2); // fix me
  if (audio_capturer_->Init(std::move(aud_cap_properties)) != RET_OK) {
    LogError("AudioCapturer Init failed");
    return RET_FAIL;
  }
  audio_capturer_->AddCallback(std::bind(&PushWorker::PcmCallback, this,
                                         std::placeholders::_1,
                                         std::placeholders::_2));

  // init aac encoder
  const auto encode_channels = 2;
  
  const auto sample_rate = 44100;
  const auto encode_sample_fmt = AV_SAMPLE_FMT_FLTP;
  auto aac_encoder_properties = std::make_unique<Properties>();
  aac_encoder_properties->SetProperty(AAC_ENCODER_PROP_CHANNELS,
                                      encode_channels);
  aac_encoder_properties->SetProperty(AAC_ENCODER_PROP_NB_SAMPLES, nb_samples);
  aac_encoder_properties->SetProperty(AAC_ENCODER_PROP_SAMPLE_RATE,
                                      sample_rate);
  aac_encoder_properties->SetProperty(AAC_ENCODER_PROP_SAMPLE_FMT,
                                      encode_sample_fmt);
  audio_encoder_ = std::make_unique<AACEncoder>();
  if (audio_encoder_->Init(std::move(aac_encoder_properties)) != RET_OK) {
    LogError("AACEncoder Init failed");
    return RET_FAIL;
  }

  // Init Resampler
  resampler_ = std::make_unique<Resampler>();

  // init aac frame, it is resampled from pcm frame and intent to be encoded.
  a_frame_ = av_frame_alloc();
  a_frame_->format = encode_sample_fmt;
  a_frame_->nb_samples = nb_samples;
  a_frame_->channels = encode_channels;
  a_frame_->channel_layout = av_get_default_channel_layout(encode_channels);
  if (av_frame_get_buffer(a_frame_, 0) < 0) {
    LogError("av_frame_get_buffer failed");
    return RET_FAIL;
  }

  // init aac packet, it is encoded from aac frame.
  a_packet_ = av_packet_alloc();

  return RET_OK;
}

RET_CODE PushWorker::Start() {
  if (audio_capturer_->Start() != RET_OK) {
    LogError("AudioCapturer Start failed");
    return RET_FAIL;
  }

  return RET_OK;
}

RET_CODE PushWorker::Stop() {
  /*
   * must stop audio capturer first since there is
   * callback from audio capturer, would crash if release
   * encoder or other first without release capturer.
   */
  audio_capturer_->Stop();

  return RET_OK;
}
