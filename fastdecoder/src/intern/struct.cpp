#include "../struct.hpp"
#include "util_p.hpp"
#include "../colorformat.hpp"

#include <cstdint>
#include <cstring>

namespace LightVideoDecoder
{
  bool verifyMainStruct(const MainStruct &mainStruct)
  {
    if(strncmp(mainStruct.aria, "ARiA", 4))
    {
      critical("Bad file header(excepted ARiA).");
      return false;
    }
    if(mainStruct.version != 0)
    {
      critical("Unknown version.");
      return false;
    }
    if(mainStruct.colorFormat >= _COLORFORMAT_ENUM_MAX)
    {
      critical("Invalid color format.");
      return false;
    }
    if(mainStruct.framerate <= 0)
    {
      critical("Invalid framerate.");
      return false;
    }
    if(mainStruct.maxPacketSize == 0)
    {
      critical("Invalid maxPacketSize.");
      return false;
    }
    if(mainStruct.width <= 0 && mainStruct.height <= 0)
    {
      critical("Invalid size.");
      return false;
    }

    if(mainStruct.width > 32767 || mainStruct.height > 32767)
    {
      critical("This decoder cannot decode such a large video.");
      return false;
    }

    return true;
  }

  bool verifyVFPK(const MainStruct &mainStruct, const VideoFramePacket &vfpk)
  {
    if(strncmp(vfpk.vfpk, "VFPK", 4) || vfpk.nFrame == 0 || vfpk.nFrame > mainStruct.maxPacketSize || vfpk.nFullFrame > vfpk.nFrame || vfpk.compressionMethod >= _COMPRESSION_ENUM_MAX)
    {
      critical("Video frame packet header is broken.");
      return false;
    }
    return true;
  }

  bool verifyVFRM(const MainStruct &mainStruct, const VideoFrameStruct &vfrm)
  {
    if(strncmp(vfrm.vfrm, "VFRM", 4) || vfrm.referenceType >= _REFERENCETYPE_ENUM_MAX || vfrm.scalePredictValue > mainStruct.maxMovingRadius || -vfrm.scalePredictValue >= static_cast<int64_t>(std::min(mainStruct.width, mainStruct.height)) || vfrm.movePredictValueX > mainStruct.maxScaleRadius || vfrm.movePredictValueY > mainStruct.maxScaleRadius)
    {
      critical("Video frame struct is broken.");
      return false;
    }
    auto colorFormatInfo = getColorFormatInfo(mainStruct.colorFormat, mainStruct.width, mainStruct.height);
    int channelCount = static_cast<int>(colorFormatInfo.channelList.size());
    for(int i = 0; i < channelCount; ++i)
    {
      if(vfrm.intraPredictModeList[i] >= _INTRAPREDICTMODE_ENUM_MAX)
      {
        critical("Video frame struct is broken.");
        return false;
      }
    }
    return true;
  }
} // namespace LightVideoDecoder