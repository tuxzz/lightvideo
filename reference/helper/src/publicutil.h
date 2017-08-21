#pragma once

#include <cstddef>
#define LV_RESTRICT __restrict


#ifdef LIGHTVIDEOENCODERHELPER_EXPORTS
#define LIGHTVIDEO_EXPORT __declspec(dllexport)
#else
#define LIGHTVIDEO_EXPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

  enum TaskStatus
  {
    lvNotRunned = 0,
    lvRunning,
    lvFinished
  };

  LIGHTVIDEO_EXPORT void *lvAlloc(size_t size, const char *file, const char *func, int line);
  LIGHTVIDEO_EXPORT void lvFree(void *ptr);
  LIGHTVIDEO_EXPORT void lvCheckAllocated();

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus