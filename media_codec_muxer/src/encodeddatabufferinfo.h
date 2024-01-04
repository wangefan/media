#ifndef ENCODED_DATA_H
#define ENCODED_DATA_H
#include <memory>

struct AVPacket;

enum EncodedDataState {
  STATE_BEGIN,
  STATE_SENDING,
  STATE_END,
  STATE_ERROR,
};

struct EncodedDataBufferInfo {
  EncodedDataState state;
  AVPacket *packet;
};
#endif // ENCODED_DATA_H