#pragma once

#include <cstdint>
#include "util.hpp"

namespace LightVideoDecoder
{
  template<typename T>class ImageChannel
  {
  public:
    constexpr ImageChannel()
      : m_data(nullptr), m_width(0), m_height(0), m_selfAlloc(false)
    {}

    constexpr ImageChannel(T *ptr, uint32_t w, uint32_t h)
      : m_data(ptr), m_width(w), m_height(h), m_selfAlloc(false)
    {
      if(w > 0 && h > 0)
        lvdAssert(m_data, "No data pointer given");
    }

    constexpr ImageChannel(uint32_t w, uint32_t h)
      : m_data(nullptr), m_width(w), m_height(h), m_selfAlloc(true)
    {
      lvdAssert(w > 0 && h > 0);
      m_data = LVDALLOC(T, w * h);
    }

    constexpr ImageChannel(const ImageChannel &other) : ImageChannel(other.m_width, other.m_height)
    { *this = other; }

    constexpr ImageChannel(ImageChannel &&other) : ImageChannel()
    { *this = other; }

    inline ~ImageChannel()
    {
      if(m_selfAlloc && m_data)
        lvdFree(m_data);
      m_data = nullptr;
    }

    constexpr ImageChannel &operator=(const ImageChannel &other)
    {
      lvdAssert(other.m_width == m_width && other.m_height == m_height, "Shape doesn't match.");
      std::copy(other.m_data, other.m_data + m_width * m_height, m_data);
      return *this;
    }

    constexpr ImageChannel &operator=(ImageChannel &&other)
    {
      if(m_selfAlloc && m_data)
        lvdFree(m_data);
      m_data = other.m_data;
      m_width = other.m_width;
      m_height = other.m_height;
      m_selfAlloc = other.m_selfAlloc;
      other.m_data = nullptr;
      return *this;
    }

    constexpr T &operator()(uint32_t y, uint32_t x)
    {
      lvdDebugAssert(y < m_height && x < m_width, "Index out of range.");
      return m_data[y * m_width + x];
    }

    constexpr const T &operator()(uint32_t y, uint32_t x) const
    {
      lvdDebugAssert(y < m_height && x < m_width, "Index out of range.");
      return m_data[y * m_width + x];
    }

    constexpr uint32_t width() const
    { return m_width; }

    constexpr uint32_t height() const
    { return m_height; }

    constexpr uint32_t size() const
    { return m_width * m_height; }

    constexpr T *data()
    { return m_data; }

    constexpr const T *data() const
    { return m_data; }

    constexpr T *begin()
    { return m_data; }
    constexpr T *end()
    { return m_data + size(); }

    constexpr const T *begin() const
    { return m_data; }
    constexpr const T *end() const
    { return m_data + size(); }

  private:
    T *m_data;
    uint32_t m_width, m_height;
    bool m_selfAlloc;
  };
} // namespace LightVideoDecoder