#pragma once

#include "../colorformat.hpp"
#include "../imagechannel.hpp"
#include "decoder_p.hpp"
#include "util_p.hpp"
#include "defilter_dispatcher_p.hpp"
#include <xmmintrin.h>
#include <vector>

namespace LightVideoDecoder
{
  template<typename T>
  class DecoderImpl final : public DecoderPrivate
  {
  public:
    inline DecoderImpl(const MainStruct &mainStruct) : m_mainStruct(mainStruct), m_currIsFull(false)
    {
      m_colorFormatInfo = getColorFormatInfo(mainStruct.colorFormat, mainStruct.width, mainStruct.height);

      int nChannel = static_cast<int>(m_colorFormatInfo.channelList.size());
      for(int i = 0; i < nChannel; ++i)
      {
        Size s = m_colorFormatInfo.channelList[i];
        m_channelGroupList[0][i] = ImageChannel<T>(s.width, s.height);
        m_channelGroupList[1][i] = ImageChannel<T>(s.width, s.height);
        m_channelGroupList[2][i] = ImageChannel<T>(s.width, s.height);
      }
    }

    inline ~DecoderImpl()
    {}

    inline void decodeCurrentFrameData(const VideoFrameStruct &vfrm, const char *data)
    {
      int nChannel = static_cast<int>(m_colorFormatInfo.channelList.size());
      if(vfrm.referenceType == NoReference)
      {
        {
          const char *begin = data;
          for(int i = 0; i < nChannel; ++i)
          {
            const char *end = begin + m_colorFormatInfo.channelList[i].width * m_colorFormatInfo.channelList[i].height * sizeof(T);
            std::copy(begin, end, reinterpret_cast<char*>(m_channelGroupList[0][i].begin()));
            defilterIntra<T>(m_channelGroupList[0][i], vfrm.intraPredictModeList[i]);
            begin = end;
          }
        }
        m_currIsFull = true;
      }
      else // PrevFullReference or PrevReference
      {
        ImageChannel<T> *prevChannelList;
        if(m_currIsFull)
          prevChannelList = m_channelGroupList[0];
        else
        {
          for(int i = 0; i < nChannel; ++i)
            m_channelGroupList[1][i] = m_channelGroupList[2][i];
          prevChannelList = m_channelGroupList[1];
        }
        {
          const char *begin = data;
          for(int i = 0; i < nChannel; ++i)
          {
            const char *end = begin + m_colorFormatInfo.channelList[i].width * m_colorFormatInfo.channelList[i].height * sizeof(T);
            std::copy(begin, end, reinterpret_cast<char*>(m_channelGroupList[2][i].begin()));
            defilterIntra<T>(m_channelGroupList[2][i], vfrm.intraPredictModeList[i]);
            begin = end;
          }
        }
        if(vfrm.referenceType == PreviousFullReference)
        {
          for(int i = 0; i < nChannel; ++i)
            defilterDelta(m_channelGroupList[2][i], m_channelGroupList[0][i], vfrm.scalePredictValue, vfrm.movePredictValueX, vfrm.movePredictValueY);
        }
        else if(vfrm.referenceType == PreviousReference)
        {
          for(int i = 0; i < nChannel; ++i)
            defilterDelta(m_channelGroupList[2][i], prevChannelList[i], vfrm.scalePredictValue, vfrm.movePredictValueX, vfrm.movePredictValueY);
        }
        else
          lvdAssert(false);
        m_currIsFull = false;
      }
    }

    inline const ImageChannel<T> &currentChannelList(uint32_t iChannel) const
    {
      lvdAssert(iChannel < m_colorFormatInfo.channelList.size(), "iChannel out of range");
      if(m_currIsFull)
        return m_channelGroupList[0][iChannel];
      else
        return m_channelGroupList[2][iChannel];
    }

  private:
    ImageChannel<T> m_channelGroupList[3][8];

    const MainStruct &m_mainStruct;
    ColorFormatInfo m_colorFormatInfo;
    bool m_currIsFull;
  };
}