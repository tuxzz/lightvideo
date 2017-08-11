#include "../yuv.h"
#include "privateutil.hpp"
#include <limits>

namespace LightVideo
{
  template<typename T>static inline void yuvp2rgbi_impl(const T *LV_RESTRICT y, const T *LV_RESTRICT u, const T *LV_RESTRICT v, T *LV_RESTRICT rgb, int stride, int width, int height)
  {
    constexpr double vmin = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double vhalf = static_cast<double>(std::numeric_limits<T>::max()) / 2.0f;
    for(int i = 0; i < width * height; ++i)
    {
      double yy = static_cast<double>(y[i]);
      double uu = static_cast<double>(u[i]) - vhalf;
      double vv = static_cast<double>(v[i]) - vhalf;
      rgb[i * stride] = static_cast<T>(clip(vmin, yy - 1.21889418896677e-6f * uu + 1.40199958865734f * vv, vmax));
      rgb[i * stride + 1] = static_cast<T>(clip(vmin, yy - 0.344135678165337f * uu - 0.714136155581813f * vv, vmax));
      rgb[i * stride + 2] = static_cast<T>(clip(vmin, yy + 1.77200006607382f * uu + 4.06298062659511e-7f * vv, vmax));
    }
  }

  template<typename T>static inline void rgbi2yuvp_impl(const T *LV_RESTRICT rgb, int stride, T *LV_RESTRICT y, T *LV_RESTRICT u, T *LV_RESTRICT v, int width, int height)
  {
    constexpr double vmin = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double vhalf = static_cast<double>((std::numeric_limits<T>::max() + 1) / 2);
    for(int i = 0; i < width * height; ++i)
    {
      double r = static_cast<double>(rgb[i * stride]);
      double g = static_cast<double>(rgb[i * stride + 1]);
      double b = static_cast<double>(rgb[i * stride + 2]);
      y[i] = static_cast<T>(clip(vmin, 0.299f * r + 0.587f * g + 0.114f * b + 0.5f, vmax));
      u[i] = static_cast<T>(clip(vmin, -0.168736f * r - 0.331264f * g + 0.5f * b + vhalf, vmax));
      v[i] = static_cast<T>(clip(vmin, 0.5f * r - 0.418688f * g - 0.081312f * b + 0.5f + vhalf, vmax));
    }
  }

  template<typename T>static inline void yuvap2rgbai_impl(const T *LV_RESTRICT y, const T *LV_RESTRICT u, const T *LV_RESTRICT v, const T *LV_RESTRICT a, T *LV_RESTRICT rgba, int stride, int width, int height)
  {
    constexpr double vmin = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double vhalf = static_cast<double>(std::numeric_limits<T>::max()) / 2.0f;
    for(int i = 0; i < width * height; ++i)
    {
      double yy = static_cast<double>(y[i]);
      double uu = static_cast<double>(u[i]) - vhalf;
      double vv = static_cast<double>(v[i]) - vhalf;
      rgba[i * stride] = static_cast<T>(clip(vmin, yy - 1.21889418896677e-6f * uu + 1.40199958865734f * vv, vmax));
      rgba[i * stride + 1] = static_cast<T>(clip(vmin, yy - 0.344135678165337f * uu - 0.714136155581813f * vv, vmax));
      rgba[i * stride + 2] = static_cast<T>(clip(vmin, yy + 1.77200006607382f * uu + 4.06298062659511e-7f * vv, vmax));
      rgba[i * stride + 3] = static_cast<T>(a[i]);
    }
  }

  template<typename T>static inline void rgbai2yuvap_impl(const T *LV_RESTRICT rgba, int stride, T *LV_RESTRICT y, T *LV_RESTRICT u, T *LV_RESTRICT a, T *LV_RESTRICT v, int width, int height)
  {
    constexpr double vmin = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double vhalf = static_cast<double>((std::numeric_limits<T>::max() + 1) / 2);
    for(int i = 0; i < width * height; ++i)
    {
      double r = static_cast<double>(rgba[i * stride]);
      double g = static_cast<double>(rgba[i * stride + 1]);
      double b = static_cast<double>(rgba[i * stride + 2]);
      y[i] = static_cast<T>(clip(vmin, 0.299f * r + 0.587f * g + 0.114f * b + 0.5f, vmax));
      u[i] = static_cast<T>(clip(vmin, -0.168736f * r - 0.331264f * g + 0.5f * b + vhalf, vmax));
      v[i] = static_cast<T>(clip(vmin, 0.5f * r - 0.418688f * g - 0.081312f * b + 0.5f + vhalf, vmax));
      a[i] = rgba[i * stride + 3];
    }
  }

  void yuvp2rgbi8_generic(const uint8_t *LV_RESTRICT y, const uint8_t *LV_RESTRICT u, const uint8_t *LV_RESTRICT v, uint8_t *LV_RESTRICT rgb, int stride, int width, int height)
  { yuvp2rgbi_impl<uint8_t>(y, u, v, rgb, stride, width, height); }
  void rgbi2yuvp8_generic(const uint8_t *LV_RESTRICT rgb, int stride, uint8_t *LV_RESTRICT y, uint8_t *LV_RESTRICT u, uint8_t *LV_RESTRICT v, int width, int height)
  { rgbi2yuvp_impl<uint8_t>(rgb, stride, y, u, v, width, height); }
  void yuvap2rbgai8_generic(const uint8_t *LV_RESTRICT y, const uint8_t *LV_RESTRICT u, const uint8_t *LV_RESTRICT v, const uint8_t *LV_RESTRICT a, uint8_t *LV_RESTRICT rgba, int stride, int width, int height)
  { yuvap2rgbai_impl<uint8_t>(y, u, v, a, rgba, stride, width, height); }
  void rgbai2yuvap8_generic(const uint8_t *LV_RESTRICT rgba, int stride, uint8_t *LV_RESTRICT y, uint8_t *LV_RESTRICT u, uint8_t *LV_RESTRICT v, uint8_t *LV_RESTRICT a, int width, int height)
  { rgbai2yuvap_impl<uint8_t>(rgba, stride, y, u, v, a, width, height); }

  void yuvp2rgbi16_generic(const uint16_t *LV_RESTRICT y, const uint16_t *LV_RESTRICT u, const uint16_t *LV_RESTRICT v, uint16_t *LV_RESTRICT rgb, int stride, int width, int height)
  { yuvp2rgbi_impl<uint16_t>(y, u, v, rgb, stride, width, height); }
  void rgbi2yuvp16_generic(const uint16_t *LV_RESTRICT rgb, int stride, uint16_t *LV_RESTRICT y, uint16_t *LV_RESTRICT u, uint16_t *LV_RESTRICT v, int width, int height)
  { rgbi2yuvp_impl<uint16_t>(rgb, stride, y, u, v, width, height); }
  void yuvap2rbgai16_generic(const uint16_t *LV_RESTRICT y, const uint16_t *LV_RESTRICT u, const uint16_t *LV_RESTRICT v, const uint16_t *LV_RESTRICT a, uint16_t *LV_RESTRICT rgba, int stride, int width, int height)
  { yuvap2rgbai_impl<uint16_t>(y, u, v, a, rgba, stride, width, height); }
  void rgbai2yuvap16_generic(const uint16_t *LV_RESTRICT rgba, int stride, uint16_t *LV_RESTRICT y, uint16_t *LV_RESTRICT u, uint16_t *LV_RESTRICT v, uint16_t *LV_RESTRICT a, int width, int height)
  { rgbai2yuvap_impl<uint16_t>(rgba, stride, y, u, v, a, width, height); }
} // namespace LightVideo