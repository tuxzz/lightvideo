#include "../yuv.h"
#include "privateutil.hpp"
#include <limits>
#include <tuple>

constexpr static int64_t calcMax = 4294967296; // 32bit

namespace LightVideo
{
  template<typename T>static inline std::tuple<T, T, T> yuv2rgb_single(T _y, T _u, T _v)
  {
    constexpr int64_t max = std::numeric_limits<T>::max();
    constexpr int64_t half = (max + 1LL) / 2LL;
    int64_t y = static_cast<int64_t>(_y);
    int64_t u = static_cast<int64_t>(_u) - half;
    int64_t v = static_cast<int64_t>(_v) - half;

    int64_t r = (4294967296LL * y - 5235LL * u + 6021542382LL * v) / calcMax;
    int64_t g = (4294967296LL * y - 1478051483LL * u - 3067191433LL * v) / calcMax;
    int64_t b = (4294967296LL * y + 7610682332LL * u + 1745LL * v) / calcMax;
    return std::make_tuple(static_cast<T>(clip(0LL, r, max)), static_cast<T>(clip(0LL, g, max)), static_cast<T>(clip(0LL, b, max)));
  }

  template<typename T>static inline std::tuple<T, T, T> rgb2yuv_single(T _r, T _g, T _b)
  {
    constexpr int64_t max = std::numeric_limits<T>::max();
    constexpr int64_t half = (max + 1LL) / 2LL;
    int64_t r = static_cast<int64_t>(_r);
    int64_t g = static_cast<int64_t>(_g);
    int64_t b = static_cast<int64_t>(_b);

    int64_t y = (1284195221LL * r + 2521145803LL * g + 489626272LL * b) / calcMax;
    int64_t u = (-724715602LL * r - 1422768046LL * g + 2147483648LL * b) / calcMax + half;
    int64_t v = (2147483648LL * r - 1798251267lL * g - 349232381LL * b) / calcMax + half;
    return std::make_tuple(static_cast<T>(clip(0LL, y, max)), static_cast<T>(clip(0LL, u, max)), static_cast<T>(clip(0LL, v, max)));
  }

  template<typename T>static inline void yuvp2rgbi_impl(const T *LV_RESTRICT y, const T *LV_RESTRICT u, const T *LV_RESTRICT v, T *LV_RESTRICT rgb, int stride, int width, int height)
  {
    for(int i = 0; i < width * height; ++i)
      std::tie(rgb[i * stride], rgb[i * stride + 1], rgb[i * stride + 2]) = yuv2rgb_single(y[i], u[i], v[i]);
  }

  template<typename T>static inline void rgbi2yuvp_impl(const T *LV_RESTRICT rgb, int stride, T *LV_RESTRICT y, T *LV_RESTRICT u, T *LV_RESTRICT v, int width, int height)
  {
    for(int i = 0; i < width * height; ++i)
      std::tie(y[i], u[i], v[i]) = rgb2yuv_single(rgb[i * stride], rgb[i * stride + 1], rgb[i * stride + 2]);
  }

  template<typename T>static inline void yuvap2rgbai_impl(const T *LV_RESTRICT y, const T *LV_RESTRICT u, const T *LV_RESTRICT v, const T *LV_RESTRICT a, T *LV_RESTRICT rgba, int stride, int width, int height)
  {
    for(int i = 0; i < width * height; ++i)
    {
      std::tie(rgba[i * stride], rgba[i * stride + 1], rgba[i * stride + 2]) = yuv2rgb_single(y[i], u[i], v[i]);
      rgba[i * stride + 3] = a[i];
    }
  }

  template<typename T>static inline void rgbai2yuvap_impl(const T *LV_RESTRICT rgba, int stride, T *LV_RESTRICT y, T *LV_RESTRICT u, T *LV_RESTRICT a, T *LV_RESTRICT v, int width, int height)
  {
    for(int i = 0; i < width * height; ++i)
    {
      std::tie(y[i], u[i], v[i]) = rgb2yuv_single(rgba[i * stride], rgba[i * stride + 1], rgba[i * stride + 2]);
      a[i] = rgba[i * stride + 3];
    }
  }

  void yuvp2rgbi8_vhfp(const uint8_t *LV_RESTRICT y, const uint8_t *LV_RESTRICT u, const uint8_t *LV_RESTRICT v, uint8_t *LV_RESTRICT rgb, int stride, int width, int height)
  { yuvp2rgbi_impl<uint8_t>(y, u, v, rgb, stride, width, height); }
  void rgbi2yuvp8_vhfp(const uint8_t *LV_RESTRICT rgb, int stride, uint8_t *LV_RESTRICT y, uint8_t *LV_RESTRICT u, uint8_t *LV_RESTRICT v, int width, int height)
  { rgbi2yuvp_impl<uint8_t>(rgb, stride, y, u, v, width, height); }
  void yuvap2rbgai8_vhfp(const uint8_t *LV_RESTRICT y, const uint8_t *LV_RESTRICT u, const uint8_t *LV_RESTRICT v, const uint8_t *LV_RESTRICT a, uint8_t *LV_RESTRICT rgba, int stride, int width, int height)
  { yuvap2rgbai_impl<uint8_t>(y, u, v, a, rgba, stride, width, height); }
  void rgbai2yuvap8_vhfp(const uint8_t *LV_RESTRICT rgba, int stride, uint8_t *LV_RESTRICT y, uint8_t *LV_RESTRICT u, uint8_t *LV_RESTRICT v, uint8_t *LV_RESTRICT a, int width, int height)
  { rgbai2yuvap_impl<uint8_t>(rgba, stride, y, u, v, a, width, height); }

  void yuvp2rgbi16_vhfp(const uint16_t *LV_RESTRICT y, const uint16_t *LV_RESTRICT u, const uint16_t *LV_RESTRICT v, uint16_t *LV_RESTRICT rgb, int stride, int width, int height)
  { yuvp2rgbi_impl<uint16_t>(y, u, v, rgb, stride, width, height); }
  void rgbi2yuvp16_vhfp(const uint16_t *LV_RESTRICT rgb, int stride, uint16_t *LV_RESTRICT y, uint16_t *LV_RESTRICT u, uint16_t *LV_RESTRICT v, int width, int height)
  { rgbi2yuvp_impl<uint16_t>(rgb, stride, y, u, v, width, height); }
  void yuvap2rbgai16_vhfp(const uint16_t *LV_RESTRICT y, const uint16_t *LV_RESTRICT u, const uint16_t *LV_RESTRICT v, const uint16_t *LV_RESTRICT a, uint16_t *LV_RESTRICT rgba, int stride, int width, int height)
  { yuvap2rgbai_impl<uint16_t>(y, u, v, a, rgba, stride, width, height); }
  void rgbai2yuvap16_vhfp(const uint16_t *LV_RESTRICT rgba, int stride, uint16_t *LV_RESTRICT y, uint16_t *LV_RESTRICT u, uint16_t *LV_RESTRICT v, uint16_t *LV_RESTRICT a, int width, int height)
  { rgbai2yuvap_impl<uint16_t>(rgba, stride, y, u, v, a, width, height); }
} // namespace LightVideo
