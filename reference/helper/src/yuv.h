#pragma once

/*
* YCoCg
*/

#include "publicutil.h"
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
  LIGHTVIDEO_EXPORT void lvYUVP2RGBI8(const uint8_t *y, const uint8_t *u, const uint8_t *v, uint8_t *rgb, int outputStride, int width, int height);
  LIGHTVIDEO_EXPORT void lvRGBI2YUVP8(const uint8_t *rgb, int inputStride, uint8_t *y, uint8_t *u, uint8_t *v, int width, int height);

  LIGHTVIDEO_EXPORT void lvYUVP2RGBI16(const uint16_t *y, const uint16_t *u, const uint16_t *v, uint16_t *rgb, int outputStride, int width, int height);
  LIGHTVIDEO_EXPORT void lvRGBI2YUVP16(const uint16_t *rgb, int inputStride, uint16_t *y, uint16_t *u, uint16_t *v, int width, int height);
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus