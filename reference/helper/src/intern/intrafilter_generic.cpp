#include "../intrafilter.h"
#include "privateutil.hpp"

using namespace LightVideo;

namespace LightVideo
{
  template<typename T>static inline void lvFilterSubTop_impl(T *data, int width, int height, T threshold)
  {
    if(threshold == 0)
    {
      for(int x = 0; x < width; ++x)
      {
        T t = data[x];
        for(int y = 1; y < height; ++y)
        {
          T v = data[y * width + x] - t;
          t = data[y * width + x];
          data[y * width + x] = v;
        }
      }
    }
    else
    {
      for(int x = 0; x < width; ++x)
      {
        T t = data[x];
        for(int y = 1; y < height; ++y)
        {
          int v = static_cast<int>(data[y * width + x]) - static_cast<int>(t);
          if(data[y * width + x] <= threshold && t > threshold)
            data[y * width + x] = static_cast<T>(v);
          else
            data[y * width + x] = static_cast<T>(absDrop(v, static_cast<int>(threshold)));
          t += data[y * width + x];
        }
      }
    }
  }

  template<typename T>static inline void lvDefilterSubTop_impl(T *data, int width, int height)
  {
    for(int y = 1; y < height; ++y)
    {
      for(int x = 0; x < width; ++x)
        data[y * width + x] += data[(y - 1) * width + x];
    }
  }

  template<typename T>static inline void lvFilterSubLeft_impl(T *data, int width, int height, T threshold)
  {
    if(threshold == 0)
    {
      for(int y = 0; y < height; ++y)
      {
        T t = data[y * width];
        for(int x = 1; x < width; ++x)
        {
          T v = data[y * width + x] - t;
          t = data[y * width + x];
          data[y * width + x] = v;
        }
      }
    }
    else
    {
      for(int y = 0; y < height; ++y)
      {
        T t = data[y * width];
        for(int x = 1; x < width; ++x)
        {
          int v = static_cast<int>(data[y * width + x]) - static_cast<int>(t);
          if(data[y * width + x] <= threshold && t > threshold)
            data[y * width + x] = static_cast<T>(v);
          else
            data[y * width + x] = static_cast<T>(absDrop(v, static_cast<int>(threshold)));
          t += data[y * width + x];
        }
      }
    }
  }

  template<typename T>static inline void lvDefilterSubLeft_impl(T *data, int width, int height)
  {
    for(int y = 0; y < height; ++y)
    {
      for(int x = 1; x < width; ++x)
        data[y * width + x] += data[y * width + x - 1];
    }
  }

  template<typename T>static inline void lvFilterSubAvg_impl(T *data, int width, int height, T threshold, T *workMem)
  {
    WorkMem<T> m(workMem, width);
    workMem = m.get();
    std::copy(data, data + width, workMem);
    if(threshold == 0)
    {
      for(int y = 1; y < height; ++y)
      {
        T t = data[y * width];
        for(int x = 1; x < width; ++x)
        {
          T avg = (static_cast<unsigned int>(t) + static_cast<unsigned int>(workMem[x])) / 2;
          t = workMem[x] = data[y * width + x];
          data[y * width + x] -= avg;
        }
      }
    }
    else
    {
      for(int y = 1; y < height; ++y)
      {
        T t = data[y * width];
        for(int x = 1; x < width; ++x)
        {
          T avg = (static_cast<unsigned int>(t) + static_cast<unsigned int>(workMem[x])) / 2;
          int v = static_cast<int>(data[y * width + x]) - static_cast<int>(avg);
          T delta;
          if(data[y * width + x] <= threshold && avg > threshold)
            delta = static_cast<T>(v);
          else
            delta = static_cast<T>(absDrop(v, static_cast<int>(threshold)));
          t = workMem[x] = avg + delta;
          data[y * width + x] = delta;
        }
      }
    }
  }

