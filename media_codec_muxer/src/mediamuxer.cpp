#include "mediamuxer.h"
#include "dlog.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include <libavformat/avformat.h>
}

namespace mm {

MediaMuxer::MediaMuxer(Format format, const std::string &url) {
  int ret = avformat_alloc_output_context2(&fmt_ctx_, NULL, NULL, url.c_str());
  if (ret < 0) {
    LogInfo("MediaMuxer::MediaMuxer, avformat_alloc_output_context2 failed!");
  }

  url_ = url;
}

MediaMuxer::~MediaMuxer() {
  if (fmt_ctx_) {
    avformat_close_input(&fmt_ctx_);
  }
  url_ = "";
}

int MediaMuxer::AddTrack(AVCodecContext *codec_ctx) {
  int index = -1;
  if (!fmt_ctx_) {
    LogInfo("MediaMuxer::AddTrack, fmt ctx is nullptr");
    return index;
  }
  if (!codec_ctx) {
    LogInfo("MediaMuxer::AddTrack, codec ctx is nullptr");
    return index;
  }
  AVStream *st = avformat_new_stream(fmt_ctx_, NULL);
  if (!st) {
    LogInfo("MediaMuxer::AddTrack, avformat_new_stream failed");
    return index;
  }
  index = st->index;
  //    st->codecpar->codec_tag = 0;
  // 从编码器上下文复制
  avcodec_parameters_from_context(st->codecpar, codec_ctx);
  av_dump_format(fmt_ctx_, 0, url_.c_str(), 1);

  // 判断当前的是视频流还是音频流
  if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
    audio_codec_time_base_ = codec_ctx->time_base;
    audio_stream_ = st;
    LogInfo("MediaMuxer::AddTrack, current track is AVMEDIA_TYPE_AUDIO");
  } else if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
    video_codec_time_base_ = codec_ctx->time_base;
    video_stream_ = st;
    LogInfo("MediaMuxer::AddTrack, current track is AVMEDIA_TYPE_VIDEO");
  }
  return index;
}

bool MediaMuxer::Start() {
  LogInfo("MediaMuxer::Start, url_:%s", url_.c_str());
  int ret = avio_open(&fmt_ctx_->pb, url_.c_str(), AVIO_FLAG_WRITE);
  if (ret < 0) {
    LogInfo("MediaMuxer::Start, avio_open with url_:%s failed!", url_.c_str());
    return false;
  }
  ret = avformat_write_header(fmt_ctx_, NULL);
  if (ret < 0) {
    LogInfo("MediaMuxer::Start, avformat_write_header failed!");
    return false;
  }
  return true;
}

void MediaMuxer::WriteSampleData(int track_index, AVPacket *packet) {
  packet->stream_index = track_index;

  if (!packet || packet->size <= 0 || !packet->data) {
    LogDebug("MediaMuxer::WriteSampleData, packet is invalid");
    if (packet)
      av_packet_free(&packet);
  }

  AVRational src_time_base; // time_base from encoded packet
  AVRational dst_time_base; // time_base from stream
  if (video_stream_ && track_index == video_stream_->index) {
    src_time_base = video_codec_time_base_;
    dst_time_base = video_stream_->time_base;
  } else if (audio_stream_ && track_index == audio_stream_->index) {
    src_time_base = audio_codec_time_base_;
    dst_time_base = audio_stream_->time_base;
  }

  packet->pts = av_rescale_q(packet->pts, src_time_base, dst_time_base);
  packet->dts = av_rescale_q(packet->dts, src_time_base, dst_time_base);
  packet->duration =
      av_rescale_q(packet->duration, src_time_base, dst_time_base);

  int ret = 0;
  ret = av_interleaved_write_frame(
      fmt_ctx_,
      packet); // 不是立即写入文件，内部缓存，主要是对pts进行排序
  //    ret = av_write_frame(fmt_ctx_, packet);
  if (ret != 0) {
    LogDebug("MediaMuxer::WriteSampleData, av_write_frame failed:");
  }
}

bool MediaMuxer::Stop() {
  int ret = av_write_trailer(fmt_ctx_);
  if (ret != 0) {
    LogInfo("MediaMuxer::Stop, av_write_trailer failed:");
    return false;
  }

  return true;
}

} // namespace mm