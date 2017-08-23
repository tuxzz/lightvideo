#include "../decoder.hpp"
#include "../error.hpp"
#include "decoderimpl_p.hpp"
#include "util_p.hpp"

extern "C"
{
  extern int LZ4_decompress_safe(const char* source, char* dest, int compressedSize, int maxDecompressedSize);
  extern int LZ4_compressBound(int inputSize);
}

namespace LightVideoDecoder
{
  Decoder::Decoder(ReadFunc readFunc, SeekFunc seekFunc, PosFunc posFunc)
    : m_read(readFunc), m_seek(seekFunc), m_pos(posFunc),
    m_mainStruct({0}), m_colorFormatInfo({0}), m_currentPacket({0}), m_currentFrameStruct({0}),
    m_currentFrameNumber(0), m_prevFullFrameNumber(0), m_packetLoaded(false), m_frameLoaded(false), m_currentFrameDecoded(false),
    m_compressedDataBuffer(nullptr), m_uncompressedDataBuffer(nullptr), m_frameDataBuffer(nullptr),
    m_uncompressedDataBufferPos(0),
    m_dptr(nullptr)
  {
    lvdAssert(readFunc && seekFunc && posFunc);
    try
    {
      m_seek(0);
      m_read(reinterpret_cast<char*>(&m_mainStruct), sizeof(MainStruct));
      if(!verifyMainStruct(m_mainStruct))
        throw DataError("Video main structure is broken.");
      m_colorFormatInfo = getColorFormatInfo(m_mainStruct.colorFormat, m_mainStruct.width, m_mainStruct.height);

      int maxUncompressedPacketDataSize = (sizeof(VideoFrameStruct) + m_colorFormatInfo.dataSize) * m_mainStruct.maxPacketSize;
      if(maxUncompressedPacketDataSize > 0x7E000000 || maxUncompressedPacketDataSize <= 0)
        throw DataError("Uncompressed packet size is too large.");
      int maxCompressedPacketDataSize = std::max(maxUncompressedPacketDataSize, LZ4_compressBound(maxUncompressedPacketDataSize));

      m_compressedDataBuffer = LVDALLOC(char, maxCompressedPacketDataSize);
      m_uncompressedDataBuffer = LVDALLOC(char, maxUncompressedPacketDataSize);
      m_dptr = new DecoderImpl<uint8_t>(m_mainStruct);
    }
    catch(const std::exception &)
    {
      critical("Cannot initialize a decoder.");

      if(m_dptr)
        delete static_cast<DecoderImpl<uint8_t>*>(m_dptr);
      if(m_uncompressedDataBuffer)
        lvdFree(m_uncompressedDataBuffer);
      if(m_compressedDataBuffer)
        lvdFree(m_compressedDataBuffer);
      m_dptr = nullptr;
      m_uncompressedDataBuffer = nullptr;
      m_compressedDataBuffer = nullptr;

      m_seek(0);
      throw;
    }
  }

  Decoder::~Decoder()
  {
    if(m_dptr)
      delete static_cast<DecoderImpl<uint8_t>*>(m_dptr);
    if(m_uncompressedDataBuffer)
      lvdFree(m_uncompressedDataBuffer);
    if(m_compressedDataBuffer)
      lvdFree(m_compressedDataBuffer);
    m_dptr = nullptr;
    m_uncompressedDataBuffer = nullptr;
    m_compressedDataBuffer = nullptr;
  }

  /* property getter */
  uint32_t Decoder::width() const
  { return m_mainStruct.width; }
  uint32_t Decoder::height() const
  { return m_mainStruct.height; }
  uint32_t Decoder::framerate() const
  { return m_mainStruct.framerate; }
  uint32_t Decoder::frameCount() const
  { return m_mainStruct.nFrame; }
  ColorFormat Decoder::colorFormat() const
  { return m_mainStruct.colorFormat; }
  uint8_t Decoder::formatVersion() const
  { return m_mainStruct.version; }
  double Decoder::duration() const
  { return static_cast<double>(m_mainStruct.nFrame) / static_cast<double>(m_mainStruct.framerate); }
  uint32_t Decoder::channelCount() const
  { return static_cast<uint32_t>(m_colorFormatInfo.channelList.size()); }

