#pragma once

#include <cstdint>
#include "publicutil.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

  // 8bit
  LIGHTVIDEO_EXPORT void lvFilterSubTop8(uint8_t *data, int width, int height, uint8_t threshold);
  LIGHTVIDEO_EXPORT void lvDefilterSubTop8(uint8_t *data, int width, int height);

  LIGHTVIDEO_EXPORT void lvFilterSubLeft8(uint8_t *data, int width, int height, uint8_t threshold);
  LIGHTVIDEO_EXPORT void lvDefilterSubLeft8(uint8_t *data, int width, int height);

  LIGHTVIDEO_EXPORT void lvFilterSubAvg8(uint8_t *data, int width, int height, uint8_t threshold, uint8_t *workMem);
  LIGHTVIDEO_EXPORT void lvDefilterSubAvg8(uint8_t *data, int width, int height);

  LIGHTVIDEO_EXPORT void lvFilterSubPaeth8(uint8_t *data, int width, int height, uint8_t threshold, uint8_t *workMem);
  LIGHTVIDEO_EXPORT void lvDefilterSubPaeth8(uint8_t *data, int width, int height);

  // 16bit
  LIGHTVIDEO_EXPORT void lvFilterSubTop16(uint16_t *data, int width, int height, uint16_t threshold);
  LIGHTVIDEO_EXPORT void lvDefilterSubTop16(uint16_t *data, int width, int height);

  LIGHTVIDEO_EXPORT void lvFilterSubLeft16(uint16_t *data, int width, int height, uint16_t threshold);
  LIGHTVIDEO_EXPORT void lvDefilterSubLeft16(uint16_t *data, int width, int height);

  LIGHTVIDEO_EXPORT void lvFilterSubAvg16(uint16_t *data, int width, int height, uint16_t threshold, uint16_t *workMem);
  LIGHTVIDEO_EXPORT void lvDefilterSubAvg16(uint16_t *data, int width, int height);

  LIGHTVIDEO_EXPORT void lvFilterSubPaeth16(uint16_t *data, int width, int height, uint16_t threshold, uint16_t *workMem);
  LIGHTVIDEO_EXPORT void lvDefilterSubPaeth16(uint16_t *data, int width, int height);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus