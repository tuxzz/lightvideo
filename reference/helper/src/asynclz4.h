#pragma once

#include "publicutil.h"
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

  struct LZ4CompressionTask;
  enum LZ4CompressionMode
  {
    lvFastCompression = 0x0,
    lvHighCompression
  };

  LIGHTVIDEO_EXPORT LZ4CompressionTask *lvCreateLZ4CompressionTask(const char *src, int srcSize, int mode, int level, bool calcAdler32);
  LIGHTVIDEO_EXPORT int lvWaitLZ4CompressionTask(LZ4CompressionTask *task, int msec);
  LIGHTVIDEO_EXPORT int lvGetLZ4CompressionTaskResultSize(LZ4CompressionTask *task);
  LIGHTVIDEO_EXPORT void lvGetLZ4CompressionTaskResultData(LZ4CompressionTask *task, char *dst, int dstCapacity);
  LIGHTVIDEO_EXPORT uint32_t lvGetLZ4CompressionTaskResultAdler32(LZ4CompressionTask *task);
  LIGHTVIDEO_EXPORT void lvDestroyLZ4CompressionTask(LZ4CompressionTask *task);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus