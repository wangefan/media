#include "resampler.h"
#include "dlog.h"
#include "utility.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

Resampler::Resampler() : resample_fltp_buf_(nullptr) {}

Resampler::~Resampler() {
  LogInfo("Resampler::~Resampler() called\n");
  if (resample_fltp_buf_ != nullptr) {
    av_freep(&resample_fltp_buf_);
  }
}

void Resampler::convertToFlt(AVFrame *frame, uint8_t *pcm, int32_t size) {
  LogInfo("Resampler::convertToFlat() called\n");
  if (!resample_fltp_buf_) {
    // It is supposed to be 4*2*1024 = 8192 for this case
    int resample_fltp_buf_size =
        av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples,
                                   (enum AVSampleFormat)frame->format, 1);
    resample_fltp_buf_ = (uint8_t *)av_malloc(resample_fltp_buf_size);
  }
  short *pcm_short = (short *)pcm;
  float *resample_fltp_buf_float = (float *)resample_fltp_buf_;
  float *fltp_l = resample_fltp_buf_float; // -1~1
  float *fltp_r = resample_fltp_buf_float + frame->nb_samples;
  // sl0, sr0, sl1, sr1 => fl0, fl1, fr0, fr1
  for (int idx = 0; idx < frame->nb_samples; idx++) {
    fltp_l[idx] = pcm_short[idx * 2] / 32768.0f;
    fltp_r[idx] = pcm_short[idx * 2 + 1] / 32768.0f;
  }
  av_frame_make_writable(frame);
  av_samples_fill_arrays(frame->data, frame->linesize, resample_fltp_buf_,
                         frame->channels, frame->nb_samples,
                         (AVSampleFormat)frame->format, 0);
}