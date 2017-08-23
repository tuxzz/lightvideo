#pragma once

#include <functional>
#include <cstdint>
#include "struct.hpp"
#include "imagechannel.hpp"
#include "colorformat.hpp"

namespace LightVideoDecoder
{
  class DecoderPrivate;

  class Decoder final
  {
  public:
    typedef std::function<void(char*, int64_t)> ReadFunc;
    typedef std::function<void(int64_t)> SeekFunc;
    typedef std::function<void()> PosFunc;

    Decoder(ReadFunc readFunc, SeekFunc seekFunc, PosFunc posFunc);
    ~Decoder();

    /* property getter */
    uint32_t width() const;
    uint32_t height() const;
    uint32_t framerate() const;
    uint32_t frameCount() const;
    ColorFormat colorFormat() const;
    uint8_t formatVersion() const;

    double duration() const;
    uint32_t channelCount() const;

    /* method */
    void seekFrame(uint32_t pos);
    void nextFrame();
    void decodeCurrentFrame();

    /* status getter */
    uint32_t currentFrameNumber() const;
    bool isCurrentFrameDecoded() const;

    uint32_t getCurrentFrameFS() const;
    uint32_t getCurrentFrameHS() const;

  private:
    /* func */
    void loadPacket();
    void loadFrame();
    ReadFunc m_read;
    SeekFunc m_seek;
    PosFunc m_pos;

    /* info */
    MainStruct m_mainStruct;
    ColorFormatInfo m_colorFormatInfo;
    
    /* status */
    VideoFramePacket m_currentPacket;
    VideoFrameStruct m_currentFrameStruct;
    uint32_t m_currentFrameNumber, m_prevFullFrameNumber;
    bool m_packetLoaded, m_frameLoaded, m_currentFrameDecoded;

    /* buffer */
    char *m_compressedDataBuffer, *m_uncompressedDataBuffer, *m_frameDataBuffer;
    uint32_t m_uncompressedDataBufferPos;

    /* private */
    DecoderPrivate *m_dptr;
  };
} // namespace LightVideoDecoder