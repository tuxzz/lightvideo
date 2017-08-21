#include "../helper/src/decoder.h"
#include "../helper/src/intern/imagechannel.hpp"
#include <cstdio>
#include <cerrno>
#include <chrono>

using namespace LightVideo;

static FILE *f = nullptr;

static int64_t read(char *dst, int64_t maxSize)
{ return fread(dst, 1, maxSize, f); }
static bool seek(int64_t pos)
{ return fseek(f, static_cast<long>(pos), SEEK_SET) == 0; }
static int64_t pos()
{ return ftell(f); }

int main()
{
  f = fopen("D:/codebase/lightvideo/reference/out.rcv", "rb");
  LVDecoderIOContext io = {
    read, seek, pos
  };
  auto dec = lvCreateDecoder(&io);
  LVVideoInformation info = lvGetDecoderInformation(dec);
  {
    ImageChannel<uint8_t> y(info.width, info.height);
    ImageChannel<uint8_t> u(std::max(1U, info.width / 2), std::max(1U, info.height / 2));
    ImageChannel<uint8_t> v(std::max(1U, info.width / 2), std::max(1U, info.height / 2));
    uint8_t *channelList[3] = {y.data(), u.data(), v.data()};
    auto start = std::chrono::steady_clock::now();
    for(uint32_t i = 0; i < info.nFrame; ++i)
      lvGetFrame8(dec, channelList, 3);
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    auto tDur = static_cast<double>(duration) / 1e3;
    auto tVid = static_cast<double>(info.nFrame) / static_cast<double>(info.framerate);
    printf("Decoded %lfs in %lfs, ratio = %lfx\n", 
      tVid, tDur, tVid / tDur);
  }
  lvDestroyDecoder(dec);
  fclose(f);

  lvCheckAllocated();
  system("pause");
  return 0;
}