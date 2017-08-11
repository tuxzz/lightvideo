#pragma once

#include "publicutil.h"
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

  struct LVDecoder;

  struct LVDecoderIOContext
  {
    int64_t(*read)(char *dst, int64_t maxSize);
    bool(*seek)(int64_t pos);
    int64_t(*pos)();
  };

  struct LVVideoInformation
  {
    uint32_t width, height, framerate;
    uint32_t nFrame;
    uint8_t colorFormat;
    uint8_t formatVersion;
  };

  enum FrameGettingResult
  {
    ResFailed = 0x0,
    ResEOF,
    ResSuccess
  };

  LIGHTVIDEO_EXPORT LVDecoder *lvCreateDecoder(const LVDecoderIOContext *ioContext);
  LIGHTVIDEO_EXPORT LVVideoInformation lvGetDecoderInformation(LVDecoder *decoder);
  LIGHTVIDEO_EXPORT uint32_t lvDecoderFramePos(LVDecoder *decoder);
  LIGHTVIDEO_EXPORT int lvDecoderSeekFrame(LVDecoder *decoder, uint32_t pos);
  LIGHTVIDEO_EXPORT int lvGetFrame8(LVDecoder *decoder, uint8_t **channelList, uint32_t channelCount);
  LIGHTVIDEO_EXPORT void lvDestroyDecoder(LVDecoder *decoder);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus