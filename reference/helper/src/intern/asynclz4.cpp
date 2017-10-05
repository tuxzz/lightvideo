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

static inline void doAder_16(const uint8_t *data, uint_fast32_t &a, uint_fast32_t &b)
{
  a += data[0]; b += a;
  a += data[1]; b += a;
  a += data[2]; b += a;
  a += data[3]; b += a;
  a += data[4]; b += a;
  a += data[5]; b += a;
  a += data[6]; b += a;
  a += data[7]; b += a;
  a += data[8]; b += a;
  a += data[9]; b += a;
  a += data[10]; b += a;
  a += data[11]; b += a;
  a += data[12]; b += a;
  a += data[13]; b += a;
  a += data[14]; b += a;
  a += data[15]; b += a;
}

static uint32_t doCalcAdler32(const uint8_t *data, int dataSize)
{
  /* 
  This function is from zlib project.

  interface of the 'zlib' general purpose compression library
  version 1.2.11, January 15th, 2017
  Copyright (C) 1995-2017 Jean-loup Gailly and Mark Adler
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software
  in a product, an acknowledgment in the product documentation would be
  appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu
  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
  (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).
  */

  uint_fast32_t a = 1;
  uint_fast32_t b = 0;

  /* do length NMAX blocks -- requires just one modulo operation */
  while (dataSize >= 5552)
  {
    dataSize -= 5552;
    uint_fast32_t n = 5552 / 16;          /* NMAX is divisible by 16 */
    do
    {
      doAder_16(data, a, b);          /* 16 sums unrolled */
      data += 16;
    } while (--n);
    a %= 65521U;
    b %= 65521U;
  }

  /* do remaining bytes (less than NMAX, still just one modulo) */
  if(dataSize) /* avoid modulos if none remaining */
  {
    while(dataSize >= 16)
    {
      dataSize -= 16;
      doAder_16(data, a, b);
      data += 16;
    }
    while(dataSize--) {
      a += *data++;
      b += a;
    }
    a %= 65521U;
    b %= 65521U;
  }

  /* return recombined sums */
  return a + b * 65536;
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
    checksum = doCalcAdler32(reinterpret_cast<uint8_t*>(dest), compressedSize);

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
    status = task->future.wait_for(std::chrono::milliseconds(msec));

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

int lvDecompressLZ4(const uint8_t *data, int dataSize, uint8_t *out, int maxOutSize)
{ return LZ4_decompress_safe(reinterpret_cast<const char*>(data), reinterpret_cast<char*>(out), dataSize, maxOutSize); }