#include "util_p.hpp"
#include <limits>

namespace LightVideoDecoder
{
  template<typename T>static inline void yuvp2rgbi(const T *LV_RESTRICT y, const T *LV_RESTRICT u, const T *LV_RESTRICT v, T *LV_RESTRICT rgb, int stride, int width, int height)
  {
    constexpr double vmin = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double vhalf = static_cast<double>(std::numeric_limits<T>::max()) / 2.0;
    for(int i = 0; i < width * height; ++i)
    {
      double yy = static_cast<double>(y[i]);
      double uu = static_cast<double>(u[i]) - vhalf;
      double vv = static_cast<double>(v[i]) - vhalf;
      rgb[i * stride] = static_cast<T>(clip(vmin, yy + 1.40199958865734 * vv, vmax));
      rgb[i * stride + 1] = static_cast<T>(clip(vmin, yy - 0.344135678165337 * uu - 0.714136155581813 * vv, vmax));
      rgb[i * stride + 2] = static_cast<T>(clip(vmin, yy + 1.77200006607382 * uu, vmax));
    }
  }

  template<typename T>static inline void rgbi2yuvp(const T *LV_RESTRICT rgb, int stride, T *LV_RESTRICT y, T *LV_RESTRICT u, T *LV_RESTRICT v, int width, int height)
  {
    constexpr double vmin = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double vhalf = static_cast<double>((std::numeric_limits<T>::max() + 1) / 2);
    for(int i = 0; i < width * height; ++i)
    {
      double r = static_cast<double>(rgb[i * stride]);
      double g = static_cast<double>(rgb[i * stride + 1]);
      double b = static_cast<double>(rgb[i * stride + 2]);
      y[i] = static_cast<T>(clip(vmin, 0.299 * r + 0.587 * g + 0.114 * b + 0.5, vmax));
      u[i] = static_cast<T>(clip(vmin, -0.168736 * r - 0.331264 * g + 0.5 * b + vhalf, vmax));
      v[i] = static_cast<T>(clip(vmin, 0.5 * r - 0.418688 * g - 0.081312 * b + 0.5 + vhalf, vmax));
    }
  }

  template<typename T>static inline void yuvap2rgbai(const T *LV_RESTRICT y, const T *LV_RESTRICT u, const T *LV_RESTRICT v, const T *LV_RESTRICT a, T *LV_RESTRICT rgba, int stride, int width, int height)
  {
    constexpr double vmin = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double vhalf = static_cast<double>(std::numeric_limits<T>::max()) / 2.0;
    for(int i = 0; i < width * height; ++i)
    {
      double yy = static_cast<double>(y[i]);
      double uu = static_cast<double>(u[i]) - vhalf;
      double vv = static_cast<double>(v[i]) - vhalf;
      rgba[i * stride] = static_cast<T>(clip(vmin, yy + 1.40199958865734 * vv, vmax));
      rgba[i * stride + 1] = static_cast<T>(clip(vmin, yy - 0.344135678165337 * uu - 0.714136155581813 * vv, vmax));
      rgba[i * stride + 2] = static_cast<T>(clip(vmin, yy + 1.77200006607382 * uu, vmax));
      rgba[i * stride + 3] = static_cast<T>(a[i]);
    }
  }

  template<typename T>static inline void rgbai2yuvap(const T *LV_RESTRICT rgba, int stride, T *LV_RESTRICT y, T *LV_RESTRICT u, T *LV_RESTRICT a, T *LV_RESTRICT v, int width, int height)
  {
    constexpr double vmin = static_cast<double>(std::numeric_limits<T>::min());
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double vhalf = static_cast<double>((std::numeric_limits<T>::max() + 1) / 2);
    for(int i = 0; i < width * height; ++i)
    {
      double r = static_cast<double>(rgba[i * stride]);
      double g = static_cast<double>(rgba[i * stride + 1]);
      double b = static_cast<double>(rgba[i * stride + 2]);
      y[i] = static_cast<T>(clip(vmin, 0.299 * r + 0.587 * g + 0.114 * b + 0.5, vmax));
      u[i] = static_cast<T>(clip(vmin, -0.168736 * r - 0.331264 * g + 0.5 * b + vhalf, vmax));
      v[i] = static_cast<T>(clip(vmin, 0.5 * r - 0.418688 * g - 0.081312 * b + 0.5 + vhalf, vmax));
      a[i] = rgba[i * stride + 3];
    }
  }
} // namespace LightVideo