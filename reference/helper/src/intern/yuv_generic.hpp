#include "privateutil.hpp"
#include <limits>

namespace LightVideo
{
  template<typename T>static inline void yuvp2rgbi(const T *LV_RESTRICT y, const T *LV_RESTRICT u, const T *LV_RESTRICT v, T *LV_RESTRICT rgb, int stride, int width, int height)
  {
    constexpr int vmax = std::numeric_limits<T>::max();
    constexpr int ihvmax = vmax / 2;
    for(int i = 0; i < width * height; ++i)
    {
      int cy = y[i];
      int co = u[i] - ihvmax;
      int cg = v[i] - ihvmax;
      int r = cy - cg + co;
      int g = cy + cg;
      int b = cy - cg - co;
      rgb[i * stride] = clip(0, r, vmax);
      rgb[i * stride + 1] = clip(0, g, vmax);
      rgb[i * stride + 2] = clip(0, b, vmax);
    }
  }

  template<typename T>static inline void rgbi2yuvp(const T *LV_RESTRICT rgb, int stride, T *LV_RESTRICT y, T *LV_RESTRICT u, T *LV_RESTRICT v, int width, int height)
  {
    constexpr double vmax = static_cast<double>(std::numeric_limits<T>::max());
    constexpr double ihvmax = static_cast<double>(static_cast<int>(vmax / 2.0));
    for(int i = 0; i < width * height; ++i)
    {
      double r = static_cast<double>(rgb[i * stride]);
      double g = static_cast<double>(rgb[i * stride + 1]);
      double b = static_cast<double>(rgb[i * stride + 2]);
      double cy = r * 0.25 + g * 0.5 + b * 0.25;
      double cg = (-r * 0.25 + g * 0.5 - b * 0.25) + ihvmax;
      double co = (r * 0.5 - b * 0.5) + ihvmax;
      y[i] = static_cast<T>(clip(0.0, cy, vmax));
      u[i] = static_cast<T>(clip(0.0, co, vmax));
      v[i] = static_cast<T>(clip(0.0, cg, vmax));
    }
  }
} // namespace LightVideo