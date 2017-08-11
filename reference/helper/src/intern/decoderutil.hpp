#pragma once

#include "struct.hpp"
#include "imagechannel.hpp"

namespace LightVideo
{
  void defilterIntra8(ImageChannel<uint8_t> &img, IntraPredictMode mode);
  void defilterIntra16(ImageChannel<uint16_t> &img, IntraPredictMode mode);
  bool verifyMainStruct(const MainStruct &mainStruct);
  bool verifyVFPK(const MainStruct &mainStruct, const VideoFramePacket &vfpk);
  bool verifyVFRM(const MainStruct &mainStruct, const VideoFrameStruct &vfrm);

  template<typename T>inline void defilterIntra(ImageChannel<T> &img, IntraPredictMode mode)
  { static_assert(false, "Invalid T"); }
  template<>inline void defilterIntra<uint8_t>(ImageChannel<uint8_t> &img, IntraPredictMode mode)
  { defilterIntra8(img, mode); }
  template<>inline void defilterIntra<uint16_t>(ImageChannel<uint16_t> &img, IntraPredictMode mode)
  { defilterIntra16(img, mode); }

  template<typename T>
  void defilterDelta(ImageChannel<T> &img, const ImageChannel<T> &ref, int scale, int moveX, int moveY, IntraPredictMode mode)
  {
    lvAssert(img.width() == ref.width() && img.height() == ref.height(), "Shape dismatch");
    int64_t width = img.width(), height = img.height();
    int64_t nScaledW, nScaledH;
    if(height < width)
    {
      nScaledH = std::max(1LL, height + scale);
      nScaledW = std::max(1LL, width * nScaledH / height);
    }
    else
    {
      nScaledW = std::max(1LL, width + scale);
      nScaledH = std::max(1LL, height * nScaledW / width);
    }

    defilterIntra(img, mode);

    int64_t halfDiffW = (nScaledW - width) / 2;
    int64_t halfDiffH = (nScaledH - height) / 2;
    for(int i = 0; i < height; ++i)
    {
      for(int j = 0; j < width; ++j)
      {
        uint32_t mappedX = static_cast<uint32_t>(clip(0LL, (j + halfDiffW - moveX) * width / nScaledW, width - 1));
        uint32_t mappedY = static_cast<uint32_t>(clip(0LL, (i + halfDiffH - moveY) * height / nScaledH, height - 1));
        T refV = ref(mappedY, mappedX);
        img(i, j) += refV;
      }
    }
  }
}