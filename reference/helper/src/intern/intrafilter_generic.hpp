#include "privateutil.hpp"
#include <cstdint>

using namespace LightVideo;

namespace LightVideo
{
  template<typename T>static inline void filterSubTop(T *data, int width, int height, T threshold)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    auto workMem = GUARDED_ALLOC(typename std::remove_pointer<decltype(data)>::type, width * height);
    std::copy(data, data + width * height, workMem);
    if(threshold == 0)
    {
      for(int y = 1; y < height; ++y)
      {
        for(int x = 0; x < width; ++x)
          data[y * width + x] -= workMem[(y - 1) * width + x];
      }
    }
    else
    {
      for(int y = 1; y < height; ++y)
      {
        for(int x = 0; x < width; ++x)
        {
          int v = static_cast<int>(data[y * width + x]) - workMem[(y - 1) * width + x];
          if(data[y * width + x] <= threshold && workMem[(y - 1) * width + x] > threshold)
            data[y * width + x] = static_cast<uint8_t>(v);
          else
            data[y * width + x] = static_cast<uint8_t>(absDrop(v, static_cast<int>(threshold)));
          workMem[y * width + x] = workMem[(y - 1) * width + x] + data[y * width + x];
        }
      }
    }
    lvFree(workMem);
  }

  template<typename T>static inline void defilterSubTop(T *data, int width, int height, int bpp)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    lvAssert(bpp > 0 && bpp <= 4, "bpp must be in range [1, 4]");
    for(int y = 1; y < height; ++y)
    {
      for(int xb = 0; xb < width * bpp; ++xb)
        data[y * width * bpp + xb] += data[(y - 1) * width * bpp + xb];
    }
  }

  template<typename T>static inline void filterSubLeft(T *data, int width, int height, T threshold)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
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

  template<typename T>static inline void defilterSubLeft(T *data, int width, int height, int bpp)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    lvAssert(bpp > 0 && bpp <= 4, "bpp must be in range [1, 4]");
    for(int b = 0; b < bpp; ++b)
    {
      for(int y = 0; y < height; ++y)
      {
        for(int xb = b + bpp; xb < width * bpp; xb += bpp)
          data[y * width * bpp + xb] += data[y * width * bpp + (xb - bpp)];
      }
    }
  }

  template<typename T>static inline void filterSubAvg(T *data, int width, int height, T threshold)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    auto workMem = GUARDED_ALLOC(typename std::remove_pointer<decltype(data)>::type, width * height);
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
    lvFree(workMem);
  }

  template<typename T>static inline void defilterSubAvg(T *data, int width, int height, int bpp)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    lvAssert(bpp > 0 && bpp <= 4, "bpp must be in range [1, 4]");
    for(int b = 0; b < bpp; ++b)
    {
      for(int y = 1; y < height; ++y)
      {
        for(int xb = b + bpp; xb < width * bpp; xb += bpp)
        {
          T avg = (static_cast<unsigned int>(data[(y - 1) * width * bpp + xb]) + static_cast<unsigned int>(data[y * width * bpp + xb - bpp])) / 2;
          data[y * width * bpp + xb] += avg;
        }
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

  template<typename T>static inline void filterSubPaeth(T *data, int width, int height, T threshold)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    auto workMem = GUARDED_ALLOC(typename std::remove_pointer<decltype(data)>::type, width * height);
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
    lvFree(workMem);
  }

  template<typename T>static inline void defilterSubPaeth(T *data, int width, int height, int bpp)
  {
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value, "unsupported data type");
    lvAssert(bpp > 0 && bpp <= 4, "bpp must be in range [1, 4]");
    for(int b = 0; b < bpp; ++b)
    {
      for(int y = 1; y < height; ++y)
      {
        auto left = data[y * width * bpp + b];
        auto lu = data[(y - 1) * width * bpp + b];
        for(int xb = b + bpp; xb < width * bpp; xb += bpp)
        {
          auto up = data[(y - 1) * width * bpp + xb];
          auto filtered = paeth(left, up, lu);
          uint8_t v = data[y * width * bpp + xb] + filtered;
          data[y * width * bpp + xb] = v;
          left = v;
          lu = up;
        }
      }
    }
  }
} // namespace LightVideo