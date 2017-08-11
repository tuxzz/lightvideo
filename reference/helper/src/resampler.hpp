#pragma once

#include "publicutil.h"
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

  LIGHTVIDEO_EXPORT void lvMotionResample8(const uint8_t *src, int width, int height, int scale, int moveX, int moveY, uint8_t *out);
  LIGHTVIDEO_EXPORT void lvMotionResample16(const uint16_t *src, int width, int height, int scale, int moveX, int moveY, uint16_t *out);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus