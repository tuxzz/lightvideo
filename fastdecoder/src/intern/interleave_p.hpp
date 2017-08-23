#pragma once

#include <array>
#include "../imagechannel.hpp"

namespace LightVideoDecoder
{
  template<typename T, int N>static inline void convertToInterleave(const std::array<const ImageChannel<T>*, N> &planeList, ImageChannel<T> &target)
  {
    static_assert(N > 1, "N must be greater than 1.");
    lvdAssert(target.width() / N == planeList[0]->width(), "Bad target shape");
    lvdAssert(target.height() == planeList[0]->height(), "Bad target shape");

    int size = target.width() * target.height();
    T *targetData = target.data();
    for(int j = 0; j < N; ++j)
    {
      for(int i = j; i < size; i += N)
        targetData[i] = planeList[j]->data()[i / N];
    }
  }
} // namespace LightVideoDecoder