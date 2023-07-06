#include <iostream>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rect.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
using namespace std;

int main(int argc, char *argv[]) {
  AVFormatContext *p_fmt_ctx = NULL;      // demux
  AVCodecContext *p_codec_ctx = NULL;     // ctx to decode
  AVCodecParameters *p_codec_par = NULL;  
  AVCodec *p_codec = NULL;            
  AVPacket *p_packet = NULL; // encoded packet from demuxer
  AVFrame *p_frm_raw = NULL; // raw frame from decoded file
  AVFrame *p_frm_yuv = NULL; // raw frame after re-sampler

  int                 ret;

  SDL_Window *screen;
  SDL_Renderer *sdl_renderer;
  SDL_Texture *sdl_texture;
  SDL_Rect sdl_rect;

  // 0.
  if (argc < 2) {
    printf("Please provide a movie file\n");
    return -1;
  }

  // 1. Register all formats and codecs
  // av_register_all();

  // 2. Open input file
  std::string input_file(argv[1]);
  ret = avformat_open_input(&p_fmt_ctx, input_file.c_str(), NULL, NULL);
  if (ret != 0) {
    printf("avformat_open_input() failed\n");
    return -1;
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
    printf("SDL_Init() failed: %s\n", SDL_GetError());
    return -1;
  }
  screen = SDL_CreateWindow("Simplest ffmpeg player's Window",
                            SDL_WINDOWPOS_UNDEFINED, // 不关心窗口X坐标
                            SDL_WINDOWPOS_UNDEFINED, // 不关心窗口Y坐标
                            360, 240, SDL_WINDOW_OPENGL);

  if (screen == NULL) {
    printf("SDL_CreateWindow() failed: %s\n", SDL_GetError());
    return -1;
  }
  getchar();
  return 0;
}
