#include "../decoder.h"
#include "struct.hpp"
#include "privateutil.hpp"
#include "imagechannel.hpp"
#include "decoderutil.hpp"
#include "colorformat.hpp"
#include "lz4.h"
#include <vector>
#include <algorithm>

using namespace LightVideo;

struct LVDecoder
{
  LVDecoder(const LVDecoderIOContext &ioCtx, const MainStruct &main)
    : io(ioCtx), mainStruct(main), framePos(0), decompressedPacketBufferPos(0), packetLoaded(false), frameDataSize(0)
  {}

  virtual ~LVDecoder()
  {}

  LVDecoderIOContext io;
  MainStruct mainStruct;
  ColorFormatInfo colorFormatInfo;

  VideoFramePacket currPacket;
  uint32_t framePos, decompressedPacketBufferPos;
  std::vector<char> decompressedPacketBuffer;
  std::vector<char> packetRawDataBuffer;
  bool packetLoaded;

  uint32_t frameDataSize;
};

template<typename T>struct DecoderImpl : public LVDecoder
{
  DecoderImpl(const LVDecoderIOContext &ioCtx, const MainStruct &main, ColorType colorFormat, uint32_t width, uint32_t height)
    : LVDecoder(ioCtx, main)
  {
    colorFormatInfo = getColorFormatInfo(mainStruct.colorFormat, width, height);

    size_t channelCount = colorFormatInfo.channelList.size();
    currChannelList.reserve(channelCount);
    prevChannelList.reserve(channelCount);
    prevFullChannelList.reserve(channelCount);
    for(size_t i = 0; i < channelCount; ++i)
      currChannelList.emplace_back(colorFormatInfo.channelList[i].width, colorFormatInfo.channelList[i].height);
    for(size_t i = 0; i < channelCount; ++i)
      prevFullChannelList.emplace_back(currChannelList[i].width(), currChannelList[i].height());
    for(size_t i = 0; i < channelCount; ++i)
      prevChannelList.emplace_back(currChannelList[i].width(), currChannelList[i].height());

    frameDataSize = 0;
    for(size_t i = 0; i < channelCount; ++i)
      frameDataSize += prevChannelList[i].size() * sizeof(T);

    uint32_t decompressPacketBufferSize = (sizeof(VideoFrameStruct) + frameDataSize) * main.maxPacketSize;
    decompressedPacketBuffer.resize(decompressPacketBufferSize);
    packetRawDataBuffer.resize(std::max(static_cast<uint32_t>(LZ4_compressBound(decompressPacketBufferSize)), decompressPacketBufferSize));
  }

  ~DecoderImpl()
  {}

  std::vector<ImageChannel<T>> prevChannelList;
  std::vector<ImageChannel<T>> prevFullChannelList;
  std::vector<ImageChannel<T>> currChannelList;
};

LVDecoder *lvCreateDecoder(const LVDecoderIOContext *ioContext)
{
  MainStruct mainStruct;
  ColorFormatInfo colorFormatInfo;

  int64_t origPos = ioContext->pos();
  if(!ioContext->seek(0))
  {
    critical("Failed to seek");
    goto onFailed;
  }
  // read and check data
  if(ioContext->read(reinterpret_cast<char*>(&mainStruct), sizeof(MainStruct)) != sizeof(MainStruct))
  {
    critical("Input is too short.");
    goto onFailed;
  }
  if(!verifyMainStruct(mainStruct))
    goto onFailed;

  colorFormatInfo = getColorFormatInfo(mainStruct.colorFormat, mainStruct.width, mainStruct.height);
  if(colorFormatInfo.typeSize == 1)
    return new DecoderImpl<uint8_t>(*ioContext, mainStruct, mainStruct.colorFormat, mainStruct.width, mainStruct.height);
  else
    lvAssert(false);

onFailed:
  if(!ioContext->seek(origPos))
    critical("Cannot recovery ioContext state.");
  return nullptr;
}

LVVideoInformation lvGetDecoderInformation(LVDecoder *decoder)
{
  return {
    decoder->mainStruct.width,
    decoder->mainStruct.height,
    decoder->mainStruct.framerate,
    decoder->mainStruct.nFrame,
    decoder->mainStruct.colorFormat,
    decoder->mainStruct.version
  };
}

uint32_t lvDecoderFramePos(LVDecoder *decoder)
{ return decoder->framePos; }

int lvDecoderSeekFrame(LVDecoder *decoder, uint32_t pos)
{
  // TODO
  return true; 
}

