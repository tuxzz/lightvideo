#pragma once

#include <cstdint>

namespace LightVideo
{
  void lvFilterSubTop8_generic(uint8_t *data, int width, int height, uint8_t threshold);
  void lvDefilterSubTop8_generic(uint8_t *data, int width, int height);
  void lvFilterSubLeft8_generic(uint8_t *data, int width, int height, uint8_t threshold);
  void lvDefilterSubLeft8_generic(uint8_t *data, int width, int height);
  void lvFilterSubAvg8_generic(uint8_t *data, int width, int height, uint8_t threshold, uint8_t *workMem);
  void lvDefilterSubAvg8_generic(uint8_t *data, int width, int height);
  void lvFilterSubPaeth8_generic(uint8_t *data, int width, int height, uint8_t threshold, uint8_t *workMem);
  void lvDefilterSubPaeth8_generic(uint8_t *data, int width, int height);

  void lvFilterSubTop16_generic(uint16_t *data, int width, int height, uint16_t threshold);
  void lvDefilterSubTop16_generic(uint16_t *data, int width, int height);
  void lvFilterSubLeft16_generic(uint16_t *data, int width, int height, uint16_t threshold);
  void lvDefilterSubLeft16_generic(uint16_t *data, int width, int height);
  void lvFilterSubAvg16_generic(uint16_t *data, int width, int height, uint16_t threshold, uint16_t *workMem);
  void lvDefilterSubAvg16_generic(uint16_t *data, int width, int height);
  void lvFilterSubPaeth16_generic(uint16_t *data, int width, int height, uint16_t threshold, uint16_t *workMem);
  void lvDefilterSubPaeth16_generic(uint16_t *data, int width, int height);
} // namespace LightVideo