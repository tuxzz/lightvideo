#include "../astcwrapper.h"
#include "astc.h"
#include <cstdio>
#include <cstdlib>

static int blockSizeList[14][2] = {
  {4, 4}, {5, 4}, {5, 5}, {6, 5},
  {6, 6}, {8, 5}, {8, 6}, {10, 5},
  {10, 6}, {8, 8}, {10, 8}, {10, 10},
  {12, 10}, {12, 12}
};

static const char *blockSizeStrList[] = {
  "4x4", "5x4", "5x5", "6x5",
  "6x6", "8x5", "8x6", "10x5",
  "10x6", "8x8", "10x8", "10x10",
  "12x10", "12x12"
};

static const char *encodeLevelStr[] = {
  "-veryfast",
  "-fast",
  "-medium",
  "-thorough",
  "-exhaustive"
};

static const char *blockSizeToStr(int blockWidth, int blockHeight)
{
  for(int i = 0; i < sizeof(blockSizeList) / 4 / 2; ++i)
  {
    if(blockSizeList[i][0] == blockWidth && blockSizeList[i][1] == blockHeight)
      return blockSizeStrList[i];
  }
  fprintf(stderr, "invalid block size\n");
  std::abort();
}

static const char *encodeLevelToStr(int encodeLevel)
{
  if(encodeLevel < 0x0 || encodeLevel > 0x4)
  {
    fprintf(stderr, "invalid encode level\n");
    std::abort();
  }
  return encodeLevelStr[encodeLevel];
}

int lvGetASTCCompressedSize(const LvASTCInformation &info)
{ return astc_calc_compressed_size(info.width, info.height, info.blockWidth, info.blockHeight); }

LvASTCInformation lvGetASTCInformation(const uint8_t *data)
{
  auto info = astc_get_header_info(const_cast<uint8_t*>(data));
  return {info.width, info.height, info.blockWidth, info.blockHeight};
}

int lvEncodeASTC8(const uint8_t *img, int width, int height, int blockWidth, int blockHeight, int encodeLevel, uint8_t *out)
{
  const char *argv[6] = {"", "-cl", "", "", blockSizeToStr(blockWidth, blockHeight), encodeLevelToStr(encodeLevel)};
  return astc_main(6, const_cast<char**>(argv), width, height, const_cast<uint8_t*>(img), out);
}

int lvDecodeASTC8(const uint8_t *data, uint8_t *out)
{
  const char *argv[4] = {"", "-dl", "", ""};
  return astc_main(4, const_cast<char**>(argv), -1, -1, const_cast<uint8_t*>(data), out);
}

int lvEncodeASTC16(const LvFloat16 *img, int width, int height, int blockWidth, int blockHeight, int encodeLevel, uint8_t *out)
{
  const char *argv[6] = {"", "-c", "", "", blockSizeToStr(blockWidth, blockHeight), encodeLevelToStr(encodeLevel)};
  int ret = astc_main(6, const_cast<char**>(argv), width, height, const_cast<LvFloat16*>(img), out);
  printf("\n");
  return ret;
}

int lvDecodeASTC16(const uint8_t *data, LvFloat16 *out)
{
  const char *argv[4] = {"", "-d", "", ""};
  int ret = astc_main(4, const_cast<char**>(argv), -1, -1, const_cast<uint8_t*>(data), out);
  printf("\n");
  return ret;
}