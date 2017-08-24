#pragma once

#include "../colorformat.hpp"
#include "../imagechannel.hpp"
#include "decoder_p.hpp"
#include "util_p.hpp"
#include "defilter_dispatcher_p.hpp"
#include "interleave_p.hpp"
#include <vector>
#include "glad/glad.h"

namespace LightVideoDecoder
{
  static const GLuint internalFormat8[] = {GL_R8, GL_RG8, GL_RGB8, GL_RGBA8};
  static const GLuint format8[] = {GL_RED, GL_RG, GL_RGB, GL_RGBA};

  void initializeDecoder();
  void destroyDecoder();
  void drawNMS();

  template<typename T>
  class DecoderImpl final : public DecoderPrivate
  {
  public:
    inline DecoderImpl(const MainStruct &mainStruct) : m_mainStruct(mainStruct), m_currIsFull(false)
    {
      initializeDecoder();
      m_colorFormatInfo = getColorFormatInfo(mainStruct.colorFormat, mainStruct.width, mainStruct.height);

      int nChannel = static_cast<int>(m_colorFormatInfo.channelList.size());
      for(int i = 0; i < nChannel; ++i)
      {
        Size s = m_colorFormatInfo.channelList[i];
        m_deintraBuffer[i] = ImageChannel<T>(s.width, s.height);
      }

      if(mainStruct.colorFormat == YUV420P)
      {
        m_nFS = 1;
        m_nHS = 2;
      }
      else if(mainStruct.colorFormat == YUVA420P)
      {
        m_nFS = 2;
        m_nHS = 2;
      }
      if(m_nFS > 0)
        m_bufferFS = ImageChannel<T>(m_mainStruct.width * m_nFS, m_mainStruct.height);
      if(m_nHS > 0)
        m_bufferHS = ImageChannel<T>(std::max(1U, m_mainStruct.width / 2) * m_nHS, std::max(1U, m_mainStruct.height / 2));
      glGenTextures(4, m_texFS);
      glGenTextures(4, m_texHS);
    }

    inline ~DecoderImpl()
    {
      glDeleteTextures(4, m_texFS);
      glDeleteTextures(4, m_texHS);
      destroyDecoder();
    }

    inline void decodeCurrentFrameData(const VideoFrameStruct &vfrm, const char *data)
    {
      int nChannel = static_cast<int>(m_colorFormatInfo.channelList.size());

      // deintra
      {
        const char *begin = data;
        for(int i = 0; i < nChannel; ++i)
        {
          const char *end = begin + m_colorFormatInfo.channelList[i].width * m_colorFormatInfo.channelList[i].height * sizeof(T);
          std::copy(begin, end, reinterpret_cast<char*>(m_deintraBuffer[i].begin()));
          defilterIntra<T>(m_deintraBuffer[i], vfrm.intraPredictModeList[i]);
          begin = end;
        }
      }
      // convert to interleaved
      {
        if(m_mainStruct.colorFormat == YUV420P)
          std::copy(m_deintraBuffer[0].begin(), m_deintraBuffer[0].end(), m_bufferFS.begin());
        else if(m_mainStruct.colorFormat == YUVA420P)
          convertToInterleave<T, 2>({&m_deintraBuffer[0], &m_deintraBuffer[3]}, m_bufferFS);
        convertToInterleave<T, 2>({&m_deintraBuffer[1], &m_deintraBuffer[2]}, m_bufferHS);
      }

      if(m_nFS > 0)
      {
        glBindTexture(GL_TEXTURE_2D, m_texFS[3]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat8[m_nFS - 1], m_mainStruct.width, m_mainStruct.height, 0, format8[m_nFS - 1], GL_UNSIGNED_BYTE, m_bufferFS.data());
      }
      if(m_nHS > 0)
      {
        glBindTexture(GL_TEXTURE_2D, m_texHS[3]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat8[m_nHS - 1], std::max(1U, m_mainStruct.width / 2), std::max(1U, m_mainStruct.height / 2), 0, format8[m_nHS - 1], GL_UNSIGNED_BYTE, m_bufferHS.data());
      }

      if(vfrm.referenceType == NoReference)
      {
        std::swap(m_texFS[2], m_texFS[3]);
        std::swap(m_texHS[2], m_texHS[3]);
        m_currIsFull = true;
      }
      else // PrevFullReference or PrevReference
      {
        GLuint texPrevFS, texPrevHS;
        if(m_currIsFull)
        {
          std::swap(m_texFS[0], m_texFS[2]);
          std::swap(m_texHS[0], m_texHS[2]);
          texPrevFS = m_texFS[0];
          texPrevHS = m_texHS[0];
        }
        else
        {
          std::swap(m_texFS[1], m_texFS[2]);
          std::swap(m_texHS[1], m_texHS[2]);
          texPrevFS = m_texFS[1];
          texPrevHS = m_texHS[1];
        }

        GLuint refFS, refHS;
        if(vfrm.referenceType == PreviousFullReference)
        {
          refFS = m_texFS[0];
          refHS = m_texHS[0];
        }
        else
        {
          refFS = texPrevFS;
          refHS = texPrevHS;
        }

        GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
        GLuint gBuffer;
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        
        glBindTexture(GL_TEXTURE_2D, m_texFS[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat8[m_nFS - 1], m_mainStruct.width, m_mainStruct.height, 0, format8[m_nFS - 1], GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texFS[2], 0);
        glDrawBuffers(1, attachments);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texFS[3]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, refFS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glViewport(0, 0, m_mainStruct.width, m_mainStruct.height);
        drawNMS();

        glBindTexture(GL_TEXTURE_2D, m_texHS[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat8[m_nHS - 1], std::max(1U, m_mainStruct.width / 2), std::max(1U, m_mainStruct.height / 2), 0, format8[m_nHS - 1], GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texHS[2], 0);
        glDrawBuffers(1, attachments);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texHS[3]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, refHS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glViewport(0, 0, std::max(1U, m_mainStruct.width / 2), std::max(1U, m_mainStruct.height / 2));
        drawNMS();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &gBuffer);

        m_currIsFull = false;
      }
    }

    inline uint32_t currentTextureFS() const
    { return m_texFS[2]; }

    inline uint32_t currentTextureHS() const
    { return m_texHS[2]; }

  private:
    ImageChannel<T> m_deintraBuffer[8];
    ImageChannel<T> m_bufferFS, m_bufferHS;
    GLuint m_texFS[4];
    GLuint m_texHS[4];
    int m_nFS, m_nHS;

    const MainStruct &m_mainStruct;
    ColorFormatInfo m_colorFormatInfo;
    bool m_currIsFull;
  };
}