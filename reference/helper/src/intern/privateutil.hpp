#pragma once

#include "../publicutil.h"
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <future>

#define _S(a) __S(a)
#define __S(a) #a

#define lvAssert(a, ...) \
        (void)((!(a)) ?  ( \
            ( \
            fprintf(stderr, \
                "lvAssert failed: %s:%d, %s(), at \'%s\' " __VA_ARGS__ "\n", \
                __FILE__, __LINE__, __func__, _S(a)), \
            std::abort(), \
            NULL)) : NULL)

#ifndef NDEBUG
#define lvDebugAssert(a, ...) \
        (void)((!(a)) ?  ( \
            ( \
            fprintf(stderr, \
                "lvDebugAssert failed: %s:%d, %s(), at \'%s\' " __VA_ARGS__ "\n", \
                __FILE__, __LINE__, __func__, _S(a)), \
            std::abort(), \
            NULL)) : NULL)
#else
#define lvDebugAssert(a, ...)
#endif // NDEBUG

#define GUARDED_ALLOC(t, n) LightVideo::lvAlloc< t >(n, __FILE__, __func__, __LINE__);

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
#define LV_ALIGNED(x) __declspec(align(x))
#elif defined(__GNUC__)
#define LV_ALIGNED(x) __attribute__ ((aligned(x)))
#endif

namespace LightVideo
{
  template<typename T>static inline T *lvAlloc(size_t n, const char *file, const char *func, int line)
  { return reinterpret_cast<T*>(::lvAlloc(n * sizeof(T), file, func, line)); }

  template<typename T>static inline void lvFree(T *&ptr)
  {
    ::lvFree(reinterpret_cast<void*>(ptr));
    ptr = nullptr;
  }

  template<typename T>static inline T clip(T min, T v, T max)
  { return std::min(std::max(min, v), max); }

  template<typename T>static inline T absDrop(T x, T dropOrigin, T dropDst)
  {
    if(std::abs(x) - dropOrigin <= dropDst)
      return dropOrigin;
    return x;
  }

  template<typename T>static inline T absDrop(T x, T dropDst)
  {
    if(std::abs(x) <= dropDst)
      return 0;
    return x;
  }

  template<typename T>class WorkMem
  {
  public:
    inline WorkMem(T *workMem, int size) : m_workMem(workMem), m_selfAlloc(workMem == nullptr)
    {
      if(!workMem)
        m_workMem = GUARDED_ALLOC(T, size);
    }

    inline WorkMem(int size) : m_selfAlloc(true)
    { m_workMem = GUARDED_ALLOC(T, size); }

    inline ~WorkMem()
    {
      if(m_selfAlloc)
        lvFree(m_workMem);
    }

    inline T *get() const
    { return m_workMem; }

  private:
    T *m_workMem;
    bool m_selfAlloc;
  };

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

  TaskStatus futureStatusToLvTaskStatus(std::future_status status);

  void vPrintMessage(MessageCategory category, const char *msg, va_list args);
}