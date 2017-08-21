#pragma once

#include <cstdint>
#include <immintrin.h>
#include <xmmintrin.h>
#include "../imagechannel.hpp"

namespace LightVideoDecoder
{
  /* generic */
  template<typename T>static void defilterSubTop(ImageChannel<T> &img)
  { static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type"); }
  template<typename T>static void defilterSubLeft(ImageChannel<T> &img)
  { static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type"); }
  
  template<typename T>static void defilterSubAvg(ImageChannel<T> &img) // seems impossable to vectorize
  {
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

  template<typename T>void defilterSubPaeth(ImageChannel<T> &img) // seems impossable to vectorize
  {
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
  static void defilterDelta(ImageChannel<T> &img, const ImageChannel<T> &ref, int scale, int moveX, int moveY) // not vectorized, use glsl instead
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

  /* uint8_t simd */
  template<>void defilterSubTop<uint8_t>(ImageChannel<uint8_t> &img)
  {
    int width = img.width();
    int height = img.height();
    if(width < 32)
    {
      for(int y = 1; y < height; ++y)
      {
        for(int x = 0; x < width; ++x)
          img(y, x) += img(y - 1, x);
      }
    }
    else
    {
      for(int y = 1; y < height; ++y)
      {
        int alignDiff = (32 - (reinterpret_cast<size_t>(img.data()) % 32)) % 32;
        for(int x = 0; x < alignDiff; ++x)
          img(y, x) += img(y - 1, x);
        for(int x = alignDiff; x < width - (width - alignDiff) % 32; x += 32)
        {
          __m256i a = _mm256_load_si256(reinterpret_cast<__m256i*>(&img(y - 1, x)));
          __m256i b = _mm256_load_si256(reinterpret_cast<__m256i*>(&img(y, x)));
          a = _mm256_add_epi8(a, b);
          _mm256_store_si256(reinterpret_cast<__m256i*>(&img(y, x)), a);
        }
        for(int x = width - (width - alignDiff) % 32; x < width; ++x)
          img(y, x) += img(y - 1, x);
      }
    }
  }

  template<>void defilterSubLeft<uint8_t>(ImageChannel<uint8_t> &img)
  {
    int width = img.width();
    int height = img.height();
    if(width <= 32)
    {
      for(int y = 0; y < height; ++y)
      {
        for(int x = 1; x < width; ++x)
          img(y, x) += img(y, x - 1);
      }
    }
    else
    {
      for(int y = 0; y < height; ++y)
      {
        int alignDiff = 32 - reinterpret_cast<size_t>(&img(y, 0)) % 32;
        for(int x = 1; x < alignDiff; ++x)
          img(y, x) += img(y, x - 1);
        for(int x = alignDiff; x < width - (width - alignDiff) % 32; x += 32)
        {
          __m256i p0, s0;
          p0 = _mm256_load_si256(reinterpret_cast<__m256i*>(&img(y, x)));

          s0 = _mm256_alignr_epi8(p0, _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0)), 16 - 1);
          p0 = _mm256_add_epi8(p0, s0);
          s0 = _mm256_alignr_epi8(p0, _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0)), 16 - 2);
          p0 = _mm256_add_epi8(p0, s0);
          s0 = _mm256_alignr_epi8(p0, _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0)), 16 - 4);
          p0 = _mm256_add_epi8(p0, s0);
          s0 = _mm256_alignr_epi8(p0, _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0)), 16 - 8);
          p0 = _mm256_add_epi8(p0, s0);
          s0 = _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0));
          p0 = _mm256_add_epi8(p0, s0);

          s0 = _mm256_set1_epi8(img(y, x - 1));
          p0 = _mm256_add_epi8(p0, s0);

          _mm256_store_si256(reinterpret_cast<__m256i*>(&img(y, x)), p0);
        }
        for(int x = width - (width - alignDiff) % 32; x < width; ++x)
          img(y, x) += img(y, x - 1);
      }
    }
  }

  /* uint16_t simd */
  template<>void defilterSubTop<uint16_t>(ImageChannel<uint16_t> &img)
  {
    lvdAssert(reinterpret_cast<size_t>(img.data()) % 2 == 0, "data must be aligned with its size.");
    int width = img.width();
    int height = img.height();
    if(width < 16)
    {
      for(int y = 1; y < height; ++y)
      {
        for(int x = 0; x < width; ++x)
          img(y, x) += img(y - 1, x);
      }
    }
    else
    {
      for(int y = 1; y < height; ++y)
      {
        int alignDiff = (16 - (reinterpret_cast<size_t>(img.data()) % 32) / 2) % 16;
        for(int x = 0; x < alignDiff; ++x)
          img(y, x) += img(y - 1, x);
        for(int x = alignDiff; x < width - (width - alignDiff) % 16; x += 16)
        {
          __m256i a = _mm256_load_si256(reinterpret_cast<__m256i*>(&img(y - 1, x)));
          __m256i b = _mm256_load_si256(reinterpret_cast<__m256i*>(&img(y, x)));
          a = _mm256_add_epi16(a, b);
          _mm256_store_si256(reinterpret_cast<__m256i*>(&img(y, x)), a);
        }
        for(int x = width - (width - alignDiff) % 16; x < width; ++x)
          img(y, x) += img(y - 1, x);
      }
    }
  }

  template<>void defilterSubLeft<uint16_t>(ImageChannel<uint16_t> &img)
  {
    lvdAssert(reinterpret_cast<size_t>(img.data()) % 2 == 0, "data must be aligned with its size.");
    int width = img.width();
    int height = img.height();
    if(width <= 16)
    {
      for(int y = 0; y < height; ++y)
      {
        for(int x = 1; x < width; ++x)
          img(y, x) += img(y, x - 1);
      }
    }
    else
    {
      for(int y = 0; y < height; ++y)
      {
        int alignDiff = 16 - (reinterpret_cast<size_t>(&img(y, 0)) % 32) / 2;
        for(int x = 1; x < alignDiff; ++x)
          img(y, x) += img(y, x - 1);
        for(int x = alignDiff; x < width - (width - alignDiff) % 16; x += 16)
        {
          __m256i p0, s0;
          p0 = _mm256_load_si256(reinterpret_cast<__m256i*>(&img(y, x)));

          s0 = _mm256_alignr_epi8(p0, _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0)), 16 - 2);
          p0 = _mm256_add_epi16(p0, s0);
          s0 = _mm256_alignr_epi8(p0, _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0)), 16 - 4);
          p0 = _mm256_add_epi16(p0, s0);
          s0 = _mm256_alignr_epi8(p0, _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0)), 16 - 8);
          p0 = _mm256_add_epi16(p0, s0);
          s0 = _mm256_permute2x128_si256(p0, p0, _MM_SHUFFLE(0, 0, 2, 0));
          p0 = _mm256_add_epi16(p0, s0);

          s0 = _mm256_set1_epi16(img(y, x - 1));
          p0 = _mm256_add_epi16(p0, s0);

          _mm256_store_si256(reinterpret_cast<__m256i*>(&img(y, x)), p0);
        }
        for(int x = width - (width - alignDiff) % 16; x < width; ++x)
          img(y, x) += img(y, x - 1);
      }
    }
  }
} // namespace LightVideoDecoder