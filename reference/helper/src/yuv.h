#pragma once

/*
* ITU-T T.871
* see https://en.wikipedia.org/wiki/YCbCr
*
* Y = 0.299 * R     + 0.587 * G    + 0.114 * B
* U = -0.168736 * R - 0.331264 * G + 0.5 * B      + 0.5
* V = 0.5 * R       - 0.418688 * G - 0.081312 * B + 0.5
* where R,G,B are both in range [0.0, 1.0]
*
* U' = U - 0.5
* V' = V - 0.5
* R = Y - 1.21889418896677e-6 * U' + 1.40199958865734 * V'
* G = Y - 0.344135678165337 * U'   - 0.714136155581813 * V'
* B = Y + 1.77200006607382 * U'    + 4.06298062659511e-7 * V'
* where Y,U,V are both in range [0.0, 1.0]
*/

#include "publicutil.h"
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
  LIGHTVIDEO_EXPORT void lvYUVP2RGBI8(const uint8_t *y, const uint8_t *u, const uint8_t *v, uint8_t *rgb, int outputStride, int width, int height);
  LIGHTVIDEO_EXPORT void lvRGBI2YUVP8(const uint8_t *rgb, int inputStride, uint8_t *y, uint8_t *u, uint8_t *v, int width, int height);
  LIGHTVIDEO_EXPORT void lvYUVAP2RGBAI8(const uint8_t *y, const uint8_t *u, const uint8_t *v, const uint8_t *a, uint8_t *rgba, int outputStride, int width, int height);
  LIGHTVIDEO_EXPORT void lvRGBAI2YUVAP8(const uint8_t *rgba, int inputStride, uint8_t *y, uint8_t *u, uint8_t *v, uint8_t *a, int width, int height);

  LIGHTVIDEO_EXPORT void lvYUVP2RGBI16(const uint16_t *y, const uint16_t *u, const uint16_t *v, uint16_t *rgb, int outputStride, int width, int height);
  LIGHTVIDEO_EXPORT void lvRGBI2YUVP16(const uint16_t *rgb, int inputStride, uint16_t *y, uint16_t *u, uint16_t *v, int width, int height);
  LIGHTVIDEO_EXPORT void lvYUVAP2RGBAI16(const uint16_t *y, const uint16_t *u, const uint16_t *v, const uint16_t *a, uint16_t *rgba, int outputStride, int width, int height);
  LIGHTVIDEO_EXPORT void lvRGBAI2YUVAP16(const uint16_t *rgba, int inputStride, uint16_t *y, uint16_t *u, uint16_t *v, uint16_t *a, int width, int height);
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus