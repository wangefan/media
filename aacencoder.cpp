#include "aacencoder.h"
#include "utility.h"
#include "dlog.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

AACEncoder::AACEncoder() : codec_ctx_(nullptr), codec_(nullptr) {}

AACEncoder::~AACEncoder() {
  LogInfo("AACEncoder::~AACEncoder() called");
  if (codec_ctx_) {
    avcodec_free_context(&codec_ctx_);
  }
}

RET_CODE AACEncoder::Init(std::unique_ptr<Properties> properties) {
  if (!properties) {
    LogInfo("properties is nullptr");
    return RET_ERR_MISMATCH_CODE;
  }
  codec_ = avcodec_find_encoder(AV_CODEC_ID_AAC);
  if(!codec_) {
    LogInfo("AACEncoder::Init, avcodec_find_encoder failed");
    return RET_ERR_MISMATCH_CODE;
  }
  codec_ctx_ = avcodec_alloc_context3(codec_);
  if (!codec_ctx_) {
    LogInfo("AACEncoder::Init, avcodec_alloc_context3 failed");
    return RET_ERR_OUTOFMEMORY;
  }
  //set encoder parameters
  auto encode_channels = properties->GetProperty(AAC_ENCODER_PROP_CHANNELS, 2);
  auto encode_nb_samples = properties->GetProperty(AAC_ENCODER_PROP_NB_SAMPLES, 1024);
  auto encode_sample_rate = properties->GetProperty(AAC_ENCODER_PROP_SAMPLE_RATE, 44100);
  auto encode_sample_fmt = (AVSampleFormat)properties->GetProperty(
      AAC_ENCODER_PROP_SAMPLE_FMT, AV_SAMPLE_FMT_FLTP); //AAC sample format is AV_SAMPLE_FMT_FLTP planar float
  codec_ctx_->channels = encode_channels;
  codec_ctx_->channel_layout =
      (int)av_get_default_channel_layout(encode_channels);
  codec_ctx_->sample_rate = encode_sample_rate;
  codec_ctx_->sample_fmt = encode_sample_fmt;
  auto bit_rate = 8 * av_get_bytes_per_sample(encode_sample_fmt) *
                  encode_channels * encode_sample_rate;
  codec_ctx_->bit_rate = bit_rate;
  
  if(avcodec_open2(codec_ctx_, codec_, nullptr) < 0) {
    LogInfo("AACEncoder::Init, avcodec_open2 failed");
    return RET_ERR_MISMATCH_CODE;
  }
  return RET_OK;
}

RET_CODE AACEncoder::Encode(AVFrame *frame, AVPacket *packet,
                            const int64_t pts) {
  LogInfo("AACEncoder::Encode called\n");
  if (!codec_ctx_) {
    LogInfo("AACEncoder::Encode, codec_ctx_ is nullptr");
    return RET_FAIL;
  }
  if (!frame) {
    LogInfo("AACEncoder::Encode, frame is nullptr");
    return RET_FAIL;
  }

  frame->pts = pts;

  int ret_encode = RET_OK;
  ret_encode = avcodec_send_frame(codec_ctx_, frame);
  if (ret_encode != RET_OK) {
    if (ret_encode == AVERROR(EAGAIN)) {
      LogInfo("AACEncoder::Encode, avcodec_send_frame failed with EAGAIN, "
              "ret_encode=%d",
              ret_encode);
      return RET_ERR_EAGAIN;
    } else if (ret_encode == AVERROR_EOF) {
      LogInfo("AACEncoder::Encode, avcodec_send_frame failed with AVERROR_EOF, "
              "ret_encode=%d",
              ret_encode);
      return RET_ERR_EOF;
    } else {
      LogInfo("AACEncoder::Encode, avcodec_send_frame failed, ret_encode=%d",
              ret_encode);
      return RET_FAIL;
    }
  }

  av_packet_unref(packet);
  ret_encode = avcodec_receive_packet(codec_ctx_, packet);
  if (ret_encode != RET_OK) {
    if (ret_encode == AVERROR(EAGAIN)) {
      LogInfo("AACEncoder::Encode, avcodec_receive_packet failed with EAGAIN, "
              "ret_encode=%d",
              ret_encode);
      return RET_ERR_EAGAIN;
    } else if (ret_encode == AVERROR_EOF) {
      LogInfo(
          "AACEncoder::Encode, avcodec_receive_packet failed with AVERROR_EOF, "
          "ret_encode=%d",
          ret_encode);
      return RET_ERR_EOF;
    } else {
      LogInfo(
          "AACEncoder::Encode, avcodec_receive_packet failed, ret_encode=%d",
          ret_encode);
      return RET_FAIL;
    }
  }
  return RET_OK;
}

RET_CODE AACEncoder::GetAdtsHeader(uint8_t *adts_header, int aac_length) {
  uint8_t freqIdx = 0; // 0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
  switch (codec_ctx_->sample_rate) {
  case 96000:
    freqIdx = 0;
    break;
  case 88200:
    freqIdx = 1;
    break;
  case 64000:
    freqIdx = 2;
    break;
  case 48000:
    freqIdx = 3;
    break;
  case 44100:
    freqIdx = 4;
    break;
  case 32000:
    freqIdx = 5;
    break;
  case 24000:
    freqIdx = 6;
    break;
  case 22050:
    freqIdx = 7;
    break;
  case 16000:
    freqIdx = 8;
    break;
  case 12000:
    freqIdx = 9;
    break;
  case 11025:
    freqIdx = 10;
    break;
  case 8000:
    freqIdx = 11;
    break;
  case 7350:
    freqIdx = 12;
    break;
  default:
    LogError("can't support sample_rate:%d");
    freqIdx = 4;
    return RET_FAIL;
  }
  uint8_t ch_cfg = codec_ctx_->channels;
  uint32_t frame_length = aac_length + 7;
  adts_header[0] = 0xFF;
  adts_header[1] = 0xF1;
  adts_header[2] = ((codec_ctx_->profile) << 6) + (freqIdx << 2) + (ch_cfg >> 2);
  adts_header[3] = (((ch_cfg & 3) << 6) + (frame_length >> 11));
  adts_header[4] = ((frame_length & 0x7FF) >> 3);
  adts_header[5] = (((frame_length & 7) << 5) + 0x1F);
  adts_header[6] = 0xFC;
  return RET_OK;
}

int AACEncoder::GetFrameSize() { return codec_ctx_->frame_size; }