int lvGetFrame8(LVDecoder *_decoder, uint8_t **outChannelList, uint32_t channelCount)
{
  if(_decoder->colorFormatInfo.typeSize != 1)
  {
    fatal("Not 8-bit format.");
    return ResFailed;
  }

  if(_decoder->colorFormatInfo.channelList.size() != channelCount)
  {
    fatal("channelCount doesn't match.");
    return ResFailed;
  }

  if(_decoder->framePos >= _decoder->mainStruct.nFrame)
    return ResEOF;

  DecoderImpl<uint8_t> *decoder = static_cast<DecoderImpl<uint8_t>*>(_decoder);

  int64_t origPos = decoder->io.pos();
  goto skipFail;
onFailed:
  if(!_decoder->io.seek(origPos))
    critical("Failed to recovery decoder status.");
  return ResFailed;
  
skipFail:
  if(!decoder->packetLoaded)
  {
    // read header and check
    VideoFramePacket vfpk;
    if(decoder->io.read(reinterpret_cast<char*>(&vfpk), sizeof(VideoFramePacket)) != sizeof(VideoFramePacket))
    {
      critical("Video frame packet header is incomplete.");
      goto onFailed;
    }
    if(!verifyVFPK(decoder->mainStruct, vfpk))
      goto onFailed;

    // read data and decompress
    uint32_t exceptedDecompressedSize = (sizeof(VideoFrameStruct) + decoder->frameDataSize) * vfpk.nFrame;
    if(vfpk.compressionMethod == NoCompression)
    {
      if(vfpk.size != exceptedDecompressedSize)
      {
        critical("Video frame packet data is too short or broken.");
        goto onFailed;
      }
      if(decoder->io.read(decoder->decompressedPacketBuffer.data(), vfpk.size) != vfpk.size)
      {
        critical("Video frame packet data is too short or broken.");
        goto onFailed;
      }
    }
    else // vfpk.compressionMethod == LZ4Compression
    {
      if(decoder->io.read(decoder->packetRawDataBuffer.data(), vfpk.size) != vfpk.size)
      {
        critical("Video frame packet data is too short or broken.");
        goto onFailed;
      }
      if(LZ4_decompress_safe(decoder->packetRawDataBuffer.data(), decoder->decompressedPacketBuffer.data(), vfpk.size, exceptedDecompressedSize) != static_cast<int>(exceptedDecompressedSize))
      {
        critical("Video frame packet data is too short or broken.");
        goto onFailed;
      }
    }
    decoder->currPacket = vfpk;
    decoder->decompressedPacketBufferPos = 0;
    decoder->packetLoaded = true;
  }

  // load video frame and check
  VideoFrameStruct vfrm;
  char *begin = decoder->decompressedPacketBuffer.data() + decoder->decompressedPacketBufferPos;
  char *end = begin + sizeof(VideoFrameStruct);
  std::copy(begin, end, reinterpret_cast<char*>(&vfrm));
  if(!verifyVFRM(decoder->mainStruct, vfrm))
    goto onFailed;

  // load channels
  for(uint32_t i = 0; i < channelCount; ++i)
  {
    begin = end;
    end += decoder->currChannelList[i].size() * sizeof(uint8_t);
    std::copy(begin, end, reinterpret_cast<char*>(decoder->currChannelList[i].data()));
  }

  // defilter channels
  for(uint32_t i = 0; i < channelCount; ++i)
  {
    if(vfrm.referenceType == NoReference)
      defilterIntra<uint8_t>(decoder->currChannelList[i], vfrm.intraPredictModeList[i]);
    else if(vfrm.referenceType == PreviousFullReference)
      defilterDelta<uint8_t>(decoder->currChannelList[i], decoder->prevFullChannelList[i], vfrm.scalePredictValue, vfrm.movePredictValueX, vfrm.movePredictValueY, vfrm.intraPredictModeList[i]);
    else if(vfrm.referenceType == PreviousReference)
      defilterDelta<uint8_t>(decoder->currChannelList[i], decoder->prevChannelList[i], vfrm.scalePredictValue, vfrm.movePredictValueX, vfrm.movePredictValueY, vfrm.intraPredictModeList[i]);
  }

  // save state
  for(uint32_t i = 0; i < channelCount; ++i)
  {
    std::copy(decoder->currChannelList[i].begin(), decoder->currChannelList[i].end(), decoder->prevChannelList[i].begin());
    if(vfrm.referenceType == NoReference)
      std::copy(decoder->currChannelList[i].begin(), decoder->currChannelList[i].end(), decoder->prevFullChannelList[i].begin());
  }

  // output
  for(uint32_t i = 0; i < channelCount; ++i)
    std::copy(decoder->currChannelList[i].begin(), decoder->currChannelList[i].end(), outChannelList[i]);

  // move buffer pos
  decoder->decompressedPacketBufferPos = static_cast<uint32_t>(end - decoder->decompressedPacketBuffer.data());

  // check packet
  lvAssert(decoder->decompressedPacketBufferPos <= decoder->decompressedPacketBuffer.size());
  if(decoder->decompressedPacketBufferPos == decoder->decompressedPacketBuffer.size())
    decoder->packetLoaded = false;

  ++decoder->framePos;
  return ResSuccess;
}

void lvDestroyDecoder(LVDecoder *decoder)
{
  delete decoder;
}