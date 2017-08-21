#pragma once
#include "../util.hpp"
#include <algorithm>

#if (defined(__AVX2__) || defined( __AVX__ ) || defined(_M_AMD64) || defined(_M_X64) || _M_IX86_FP == 2)
#ifndef __SSE2__
#define __SSE2__
#endif
#ifndef __SSE__
#define __SSE__
#endif
#endif

#if _M_IX86_FP == 1
#ifndef __SSE__
#define __SSE__
#endif
#endif

#if defined(_MSC_VER)
#define LVD_ALIGNED(x) __declspec(align(x))
#elif defined(__GNUC__)
#define LVD_ALIGNED(x) __attribute__ ((aligned(x)))
#endif

namespace LightVideoDecoder
{
  template<typename T>static inline T clip(T min, T v, T max)
  { return std::min(std::max(min, v), max); }

  template<typename T>static inline T absDrop(T x, T dropOrigin, T dropDst)
  {
    if(std::abs(x) - dropOrigin <= dropDst)
      return dropOrigin;
    return x;
  }

  enum MessageCategory
  {
    DebugMessage = 0,
    WarningMessage,
    CriticalMessage,
    FatalMessage
  };

  void debug(const char *msg, ...);
  void warning(const char *msg, ...);
  void critical(const char *msg, ...);
  void fatal(const char *msg, ...);

  void vPrintMessage(MessageCategory category, const char *msg, va_list args);
} // namespace LightVideoDecoder