  template<typename T>static inline void lvDefilterSubAvg_impl(T *data, int width, int height)
  {
    for(int y = 1; y < height; ++y)
    {
      for(int x = 1; x < width; ++x)
      {
        T avg = (static_cast<unsigned int>(data[(y - 1) * width + x]) + static_cast<unsigned int>(data[y * width + x - 1])) / 2;
        data[y * width + x] += avg;
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

  template<typename T>static inline void lvFilterSubPaeth_impl(T *data, int width, int height, T threshold, T *workMem)
  {
    WorkMem<T> m(workMem, width * height);
    workMem = m.get();
    std::copy(data, data + width * height, workMem);
    if(threshold == 0)
    {
      for(int y = 1; y < height; ++y)
      {
        for(int x = 1; x < width; ++x)
        {
          T filtered = paeth(workMem[y * width + x - 1], workMem[(y - 1) * width + x], workMem[(y - 1) * width + x - 1]);
          data[y * width + x] -= filtered;
        }
      }
    }
    else
    {
      for(int y = 1; y < height; ++y)
      {
        T t = data[y * width];
        for(int x = 1; x < width; ++x)
        {
          T filtered = paeth(workMem[y * width + x - 1], workMem[(y - 1) * width + x], workMem[(y - 1) * width + x - 1]);
          int v = static_cast<int>(data[y * width + x]) - static_cast<int>(filtered);
          T delta;
          if(data[y * width + x] <= threshold && filtered > threshold)
            delta = static_cast<T>(v);
          else
            delta = static_cast<T>(absDrop(v, static_cast<int>(threshold)));
          workMem[y * width + x] = filtered + delta;
          data[y * width + x] = delta;
        }
      }
    }
  }

  template<typename T>static inline void lvDefilterSubPaeth_impl(T *data, int width, int height)
  {
    for(int y = 1; y < height; ++y)
    {
      for(int x = 1; x < width; ++x)
      {
        T filtered = paeth(data[y * width + x - 1], data[(y - 1) * width + x], data[(y - 1) * width + x - 1]);
        data[y * width + x] += filtered;
      }
    }
  }

  // uint8
  void lvFilterSubTop8_generic(uint8_t *data, int width, int height, uint8_t threshold)
  { lvFilterSubTop_impl<uint8_t>(data, width, height, threshold); }
  void lvDefilterSubTop8_generic(uint8_t *data, int width, int height)
  { lvDefilterSubTop_impl<uint8_t>(data, width, height); }

  void lvFilterSubLeft8_generic(uint8_t *data, int width, int height, uint8_t threshold)
  { lvFilterSubLeft_impl<uint8_t>(data, width, height, threshold); }
  void lvDefilterSubLeft8_generic(uint8_t *data, int width, int height)
  { lvDefilterSubLeft_impl<uint8_t>(data, width, height); }

  void lvFilterSubAvg8_generic(uint8_t *data, int width, int height, uint8_t threshold, uint8_t *workMem)
  { lvFilterSubAvg_impl<uint8_t>(data, width, height, threshold, workMem); }
  void lvDefilterSubAvg8_generic(uint8_t *data, int width, int height)
  { lvDefilterSubAvg_impl<uint8_t>(data, width, height); }

  void lvFilterSubPaeth8_generic(uint8_t *data, int width, int height, uint8_t threshold, uint8_t *workMem)
  { lvFilterSubPaeth_impl<uint8_t>(data, width, height, threshold, workMem); }
  void lvDefilterSubPaeth8_generic(uint8_t *data, int width, int height)
  { lvDefilterSubPaeth_impl<uint8_t>(data, width, height); }

  // uint16
  void lvFilterSubTop16_generic(uint16_t *data, int width, int height, uint16_t threshold)
  { lvFilterSubTop_impl<uint16_t>(data, width, height, threshold); }
  void lvDefilterSubTop16_generic(uint16_t *data, int width, int height)
  { lvDefilterSubTop_impl<uint16_t>(data, width, height); }

  void lvFilterSubLeft16_generic(uint16_t *data, int width, int height, uint16_t threshold)
  { lvFilterSubLeft_impl<uint16_t>(data, width, height, threshold); }
  void lvDefilterSubLeft16_generic(uint16_t *data, int width, int height)
  { lvDefilterSubLeft_impl<uint16_t>(data, width, height); }

  void lvFilterSubAvg16_generic(uint16_t *data, int width, int height, uint16_t threshold, uint16_t *workMem)
  { lvFilterSubAvg_impl<uint16_t>(data, width, height, threshold, workMem); }
  void lvDefilterSubAvg16_generic(uint16_t *data, int width, int height)
  { lvDefilterSubAvg_impl<uint16_t>(data, width, height); }

  void lvFilterSubPaeth16_generic(uint16_t *data, int width, int height, uint16_t threshold, uint16_t *workMem)
  { lvFilterSubPaeth_impl<uint16_t>(data, width, height, threshold, workMem); }
  void lvDefilterSubPaeth16_generic(uint16_t *data, int width, int height)
  { lvDefilterSubPaeth_impl<uint16_t>(data, width, height); }
} // namespace LightVideo