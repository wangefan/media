#include <fstream>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please provide acc file\n");
    return -1;
  }

  const std::string aac_file_name = argv[1];
  printf("aac file name: %s\n", aac_file_name.c_str());
  std::ifstream file(aac_file_name, std::ios::binary);
  char header[7]; // 7 bytes header
  unsigned int frame_no = 1;

  do {
    memset(header, 0, 7);
    file.read(header, 7);
    if (file.gcount() < 7) {
      printf("Error reading the header.\n");
      break;
    }
    // 解析ADTS头部信息
    unsigned short syncword = ((header[0] << 4) | (header[1] >> 4)) & 0x0FFF;
    unsigned short sampling_freq_idx = (header[2] >> 2) & 0b1111; // uimsbf(4)

    unsigned short frame_length =
        (((header[3] & 0x03) << 11) | (header[4] << 3) |
         ((header[5] & 0xe0) >> 5));
    unsigned char protection_absent = (header[1]) & 0b1;    // bslbf(1)
    unsigned char num_raw_data_blocks = (header[6]) & 0b11; // uimsbf(2)

    if (syncword != 0xFFF) {
      printf("Error: syncword is not 0xFFF.\n");
      break;
    }

    unsigned short raw_data_size = 0;
    if (protection_absent == 0) { // has crc
      raw_data_size = frame_length - (7 + 2 * (num_raw_data_blocks + 1));
    } else {
      raw_data_size = frame_length - 7;
    }
    file.ignore(raw_data_size);
    // 输出解析结果
    printf("frame_no: %d, sampling_freq_idx:%d, frame_length: %d\n", frame_no++,
           sampling_freq_idx, static_cast<int>(frame_length));
  } while (true);

  file.close();
  return 1;
}
