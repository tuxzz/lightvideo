#pragma once

#include <cstdint>
#include <cmath>
#include "../imagechannel.hpp"

namespace LightVideoDecoder
{

  template<typename T>static void defilterSubTop(ImageChannel<T> &img)
  {
    int width = img.width();
    int height = img.height();
    for(int y = 1; y < height; ++y)
    {
      for(int x = 0; x < width; ++x)
        img(y, x) += img(y - 1, x);
    }
  }

  template<typename T>static inline void defilterSubLeft(ImageChannel<T> &img)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    int width = img.width();
    int height = img.height();
    for(int y = 0; y < height - height % 4; y += 4)
    {
      for(int x = 1; x < width; ++x)
      {
        img(y + 0, x) += img(y + 0, x - 1);
        img(y + 1, x) += img(y + 1, x - 1);
        img(y + 2, x) += img(y + 2, x - 1);
        img(y + 3, x) += img(y + 3, x - 1);
      }
    }
    for(int y = height - height % 4; y < height; ++y)
    {
      for(int x = 1; x < width; ++x)
        img(y, x) += img(y, x - 1);
    }
  }

  template<typename T>static void defilterSubAvg(ImageChannel<T> &img)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    int width = img.width();
    int height = img.height();
    for(int y = 1; y < height; ++y)
    {
      for(int x = 1; x < width; ++x)
      {
        T avg = (static_cast<unsigned int>(img(y - 1, x)) + static_cast<unsigned int>(img(y, x - 1))) / 2;
        img(y, x) += avg;
      }
    }
  }

  template<typename T>static inline T paeth(T _a, T _b, T _c)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    // a = left, b = above, c = upper left
    int a = static_cast<int>(_a);
    int b = static_cast<int>(_b);
    int c = static_cast<int>(_c);
    int p = a + b - c;
    int pa = std::abs(p - a);
    int pb = std::abs(p - b);
    int pc = std::abs(p - c);
    if(pa < pb && pa < pc)
      return _a;
    else if(pb < pc)
      return _b;
    else
      return _c;
  }

  template<typename T>static void defilterSubPaeth(ImageChannel<T> &img)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    int width = img.width();
    int height = img.height();
    for(int y = 1; y < height; ++y)
    {
      auto left = img(y, 0);
      auto lu = img(y - 1, 0);
      for(int x = 1; x < width; ++x)
      {
        auto up = img(y - 1, x);
        auto filtered = paeth(left, up, lu);
        uint8_t v = img(y, x) + filtered;
        img(y, x) = v;
        left = v;
        lu = up;
      }
    }
  }

  template<typename T>
  static void defilterDelta(ImageChannel<T> &img, const ImageChannel<T> &ref, int scale, int moveX, int moveY)
  {
    lvdAssert(img.width() == ref.width() && img.height() == ref.height(), "Shape dismatch");
    lvdAssert(img.width() <= 32767 && img.height() <= 32767, "Input is too large");
    int width = img.width(), height = img.height();
    if(scale != 0 || moveX != 0 || moveY == 0)
    {
      int nScaledW, nScaledH;
      if(height < width)
      {
        nScaledH = std::max(1, height + scale);
        nScaledW = std::max(1, width * nScaledH / height);
      }
      else
      {
        nScaledW = std::max(1, width + scale);
        nScaledH = std::max(1, height * nScaledW / width);
      }

      int halfDiffW = (nScaledW - width) / 2;
      int halfDiffH = (nScaledH - height) / 2;
      for(int i = 0; i < height; ++i)
      {
        for(int j = 0; j < width; ++j)
        {
          uint32_t mappedX = static_cast<uint32_t>(clip(0, (j + halfDiffW - moveX) * width / nScaledW, width - 1));
          uint32_t mappedY = static_cast<uint32_t>(clip(0, (i + halfDiffH - moveY) * height / nScaledH, height - 1));
          T refV = ref(mappedY, mappedX);
          img(i, j) += refV;
        }
      }
    }
    else
    {
      for(int i = 0; i < height; ++i)
      {
        for(int j = 0; j < width; ++j)
          img(i, j) += ref(i, j);
      }
    }
  }
} // namespace LightVideoDecoder