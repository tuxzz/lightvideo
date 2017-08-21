#pragma once

#include "../struct.hpp"
#include "../imagechannel.hpp"
#include "util_p.hpp"

#if defined(__AVX2__)
#include "defilter_avx2_p.hpp"
#elif defined(__SSE2__)
#include "defilter_sse2_p.hpp"
#else
#include "defilter_generic_p.hpp"
#endif

namespace LightVideoDecoder
{
  template<typename T>static void defilterIntra(ImageChannel<T> &img, IntraPredictMode mode)
  {
    switch(mode)
    {
    case SubTop:
      defilterSubTop<T>(img);
      break;
    case SubLeft:
      defilterSubLeft<T>(img);
      break;
    case SubAvg:
      defilterSubAvg<T>(img);
      break;
    case SubPaeth:
      defilterSubPaeth<T>(img);
      break;
    case NoIntraPredict:
      break;
    default:
      lvdAssert(false);
    }
  }
}