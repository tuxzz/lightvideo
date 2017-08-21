#include "decoderutil.hpp"
#include "privateutil.hpp"
#include "../intrafilter.h"
#include "colorformat.hpp"
#include <algorithm>
#include <cstring>

namespace LightVideo
{
  void defilterIntra8(ImageChannel<uint8_t> &img, IntraPredictMode mode)
  {
    switch(mode)
    {
    case SubTop:
      lvDefilterSubTop8(img.data(), img.width(), img.height());
      break;
    case SubLeft:
      lvDefilterSubLeft8(img.data(), img.width(), img.height());
      break;
    case SubAvg:
      lvDefilterSubAvg8(img.data(), img.width(), img.height());
      break;
    case SubPaeth:
      lvDefilterSubPaeth8(img.data(), img.width(), img.height());
      break;
    case NoIntraPredict:
      break;
    default:
      lvAssert(false);
    }
  }

  void defilterIntra16(ImageChannel<uint16_t> &img, IntraPredictMode mode)
  {
    switch(mode)
    {
    case SubTop:
      lvDefilterSubTop16(img.data(), img.width(), img.height());
      break;
    case SubLeft:
      lvDefilterSubLeft16(img.data(), img.width(), img.height());
      break;
    case SubAvg:
      lvDefilterSubAvg16(img.data(), img.width(), img.height());
      break;
    case SubPaeth:
      lvDefilterSubPaeth16(img.data(), img.width(), img.height());
      break;
    case NoIntraPredict:
      break;
    default:
      lvAssert(false);
    }
  }

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
    if(mainStruct.colorFormat != YUV420P && mainStruct.colorFormat != YUVA420P)
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
    if(mainStruct.width > static_cast<uint32_t>(std::numeric_limits<int>::max()) || mainStruct.height > static_cast<uint32_t>(std::numeric_limits<int>::max()))
    {
      critical("This decoder cannot decode such a large video.");
      return false;
    }

    return true;
  }

  bool verifyVFPK(const MainStruct &mainStruct, const VideoFramePacket &vfpk)
  {
    if(strncmp(vfpk.vfpk, "VFPK", 4) || vfpk.nFrame == 0 || vfpk.nFrame > mainStruct.maxPacketSize || vfpk.nFullFrame > vfpk.nFrame || (vfpk.compressionMethod != LZ4Compression && vfpk.compressionMethod != NoCompression))
    {
      critical("Video frame packet header is broken.");
      return false;
    }
    return true;
  }

  bool verifyVFRM(const MainStruct &mainStruct, const VideoFrameStruct &vfrm)
  {
    if(strncmp(vfrm.vfrm, "VFRM", 4) || vfrm.referenceType > PreviousReference || vfrm.scalePredictValue > mainStruct.maxMovingRadius || -vfrm.scalePredictValue >= static_cast<int64_t>(std::min(mainStruct.width, mainStruct.height)) || vfrm.movePredictValueX > mainStruct.maxScaleRadius || vfrm.movePredictValueY > mainStruct.maxScaleRadius)
    {
      critical("Video frame struct is broken.");
      return false;
    }
    auto colorFormatInfo = getColorFormatInfo(mainStruct.colorFormat, mainStruct.width, mainStruct.height);
    int channelCount = static_cast<int>(colorFormatInfo.channelList.size());
    for(int i = 0; i < channelCount; ++i)
    {
      if(vfrm.intraPredictModeList[i] > SubPaeth)
      {
        critical("Video frame struct is broken.");
        return false;
      }
    }
    return true;
  }
}