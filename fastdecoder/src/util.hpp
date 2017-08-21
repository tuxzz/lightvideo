#pragma once

#include <cstdio>
#include <cstdlib>

#define _lv_S(a) _lv__S(a)
#define _lv__S(a) #a

#define lvdAssert(a, ...) \
        (void)((!(a)) ?  ( \
            ( \
            fprintf(stderr, \
                "lvdAssert failed: %s:%d, %s(), at \'%s\' " __VA_ARGS__ "\n", \
                __FILE__, __LINE__, __func__, _lv_S(a)), \
            std::abort(), \
            NULL)) : NULL)

#ifndef NDEBUG
#define lvdDebugAssert(a, ...) \
        (void)((!(a)) ?  ( \
            ( \
            fprintf(stderr, \
                "lvDebugAssert failed: %s:%d, %s(), at \'%s\' " __VA_ARGS__ "\n", \
                __FILE__, __LINE__, __func__, _lv_S(a)), \
            std::abort(), \
            NULL)) : NULL)
#else
#define lvdDebugAssert(a, ...)
#endif // NDEBUG

#define LVDALLOC(t, n) LightVideoDecoder::lvdAlloc< t >(n, __FILE__, __func__, __LINE__)

namespace LightVideoDecoder
{
  void *lvdAlloc(size_t size, const char *file, const char *func, int line);
  void lvdFree(void *ptr);
  void lvdCheckAllocated();

  template<typename T>static inline T *lvdAlloc(size_t n, const char *file, const char *func, int line)
  { return reinterpret_cast<T*>(lvdAlloc(n * sizeof(T), file, func, line)); }

  template<typename T>static inline void lvdFree(T *&ptr)
  {
    lvdFree(reinterpret_cast<void*>(ptr));
    ptr = nullptr;
  }
}