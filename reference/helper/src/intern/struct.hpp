#pragma once

#include <cstdint>

namespace LightVideo
{
  enum CompressionMethod : uint8_t
  {
    NoCompression = 0x0,
    LZ4Compression
  };

  enum IntraPredictMode : uint8_t
  {
    NoIntraPredict = 0x0,
    SubTop,
    SubLeft,
    SubAvg,
    SubPaeth
  };

  enum ReferenceType : uint8_t
  {
    NoReference = 0x0,
    PreviousFullReference,
    PreviousReference
  };

  enum ColorType : uint8_t
  {
    YUV420P = 0x0,
    YUVA420P
  };

  struct MainStruct
  {
    char aria[4];
    uint8_t version;
    ColorType colorFormat;
    uint8_t framerate;
    uint8_t maxPacketSize;
    uint16_t maxScaleRadius;
    uint16_t maxMovingRadius;
    char _reserved_1[4];
    uint32_t width, height;
    uint32_t nFrame;
    char _reserved_2[4];
  };

  struct VideoFramePacket
  {
    char vfpk[4];
    uint8_t nFrame, nFullFrame;
    CompressionMethod compressionMethod;
    char _reserved_0;
    uint32_t size;
    uint32_t checksum;
  };

  struct VideoFrameStruct
  {
    char vfrm[4];
    ReferenceType referenceType;
    char _reserved_0[3];
    int16_t scalePredictValue;
    int16_t movePredictValueX, movePredictValueY;
    IntraPredictMode intraPredictModeList[8];
    char _reserved_1[10];
  };
}