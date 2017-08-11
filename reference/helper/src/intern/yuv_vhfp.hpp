#pragma once

#include <cstdint>
#include "privateutil.hpp"

namespace LightVideo
{
  void yuvp2rgbi8_vhfp(const uint8_t *LV_RESTRICT y, const uint8_t *LV_RESTRICT u, const uint8_t *LV_RESTRICT v, uint8_t *LV_RESTRICT rgb, int stride, int width, int height);
  void rgbi2yuvp8_vhfp(const uint8_t *LV_RESTRICT rgb, int stride, uint8_t *LV_RESTRICT y, uint8_t *LV_RESTRICT u, uint8_t *LV_RESTRICT v, int width, int height);
  void yuvap2rbgai8_vhfp(const uint8_t *LV_RESTRICT y, const uint8_t *LV_RESTRICT u, const uint8_t *LV_RESTRICT v, const uint8_t *LV_RESTRICT a, uint8_t *LV_RESTRICT rgba, int stride, int width, int height);
  void rgbai2yuvap8_vhfp(const uint8_t *LV_RESTRICT rgba, int stride, uint8_t *LV_RESTRICT y, uint8_t *LV_RESTRICT u, uint8_t *LV_RESTRICT v, uint8_t *LV_RESTRICT a, int width, int height);

  void yuvp2rgbi16_vhfp(const uint16_t *LV_RESTRICT y, const uint16_t *LV_RESTRICT u, const uint16_t *LV_RESTRICT v, uint16_t *LV_RESTRICT rgb, int stride, int width, int height);
  void rgbi2yuvp16_vhfp(const uint16_t *LV_RESTRICT rgb, int stride, uint16_t *LV_RESTRICT y, uint16_t *LV_RESTRICT u, uint16_t *LV_RESTRICT v, int width, int height);
  void yuvap2rbgai16_vhfp(const uint16_t *LV_RESTRICT y, const uint16_t *LV_RESTRICT u, const uint16_t *LV_RESTRICT v, const uint16_t *LV_RESTRICT a, uint16_t *LV_RESTRICT rgba, int stride, int width, int height);
  void rgbai2yuvap16_vhfp(const uint16_t *LV_RESTRICT rgba, int stride, uint16_t *LV_RESTRICT y, uint16_t *LV_RESTRICT u, uint16_t *LV_RESTRICT v, uint16_t *LV_RESTRICT a, int width, int height);
} // namespace LightVideo
