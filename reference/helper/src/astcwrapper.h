#pragma once

#include "publicutil.h"
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
  enum LvASTCEncodeLevel
  {
    lvVeryFast = 0x0,
    lvFast,
    lvMedium,
    lvGood,
    lvExterme
  };

  struct LvASTCInformation
  {
    int width, height;
    int blockWidth, blockHeight;
  };

  typedef uint16_t LvFloat16;

  LIGHTVIDEO_EXPORT int lvGetASTCCompressedSize(const LvASTCInformation &info);
  LIGHTVIDEO_EXPORT LvASTCInformation lvGetASTCInformation(const uint8_t *data);

  LIGHTVIDEO_EXPORT int lvEncodeASTC8(const uint8_t *img, int width, int height, int blockWidth, int blockHeight, int encodeLevel, uint8_t *out);
  LIGHTVIDEO_EXPORT int lvDecodeASTC8(const uint8_t *data, uint8_t *out);
  
  LIGHTVIDEO_EXPORT int lvEncodeASTC16(const LvFloat16 *img, int width, int height, int blockWidth, int blockHeight, int encodeLevel, uint8_t *out);
  LIGHTVIDEO_EXPORT int lvDecodeASTC16(const uint8_t *data, LvFloat16 *out);
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus