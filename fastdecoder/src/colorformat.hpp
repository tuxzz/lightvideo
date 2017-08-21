#pragma once

#include "struct.hpp"
#include <vector>
#include <algorithm>
#include "util.hpp"

namespace LightVideoDecoder
{
  struct Size
  {
    Size(uint32_t w, uint32_t h)
      : width(w), height(h)
    {}
    uint32_t width, height;
  };

  struct ColorFormatInfo
  {
    uint32_t typeSize;
    std::vector<Size> channelList;
    uint32_t dataSize;
  };

  ColorFormatInfo getColorFormatInfo(ColorFormat type, uint32_t width, uint32_t height);
} // namespace LightVideoDecoder