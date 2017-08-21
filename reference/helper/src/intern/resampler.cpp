#include "../resampler.hpp"
#include "privateutil.hpp"

using namespace LightVideo;

template<typename T>void lvMontionResmpleImpl(const T *LV_RESTRICT src, int width, int height, int scale, int moveX, int moveY, T *LV_RESTRICT out)
{
  lvAssert(width > 0 && height > 0, "width and height must be greater than 0");
  int nScaledW, nScaledH;
  if(height < width)
  {
    nScaledH = std::max(1, height + scale);
    nScaledW = std::max(1, static_cast<int>(static_cast<int64_t>(width) * static_cast<int64_t>(nScaledH) / static_cast<int64_t>(height)));
  }
  else
  {
    nScaledW = std::max(1, width + scale);
    nScaledH = std::max(1, static_cast<int>(static_cast<int64_t>(height) * static_cast<int64_t>(nScaledW) / static_cast<int64_t>(width)));
  }

  int halfDiffW = (nScaledW - width) / 2;
  int halfDiffH = (nScaledH - height) / 2;
  for(int i = 0; i < height; ++i)
  {
    for(int j = 0; j < width; ++j)
    {
      int mappedX = clip(0, (j + halfDiffW - moveX) * width / nScaledW, width - 1);
      int mappedY = clip(0, (i + halfDiffH - moveY) * height / nScaledH, height - 1);
      T ref = src[mappedY * height + mappedX];
      out[i * height + j] = ref;
    }
  }
}

void lvMotionResample8(const uint8_t *src, int width, int height, int scale, int moveX, int moveY, uint8_t *out)
{ lvMontionResmpleImpl(src, width, height, scale, moveX, moveY, out); }

void lvMotionResample16(const uint16_t *src, int width, int height, int scale, int moveX, int moveY, uint16_t *out)
{ lvMontionResmpleImpl(src, width, height, scale, moveX, moveY, out); }