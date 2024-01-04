#ifndef RAW_DATA_H
#define RAW_DATA_H
#include <memory>

enum RawDataState {
  RAW_DATA_STATE_BEGIN,
  RAW_DATA_STATE_SENDING,
  RAW_DATA_STATE_END,
  RAW_DATA_STATE_ERROR,
};

struct RawDataBufferInfo {
  RawDataState state;
  std::unique_ptr<uint8_t[]> raw_data;
  int32_t size;
  int64_t time_stamp;
};
#endif // RAW_DATA_H