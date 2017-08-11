#include "..\asynclz4.h"
#include "privateutil.hpp"
#include <future>
#include <chrono>
#include <algorithm>
#include "lz4.h"
#include "lz4hc.h"

using namespace LightVideo;

struct LZ4CompressionResult
{
  char *compressedData;
  int compressedSize;
  uint32_t adler32;
};

struct LZ4CompressionTask
{
  LZ4CompressionResult resultCache;
  std::future<LZ4CompressionResult> future;
  bool cacheValid, calcAdler32;
};

static uint32_t doCalcAdler32(const char *LV_RESTRICT data, int dataSize)
{
  uint32_t a = 1;
  uint32_t b = 0;
  for(int i = 0; i < dataSize; ++i)
  {
    a = (a + data[i]) % 65521;
    b = (b + a) % 65521;
  }
  return b * 65536 + a;
}

static LZ4CompressionResult compressCore(char *src, int srcSize, int mode, int level, bool calcAdler32)
{
  int maxDestSize = LZ4_compressBound(srcSize);
  char *dest = GUARDED_ALLOC(char, maxDestSize);
  int compressedSize;
  if(mode == lvFastCompression)
    compressedSize = LZ4_compress_fast(src, dest, srcSize, maxDestSize, level);
  else
    compressedSize = LZ4_compress_HC(src, dest, srcSize, maxDestSize, level);
  lvFree(src);

  uint32_t checksum = 0;
  if(calcAdler32)
    checksum = doCalcAdler32(dest, compressedSize);

  return {
    dest,
    compressedSize,
    checksum
  };
}

static void ensureCache(LZ4CompressionTask *task)
{
  if(!task->cacheValid)
  {
    task->resultCache = task->future.get();
    task->cacheValid = true;
  }
}

LZ4CompressionTask *lvCreateLZ4CompressionTask(const char *src, int srcSize, int mode, int level, bool calcAdler32)
{
  if(!src)
  {
    fatal("Invalid src.");
    return nullptr;
  }

  if(srcSize <= 0)
  {
    fatal("srcSize must be greater than 0");
    return nullptr;
  }

  if(mode != lvFastCompression && mode != lvHighCompression)
  {
    fatal("Invalid mode.");
    return nullptr;
  }

  if(level < 0)
  {
    fatal("Invalid level.");
    return nullptr;
  }

  LZ4CompressionTask *task = new LZ4CompressionTask;
  char *copiedSrc = GUARDED_ALLOC(char, srcSize);
  std::copy(src, src + srcSize, copiedSrc);
  task->future = std::async(std::launch::async, compressCore, copiedSrc, srcSize, mode, level, calcAdler32);
  task->cacheValid = false;
  task->calcAdler32 = calcAdler32;
  return task;
}

int lvWaitLZ4CompressionTask(LZ4CompressionTask *task, int msec)
{
  if(!task)
  {
    fatal("Invalid task.");
    return lvNotRunned;
  }

  std::future_status status = std::future_status::ready;
  if(msec < 0)
    task->future.wait();
  else
    status = task->future.wait_for(std::chrono::milliseconds::duration(msec));

  return futureStatusToLvTaskStatus(status);
}

int lvGetLZ4CompressionTaskResultSize(LZ4CompressionTask *task)
{
  if(!task)
  {
    fatal("Invalid task.");
    return -1;
  }
  
  ensureCache(task);
  return task->resultCache.compressedSize;
}

void lvGetLZ4CompressionTaskResultData(LZ4CompressionTask *task, char *dst, int dstCapacity)
{
  if(!task)
  {
    fatal("Invalid task.");
    return;
  }

  ensureCache(task);
  std::copy(task->resultCache.compressedData, task->resultCache.compressedData + std::min(task->resultCache.compressedSize, dstCapacity), dst);
}

uint32_t lvGetLZ4CompressionTaskResultAdler32(LZ4CompressionTask *task)
{
  if(!task)
  {
    fatal("Invalid task.");
    return 0;
  }
  if(!task->calcAdler32)
  {
    fatal("Adler32 is not calculated.");
    return 0;
  }

  ensureCache(task);
  return task->resultCache.adler32;
}

void lvDestroyLZ4CompressionTask(LZ4CompressionTask *task)
{
  ensureCache(task);
  lvFree(task->resultCache.compressedData);
  delete task;
}