  /* method */
  void Decoder::seekFrame(uint32_t pos)
  {
    if(pos >= m_mainStruct.nFrame)
      throw EOFError("EOF");
    if(m_frameLoaded && pos == m_currentFrameNumber)
      return;
    m_frameLoaded = false;
    m_currentFrameDecoded = false;
    if(pos == m_currentFrameNumber + 1)
    {
      bool prevIsFull = m_currentFrameStruct.referenceType == NoReference;
      if(m_packetLoaded)
      {
        uint32_t currentUncompressedPacketSize = (sizeof(VideoFrameStruct) + m_colorFormatInfo.dataSize) * m_currentPacket.nFrame;
        if(m_uncompressedDataBufferPos >= currentUncompressedPacketSize)
          m_packetLoaded = false;
      }
      if(!m_packetLoaded)
        loadPacket();
      loadFrame();

      m_currentFrameNumber = pos;
      if(prevIsFull)
        m_prevFullFrameNumber = pos - 1;
    }
    else if(pos == 0)
    {
      m_packetLoaded = false;
      m_seek(sizeof(MainStruct));

      loadPacket();
      loadFrame();

      if(m_currentFrameStruct.referenceType != NoReference)
        throw DataError("Frame 0 must be full frame.");

      m_currentFrameNumber = 0;
      m_prevFullFrameNumber = 0;
    }
    else
      lvdAssert(false, "You can only seek to start or next frame.");
  }

  void Decoder::nextFrame()
  { seekFrame(m_currentFrameNumber + 1); }

  void Decoder::decodeCurrentFrame()
  {
    if(!m_currentFrameDecoded)
    {
      DecoderImpl<uint8_t> &decoderImpl = static_cast<DecoderImpl<uint8_t>&>(*m_dptr);
      decoderImpl.decodeCurrentFrameData(m_currentFrameStruct, m_frameDataBuffer);
      m_currentFrameDecoded = true;
    }
  }

  /* status getter */
  uint32_t Decoder::currentFrameNumber() const
  { return m_currentFrameNumber; }

  bool Decoder::isCurrentFrameDecoded() const
  { return m_currentFrameDecoded; }
  
  void Decoder::loadPacket()
  {
    if(!m_packetLoaded)
    {
      m_read(reinterpret_cast<char*>(&m_currentPacket), sizeof(VideoFramePacket));
      if(!verifyVFPK(m_mainStruct, m_currentPacket))
        throw DataError("Video packet is invalid.");
      if(m_currentPacket.compressionMethod == NoCompression)
        m_read(m_uncompressedDataBuffer, m_currentPacket.size);
      else // LZ4Compression
      {
        uint32_t uncompressedPacketSize = (sizeof(VideoFrameStruct) + m_colorFormatInfo.dataSize) * m_currentPacket.nFrame;
        m_read(m_compressedDataBuffer, m_currentPacket.size);
        if(LZ4_decompress_safe(m_compressedDataBuffer, m_uncompressedDataBuffer, m_currentPacket.size, uncompressedPacketSize) == -1)
          throw DataError("Invalid compressed data.");
      }
      m_uncompressedDataBufferPos = 0;
      m_packetLoaded = true;
    }
  }

  void Decoder::loadFrame()
  {
    lvdAssert(m_packetLoaded);
    if(!m_frameLoaded)
    {
      char *begin = m_uncompressedDataBuffer + m_uncompressedDataBufferPos;
      char *end = begin + sizeof(VideoFrameStruct);
      std::copy(begin, end, reinterpret_cast<char*>(&m_currentFrameStruct));
      if(!verifyVFRM(m_mainStruct, m_currentFrameStruct))
        throw DataError("Video frame is invalid");
      m_uncompressedDataBufferPos += sizeof(VideoFrameStruct) + m_colorFormatInfo.dataSize;
      m_frameDataBuffer = begin + sizeof(VideoFrameStruct);
      m_frameLoaded = true;
      m_currentFrameDecoded = false;
    }
  }

  uint32_t Decoder::getCurrentFrameFS() const
  { return static_cast<DecoderImpl<uint8_t>*>(m_dptr)->currentTextureFS(); }
  uint32_t Decoder::getCurrentFrameHS() const
  { return static_cast<DecoderImpl<uint8_t>*>(m_dptr)->currentTextureHS(); }
} // namespace LightVideoDecoder