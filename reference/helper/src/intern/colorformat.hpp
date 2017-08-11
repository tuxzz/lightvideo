#pragma once

#include <cstdint>
#include <vector>
#include "struct.hpp"
#include "privateutil.hpp"

namespace LightVideo
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
  };

  static ColorFormatInfo getColorFormatInfo(ColorType type, uint32_t width, uint32_t height)
  {
    ColorFormatInfo out;
    switch(type)
    {
    case YUV420P:
      out.typeSize = 1;
      out.channelList.emplace_back(width, height);
      out.channelList.emplace_back(std::max(1U, width / 2), std::max(1U, height / 2));
      out.channelList.emplace_back(std::max(1U, width / 2), std::max(1U, height / 2));
      break;
    case YUVA420P:
      out.typeSize = 1;
      out.channelList.emplace_back(width, height);
      out.channelList.emplace_back(std::max(1U, width / 2), std::max(1U, height / 2));
      out.channelList.emplace_back(std::max(1U, width / 2), std::max(1U, height / 2));
      out.channelList.emplace_back(width, height);
      break;
    default:
      lvAssert(false);
    }
    return out;
  }
}