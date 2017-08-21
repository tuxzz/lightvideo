#include "../colorformat.hpp"

namespace LightVideoDecoder
{
  ColorFormatInfo getColorFormatInfo(ColorFormat type, uint32_t width, uint32_t height)
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
      lvdAssert(false);
    }

    out.dataSize = 0;
    for(const Size &s : out.channelList)
      out.dataSize += s.width * s.height;
    out.dataSize *= out.typeSize;
    return out;
  }
};