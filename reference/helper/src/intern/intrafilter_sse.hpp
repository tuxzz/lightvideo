#pragma once

#include "privateutil.hpp"
#include <emmintrin.h>
#include <cstdint>

#ifndef __SSE2__
  #error "No SSE2 support found"
#endif // __SSE2__

#define DEPNG_SSE2_SLL_ADDB_1X(P0, T0, Shift) \
  do { \
    T0 = _mm_slli_si128(P0, Shift); \
    P0 = _mm_add_epi8(P0, T0); \
  } while (0)

#define DEPNG_SSE2_SLL_ADDB_2X(P0, T0, P1, T1, Shift) \
  do { \
    T0 = _mm_slli_si128(P0, Shift); \
    T1 = _mm_slli_si128(P1, Shift); \
    P0 = _mm_add_epi8(P0, T0); \
    P1 = _mm_add_epi8(P1, T1); \
  } while (0)

namespace LightVideo
{
  void defilter_subtop_nbpp_u8(uint8_t *data, const int width, const int height, const int bpp)
  {
    lvAssert(bpp >= 1 && bpp <= 8 && bpp != 5 && bpp != 7, "bpp must be in {1, 2, 3, 4, 6, 8}");
    lvAssert(width > 0 && height > 0, "width and height must be greater than 0");
    const int wb = width * bpp;
    if(wb <= 32)
    {
      for(int y = 1; y < height; ++y)
      {
        for(int xb = 0; xb < wb; ++xb)
          data[y * wb + xb] += data[(y - 1) * wb + xb];
      }
    }
    else
    {
      for(int y = 1; y < height; ++y)
      {
        int alignDiff = (16 - (reinterpret_cast<size_t>(data) % 16)) % 16;
        for(int xb = 0; xb < alignDiff; ++xb)
          data[y * wb + xb] += data[(y - 1) * wb + xb];
        for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 16; xb += 16)
        {
          __m128i a = _mm_load_si128(reinterpret_cast<__m128i*>(&(data[(y - 1) * wb + xb])));
          __m128i b = _mm_load_si128(reinterpret_cast<__m128i*>(&(data[y * wb + xb])));
          a = _mm_add_epi8(a, b);
          _mm_store_si128(reinterpret_cast<__m128i*>(&(data[y * wb + xb])), a);
        }
        for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb)
          data[y * wb + xb] += data[(y - 1) * wb + xb];
      }
    }
  }
  
  void defilter_subleft_nbpp_u8(uint8_t *data, const int width, const int height, const int bpp)
  {
    lvAssert(bpp >= 1 && bpp <= 8 && bpp != 5 && bpp != 7, "bpp must be in {1, 2, 3, 4, 6, 8}");
    lvAssert(width > 0 && height > 0, "width and height must be greater than 0");
    const int wb = width * bpp;
    if(bpp == 1)
    {
      if(wb <= 32)
      {
        for(int y = 0; y < height; ++y)
        {
          for(int xb = 1; xb < wb; ++xb)
            data[y * wb + xb] += data[y * wb + (xb - 1)];
        }
      }
      else
      {
        for(int y = 0; y < height; ++y)
        {
          int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
          if(alignDiff < bpp)
            alignDiff += 16;
          for(int xb = 1; xb < alignDiff; ++xb)
            data[y * wb + xb] += data[y * wb + (xb - 1)];

          __m128i p0, p1, p2, p3;
          __m128i t0, t2;
          p0 = _mm_cvtsi32_si128(data[y * wb + (alignDiff - 1)]);

          // 4 * 128bit per loop
          for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 64; xb += 64)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));
            p1 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16));
            p2 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32));
            p3 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48));

            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 1);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 2);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 4);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 8);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);

            p0 = _mm_srli_si128(p0, 15);
            t2 = _mm_srli_si128(p2, 15);
            p1 = _mm_add_epi8(p1, p0);
            p3 = _mm_add_epi8(p3, t2);

            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 1);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 2);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 4);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 8);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16), p1);

            p1 = _mm_unpackhi_epi8(p1, p1);
            p1 = _mm_unpackhi_epi16(p1, p1);
            p1 = _mm_shuffle_epi32(p1, _MM_SHUFFLE(3, 3, 3, 3));

            p2 = _mm_add_epi8(p2, p1);
            p3 = _mm_add_epi8(p3, p1);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32), p2);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48), p3);
            p0 = _mm_srli_si128(p3, 15);
          }

          // 128bit per loop
          for(int xb = wb - (wb - alignDiff) % 64; xb < wb - (wb - alignDiff) % 16; xb += 16)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));

            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 1);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 2);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 4);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 8);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
            p0 = _mm_srli_si128(p0, 15);
          }

          for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb)
            data[y * wb + xb] += data[y * wb + (xb - 1)];
        }
      }
    }
    else if(bpp == 2)
    {
      if(wb <= 32)
      {
        for(int y = 0; y < height; ++y)
        {
          for(int xb = 2; xb < wb; xb += 2)
          {
            data[y * wb + xb] += data[y * wb + (xb - 2)];
            data[y * wb + (xb + 1)] += data[y * wb + (xb - 1)];
          }
        }
      }
      else
      {
        for(int y = 0; y < height; ++y)
        {
          int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
          if(alignDiff < bpp)
            alignDiff += 16;
          for(int xb = 2; xb < alignDiff; ++xb) // we can't unroll this loop due to alignDiff may be not multiply of bpp
            data[y * wb + xb] += data[y * wb + (xb - 2)];

          __m128i p0, p1, p2, p3;
          __m128i t0, t2;
          p0 = _mm_cvtsi32_si128(reinterpret_cast<uint16_t*>(data + y * wb + (alignDiff - 2))[0]);

          // 4 * 128bit per loop
          for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 64; xb += 64)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));
            p1 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16));
            p2 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32));
            p3 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48));

            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 2);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 4);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 8);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);

            p0 = _mm_srli_si128(p0, 14);
            t2 = _mm_srli_si128(p2, 14);
            p1 = _mm_add_epi8(p1, p0);
            p3 = _mm_add_epi8(p3, t2);

            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 2);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 4);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 8);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16), p1);

            p1 = _mm_unpackhi_epi16(p1, p1);
            p1 = _mm_shuffle_epi32(p1, _MM_SHUFFLE(3, 3, 3, 3));

            p2 = _mm_add_epi8(p2, p1);
            p3 = _mm_add_epi8(p3, p1);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32), p2);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48), p3);
            p0 = _mm_srli_si128(p3, 14);
          }

          // 128bit per loop
          for(int xb = wb - (wb - alignDiff) % 64; xb < wb - (wb - alignDiff) % 16; xb += 16)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 2);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 4);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 8);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
            p0 = _mm_srli_si128(p0, 14);
          }
          for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb) // can't unroll
            data[y * wb + xb] += data[y * wb + (xb - 2)];
        }
      }
    }
    else if(bpp == 3)
    {
      if(wb <= 32)
      {
        for(int y = 0; y < height; ++y)
        {
          for(int xb = 3; xb < wb; xb += 3)
          {
            data[y * wb + xb] += data[y * wb + (xb - 3)];
            data[y * wb + (xb + 1)] += data[y * wb + (xb - 2)];
            data[y * wb + (xb + 2)] += data[y * wb + (xb - 1)];
          }
        }
      }
      else
      {
        for(int y = 0; y < height; ++y)
        {
          int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
          if(alignDiff < bpp)
            alignDiff += 16;
          for(int xb = 3; xb < alignDiff; ++xb) // we can't unroll this loop due to alignDiff may be not multiply of bpp
            data[y * wb + xb] += data[y * wb + (xb - 3)];

          __m128i p0, p1, p2, p3;
          __m128i t0, t2;
          __m128i ext3b = _mm_set1_epi32(0x01000001);
          p0 = _mm_cvtsi32_si128(reinterpret_cast<uint32_t*>(data + y * wb + (alignDiff - 3))[0] & 0x00FFFFFFU); // little-endian only

                                                                                                                 // 4 * 128bit per loop
          for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 64; xb += 64)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));
            p1 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16));
            p2 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32));

            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 3);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 6);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t2, 12);

            p3 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48));
            t0 = _mm_srli_si128(p0, 13);
            t2 = _mm_srli_si128(p2, 13);

            p1 = _mm_add_epi8(p1, t0);
            p3 = _mm_add_epi8(p3, t2);

            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 3);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 6);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t2, 12);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);

            p0 = _mm_shuffle_epi32(p1, _MM_SHUFFLE(3, 3, 3, 3));
            p0 = _mm_srli_epi32(p0, 8);
            p0 = _mm_mul_epu32(p0, ext3b);

            p0 = _mm_shufflelo_epi16(p0, _MM_SHUFFLE(0, 2, 1, 0));
            p0 = _mm_shufflehi_epi16(p0, _MM_SHUFFLE(1, 0, 2, 1));

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16), p1);
            p2 = _mm_add_epi8(p2, p0);
            p0 = _mm_shuffle_epi32(p0, _MM_SHUFFLE(1, 3, 2, 1));

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32), p2);
            p0 = _mm_add_epi8(p0, p3);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48), p0);
            p0 = _mm_srli_si128(p0, 13);
          }

          // 128bit per loop
          for(int xb = wb - (wb - alignDiff) % 64; xb < wb - (wb - alignDiff) % 16; xb += 16)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));

            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 3);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 6);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 12);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
            p0 = _mm_srli_si128(p0, 13);
          }
          for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb) // can't unroll
            data[y * wb + xb] += data[y * wb + (xb - 3)];
        }
      }
    }
    else if(bpp == 4)
    {
      if(wb <= 32)
      {
        for(int y = 0; y < height; ++y)
        {
          for(int xb = 4; xb < wb; xb += 4)
          {
            data[y * wb + xb] += data[y * wb + (xb - 4)];
            data[y * wb + (xb + 1)] += data[y * wb + (xb - 3)];
            data[y * wb + (xb + 2)] += data[y * wb + (xb - 2)];
            data[y * wb + (xb + 3)] += data[y * wb + (xb - 1)];
          }
        }
      }
      else
      {
        for(int y = 0; y < height; ++y)
        {
          int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
          if(alignDiff < bpp)
            alignDiff += 16;
          for(int xb = 4; xb < alignDiff; ++xb) // we can't unroll this loop due to alignDiff may be not multiply of bpp
            data[y * wb + xb] += data[y * wb + (xb - 4)];

          __m128i p0, p1, p2, p3;
          __m128i t0, t1, t2;
          p0 = _mm_cvtsi32_si128(reinterpret_cast<uint32_t*>(data + y * wb + (alignDiff - 4))[0]);

          // 4 * 128bit per loop
          for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 64; xb += 64)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));
            p1 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + (xb + 16)));
            p2 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + (xb + 32)));
            p3 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + (xb + 48)));

            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t1, 4);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t1, 8);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);

            p0 = _mm_srli_si128(p0, 12);
            t2 = _mm_srli_si128(p2, 12);

            p1 = _mm_add_epi8(p1, p0);
            p3 = _mm_add_epi8(p3, t2);

            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t1, 4);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t1, 8);

            p0 = _mm_shuffle_epi32(p1, _MM_SHUFFLE(3, 3, 3, 3));
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + (xb + 16)), p1);

            p2 = _mm_add_epi8(p2, p0);
            p0 = _mm_add_epi8(p0, p3);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + (xb + 32)), p2);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + (xb + 48)), p0);
            p0 = _mm_srli_si128(p0, 12);
          }

          // 128bit per loop
          for(int xb = wb - (wb - alignDiff) % 64; xb < wb - (wb - alignDiff) % 16; xb += 16)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));

            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 4);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 8);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
            p0 = _mm_srli_si128(p0, 12);
          }
          for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb) // can't unroll
            data[y * wb + xb] += data[y * wb + (xb - 4)];
        }
      }
    }
    else if(bpp == 6)
    {
      if(wb <= 32)
      {
        for(int y = 0; y < height; ++y)
        {
          for(int xb = 6; xb < wb; xb += 6)
          {
            data[y * wb + xb] += data[y * wb + (xb - 6)];
            data[y * wb + (xb + 1)] += data[y * wb + (xb - 5)];
            data[y * wb + (xb + 2)] += data[y * wb + (xb - 4)];
            data[y * wb + (xb + 3)] += data[y * wb + (xb - 3)];
            data[y * wb + (xb + 4)] += data[y * wb + (xb - 2)];
            data[y * wb + (xb + 5)] += data[y * wb + (xb - 1)];
          }
        }
      }
      else
      {
        for(int y = 0; y < height; ++y)
        {
          int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
          if(alignDiff < bpp)
            alignDiff += 16;
          for(int xb = 6; xb < alignDiff; ++xb) // we can't unroll this loop due to alignDiff may be not multiply of bpp
            data[y * wb + xb] += data[y * wb + (xb - 6)];

          __m128i p0, p1, p2, p3;
          __m128i t0, t1;
          p0 = _mm_loadl_epi64(reinterpret_cast<__m128i*>(data + y * wb + alignDiff - 6));
          p0 = _mm_slli_epi64(p0, 16);
          p0 = _mm_srli_epi64(p0, 16);
          // 4 * 128bit per loop
          for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 64; xb += 64)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));
            p1 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16));
            p2 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32));

            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t1, 6);
            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t1, 12);

            p3 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48));
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);

            p0 = _mm_srli_si128(p0, 10);
            t1 = _mm_srli_si128(p2, 10);

            p1 = _mm_add_epi8(p1, p0);
            p3 = _mm_add_epi8(p3, t1);

            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t1, 6);
            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t1, 12);
            p0 = _mm_shuffle_epi32(p1, _MM_SHUFFLE(3, 2, 3, 2));

            p0 = _mm_shufflelo_epi16(p0, _MM_SHUFFLE(1, 3, 2, 1));
            p0 = _mm_shufflehi_epi16(p0, _MM_SHUFFLE(2, 1, 3, 2));

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16), p1);
            p2 = _mm_add_epi8(p2, p0);
            p0 = _mm_shuffle_epi32(p0, _MM_SHUFFLE(1, 3, 2, 1));

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32), p2);
            p0 = _mm_add_epi8(p0, p3);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48), p0);
            p0 = _mm_srli_si128(p0, 10);
          }

          // 128bit per loop
          for(int xb = wb - (wb - alignDiff) % 64; xb < wb - (wb - alignDiff) % 16; xb += 16)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));

            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 6);
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 12);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
            p0 = _mm_srli_si128(p0, 10);
          }
          for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb) // can't unroll
            data[y * wb + xb] += data[y * wb + (xb - 6)];
        }
      }
    }
    else if(bpp == 8)
    {
      if(wb <= 32)
      {
        for(int y = 0; y < height; ++y)
        {
          for(int xb = 8; xb < wb; xb += 8)
          {
            data[y * wb + xb] += data[y * wb + (xb - 8)];
            data[y * wb + (xb + 1)] += data[y * wb + (xb - 7)];
            data[y * wb + (xb + 2)] += data[y * wb + (xb - 6)];
            data[y * wb + (xb + 3)] += data[y * wb + (xb - 5)];
            data[y * wb + (xb + 4)] += data[y * wb + (xb - 4)];
            data[y * wb + (xb + 5)] += data[y * wb + (xb - 3)];
            data[y * wb + (xb + 6)] += data[y * wb + (xb - 2)];
            data[y * wb + (xb + 7)] += data[y * wb + (xb - 1)];
          }
        }
      }
      else
      {
        for(int y = 0; y < height; ++y)
        {
          int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
          if(alignDiff < bpp)
            alignDiff += 16;
          for(int xb = 8; xb < alignDiff; ++xb) // we can't unroll this loop due to alignDiff may be not multiply of bpp
            data[y * wb + xb] += data[y * wb + (xb - 8)];

          __m128i p0, p1, p2, p3;
          __m128i t0, t1, t2;
          p0 = _mm_loadl_epi64(reinterpret_cast<__m128i*>(data + y * wb + alignDiff - 8));
          // 4 * 128bit per loop
          for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 64; xb += 64)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));
            p1 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16));
            p2 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32));
            p3 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48));

            DEPNG_SSE2_SLL_ADDB_2X(p0, t0, p2, t1, 8);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);

            p0 = _mm_srli_si128(p0, 8);
            t2 = _mm_shuffle_epi32(p2, _MM_SHUFFLE(3, 2, 3, 2));
            p1 = _mm_add_epi8(p1, p0);

            DEPNG_SSE2_SLL_ADDB_2X(p1, t0, p3, t1, 8);
            p0 = _mm_shuffle_epi32(p1, _MM_SHUFFLE(3, 2, 3, 2));
            p3 = _mm_add_epi8(p3, t2);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 16), p1);

            p2 = _mm_add_epi8(p2, p0);
            p0 = _mm_add_epi8(p0, p3);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 32), p2);
            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb + 48), p0);
            p0 = _mm_srli_si128(p0, 8);
          }

          // 128bit per loop
          for(int xb = wb - (wb - alignDiff) % 64; xb < wb - (wb - alignDiff) % 16; xb += 16)
          {
            p0 = _mm_add_epi8(p0, *reinterpret_cast<__m128i*>(data + y * wb + xb));
            DEPNG_SSE2_SLL_ADDB_1X(p0, t0, 8);

            _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
            p0 = _mm_srli_si128(p0, 8);
          }
          for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb) // can't unroll
            data[y * wb + xb] += data[y * wb + (xb - 8)];
        }
      }
    }
    else
      std::abort();
  }
  
  void defilter_subavg_nbpp_u8(uint8_t *data, const int width, const int height, const int bpp)
  {
    lvAssert(bpp >= 1 && bpp <= 8 && bpp != 5 && bpp != 7, "bpp must be in {1, 2, 3, 4, 6, 8}");
    lvAssert(width > 0 && height > 0, "width and height must be greater than 0");
    const int wb = width * bpp;
    __m128i zeros = _mm_setzero_si128();
    if(wb <= 32)
    {
      for(int y = 1; y < height; ++y)
      {
        for(int xb = bpp; xb < wb; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - bpp)])) / 2;
          data[y * wb + xb] += avg;
        }
      }
    }
    else if(bpp == 1)
    {
      for(int y = 1; y < height; ++y)
      {
        int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
        if(alignDiff < bpp)
          alignDiff += 16;
        for(int xb = 1; xb < alignDiff; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - 1)])) / 2;
          data[y * wb + xb] += avg;
        }

        uint32_t t0 = data[y * wb + (alignDiff - 1)];
        uint32_t t1;
        for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 8; xb += 8)
        {
          __m128i p0 = _mm_loadl_epi64(reinterpret_cast<__m128i*>(data + y * wb + xb));
          __m128i u0 = _mm_loadl_epi64(reinterpret_cast<__m128i*>(data + (y - 1) * wb + xb));

          p0 = _mm_unpacklo_epi8(p0, zeros);
          u0 = _mm_unpacklo_epi8(u0, zeros);

          p0 = _mm_slli_epi16(p0, 1);
          p0 = _mm_add_epi16(p0, u0);

          t1 = _mm_cvtsi128_si32(p0);
          p0 = _mm_srli_si128(p0, 4);
          t0 = ((t0 + t1) >> 1) & 0xFF; t1 >>= 16;
          data[y * wb + xb] = static_cast<uint8_t>(t0);

          t0 = ((t0 + t1) >> 1) & 0xFF;
          t1 = _mm_cvtsi128_si32(p0);
          p0 = _mm_srli_si128(p0, 4);
          data[y * wb + (xb + 1)] = static_cast<uint8_t>(t0);

          t0 = ((t0 + t1) >> 1) & 0xFF; t1 >>= 16;
          data[y * wb + (xb + 2)] = static_cast<uint8_t>(t0);

          t0 = ((t0 + t1) >> 1) & 0xFF;
          t1 = _mm_cvtsi128_si32(p0);
          p0 = _mm_srli_si128(p0, 4);
          data[y * wb + (xb + 3)] = static_cast<uint8_t>(t0);

          t0 = ((t0 + t1) >> 1) & 0xFF; t1 >>= 16;
          data[y * wb + (xb + 4)] = static_cast<uint8_t>(t0);

          t0 = ((t0 + t1) >> 1) & 0xFF;
          t1 = _mm_cvtsi128_si32(p0);
          data[y * wb + (xb + 5)] = static_cast<uint8_t>(t0);

          t0 = ((t0 + t1) >> 1) & 0xFF; t1 >>= 16;
          data[y * wb + (xb + 6)] = static_cast<uint8_t>(t0);

          t0 = ((t0 + t1) >> 1) & 0xFF;
          data[y * wb + (xb + 7)] = static_cast<uint8_t>(t0);
        }
        for(int xb = wb - (wb - alignDiff) % 8; xb < wb; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - 1)])) / 2;
          data[y * wb + xb] += avg;
        }
      }
    }
    else if(bpp == 4)
    {
      for(int y = 1; y < height; ++y)
      {
        int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
        if(alignDiff < bpp)
          alignDiff += 16;
        for(int xb = 4; xb < alignDiff; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - 4)])) / 2;
          data[y * wb + xb] += avg;
        }

        __m128i m00FF = _mm_set1_epi16(0x00FF);
        __m128i m01FF = _mm_set1_epi16(0x01FF);

        __m128i t1 = _mm_unpacklo_epi8(_mm_cvtsi32_si128(reinterpret_cast<uint32_t*>(data + y * wb + (alignDiff - 4))[0]), zeros);
        for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 16; xb += 16)
        {
          __m128i p0, p1;
          __m128i u0, u1;

          p0 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb));
          u0 = _mm_loadu_si128(reinterpret_cast<__m128i*>(data + (y - 1) * wb + xb));

          p1 = p0;                          // HI | Move Ln
          p0 = _mm_unpacklo_epi8(p0, zeros); // LO | Unpack Ln

          u1 = u0;                          // HI | Move Up
          p0 = _mm_slli_epi16(p0, 1);       // LO | << 1

          u0 = _mm_unpacklo_epi8(u0, zeros); // LO | Unpack Up
          p0 = _mm_add_epi16(p0, t1);       // LO | Add Last

          p1 = _mm_unpackhi_epi8(p1, zeros); // HI | Unpack Ln
          p0 = _mm_add_epi16(p0, u0);       // LO | Add Up
          p0 = _mm_and_si128(p0, m01FF);    // LO | & 0x01FE

          u1 = _mm_unpackhi_epi8(u1, zeros); // HI | Unpack Up
          t1 = _mm_slli_si128(p0, 8);       // LO | Get Last
          p0 = _mm_slli_epi16(p0, 1);       // LO | << 1

          p1 = _mm_slli_epi16(p1, 1);       // HI | << 1
          p0 = _mm_add_epi16(p0, t1);       // LO | Add Last
          p0 = _mm_srli_epi16(p0, 2);       // LO | >> 2

          p1 = _mm_add_epi16(p1, u1);       // HI | Add Up
          p0 = _mm_and_si128(p0, m00FF);    // LO | & 0x00FF
          t1 = _mm_srli_si128(p0, 8);       // LO | Get Last

          p1 = _mm_add_epi16(p1, t1);       // HI | Add Last
          p1 = _mm_and_si128(p1, m01FF);    // HI | & 0x01FE

          t1 = _mm_slli_si128(p1, 8);       // HI | Get Last
          p1 = _mm_slli_epi16(p1, 1);       // HI | << 1

          t1 = _mm_add_epi16(t1, p1);       // HI | Add Last
          t1 = _mm_srli_epi16(t1, 2);       // HI | >> 2
          t1 = _mm_and_si128(t1, m00FF);    // HI | & 0x00FF

          p0 = _mm_packus_epi16(p0, t1);
          t1 = _mm_srli_si128(t1, 8);       // HI | Get Last
          _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
        }
        for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - 4)])) / 2;
          data[y * wb + xb] += avg;
        }
      }
    }
    else if(bpp == 6)
    {
      for(int y = 1; y < height; ++y)
      {
        int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
        if(alignDiff < bpp)
          alignDiff += 16;
        for(int xb = 6; xb < alignDiff; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - 6)])) / 2;
          data[y * wb + xb] += avg;
        }

        __m128i t1 = _mm_loadl_epi64(reinterpret_cast<__m128i*>(data + y * wb + (alignDiff - 6)));
        for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 16; xb += 16)
        {
          __m128i p0, p1, p2;
          __m128i u0, u1, u2;

          u0 = _mm_loadu_si128(reinterpret_cast<__m128i*>(data + (y - 1) * wb + xb));
          t1 = _mm_unpacklo_epi8(t1, zeros);
          p0 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb));

          p1 = _mm_srli_si128(p0, 6);       // P1 | Extract
          u1 = _mm_srli_si128(u0, 6);       // P1 | Extract

          p2 = _mm_srli_si128(p0, 12);      // P2 | Extract
          u2 = _mm_srli_si128(u0, 12);      // P2 | Extract

          p0 = _mm_unpacklo_epi8(p0, zeros); // P0 | Unpack
          u0 = _mm_unpacklo_epi8(u0, zeros); // P0 | Unpack

          p1 = _mm_unpacklo_epi8(p1, zeros); // P1 | Unpack
          u1 = _mm_unpacklo_epi8(u1, zeros); // P1 | Unpack

          p2 = _mm_unpacklo_epi8(p2, zeros); // P2 | Unpack
          u2 = _mm_unpacklo_epi8(u2, zeros); // P2 | Unpack

          u0 = _mm_add_epi16(u0, t1);       // P0 | Add Last
          u0 = _mm_srli_epi16(u0, 1);       // P0 | >> 1
          p0 = _mm_add_epi8(p0, u0);        // P0 | Add (Up+Last)/2

          u1 = _mm_add_epi16(u1, p0);       // P1 | Add P0
          u1 = _mm_srli_epi16(u1, 1);       // P1 | >> 1
          p1 = _mm_add_epi8(p1, u1);        // P1 | Add (Up+Last)/2

          u2 = _mm_add_epi16(u2, p1);       // P2 | Add P1
          u2 = _mm_srli_epi16(u2, 1);       // P2 | >> 1
          p2 = _mm_add_epi8(p2, u2);        // P2 | Add (Up+Last)/2

          p0 = _mm_slli_si128(p0, 4);
          p0 = _mm_packus_epi16(p0, p1);
          p0 = _mm_slli_si128(p0, 2);
          p0 = _mm_srli_si128(p0, 4);

          p2 = _mm_packus_epi16(p2, p2);
          p2 = _mm_slli_si128(p2, 12);
          p0 = _mm_or_si128(p0, p2);

          _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
          t1 = _mm_srli_si128(p0, 10);
        }
        for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - 6)])) / 2;
          data[y * wb + xb] += avg;
        }
      }
    }
    else if(bpp == 8)
    {
      for(int y = 1; y < height; ++y)
      {
        int alignDiff = 16 - reinterpret_cast<size_t>(data + y * wb) % 16;
        if(alignDiff < bpp)
          alignDiff += 16;
        for(int xb = 8; xb < alignDiff; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - 8)])) / 2;
          data[y * wb + xb] += avg;
        }

        __m128i t1 = _mm_unpacklo_epi8(_mm_loadl_epi64(reinterpret_cast<__m128i*>(data + y * wb + (alignDiff - 8))), zeros);
        for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 16; xb += 16)
        {
          __m128i p0, p1;
          __m128i u0, u1;

          u0 = _mm_loadu_si128(reinterpret_cast<__m128i*>(data + (y - 1) * wb + xb));
          p0 = _mm_load_si128(reinterpret_cast<__m128i*>(data + y * wb + xb));

          u1 = u0;                          // HI | Move Up
          p1 = p0;                          // HI | Move Ln
          u0 = _mm_unpacklo_epi8(u0, zeros); // LO | Unpack Up
          p0 = _mm_unpacklo_epi8(p0, zeros); // LO | Unpack Ln

          u0 = _mm_add_epi16(u0, t1);       // LO | Add Last
          p1 = _mm_unpackhi_epi8(p1, zeros); // HI | Unpack Ln
          u0 = _mm_srli_epi16(u0, 1);       // LO | >> 1
          u1 = _mm_unpackhi_epi8(u1, zeros); // HI | Unpack Up

          p0 = _mm_add_epi8(p0, u0);        // LO | Add (Up+Last)/2
          u1 = _mm_add_epi16(u1, p0);       // HI | Add LO
          u1 = _mm_srli_epi16(u1, 1);       // HI | >> 1
          p1 = _mm_add_epi8(p1, u1);        // HI | Add (Up+LO)/2

          p0 = _mm_packus_epi16(p0, p1);
          t1 = p1;                          // HI | Get Last
          _mm_store_si128(reinterpret_cast<__m128i*>(data + y * wb + xb), p0);
        }
        for(int xb = wb - (wb - alignDiff) % 16; xb < wb; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - 8)])) / 2;
          data[y * wb + xb] += avg;
        }
      }
    }
    else
    {
      for(int y = 1; y < height; ++y)
      {
        for(int xb = bpp; xb < wb; ++xb)
        {
          uint8_t avg = (static_cast<unsigned int>(data[(y - 1) * wb + xb]) + static_cast<unsigned int>(data[y * wb + (xb - bpp)])) / 2;
          data[y * wb + xb] += avg;
        }
      }
    }
  }

  void defilter_subtop_nbpp_u16(uint16_t *data, const int width, const int height, const int bpp)
  {
    lvAssert(bpp >= 1 && bpp <= 8 && bpp != 5 && bpp != 7, "bpp must be in {1, 2, 3, 4, 6, 8}");
    lvAssert(width > 0 && height > 0, "width and height must be greater than 0");
    const int wb = width * bpp;
    if(wb <= 32)
    {
      for(int y = 1; y < height; ++y)
      {
        for(int xb = 0; xb < wb; ++xb)
          data[y * wb + xb] += data[(y - 1) * wb + xb];
      }
    }
    else
    {
      for(int y = 1; y < height; ++y)
      {
        int alignDiff = (16 - (reinterpret_cast<size_t>(data) % 16)) % 16;
        lvAssert(alignDiff % 2 == 0, "invalid memory alignment");
        alignDiff /= 2;
        for(int xb = 0; xb < alignDiff; ++xb)
          data[y * wb + xb] += data[(y - 1) * wb + xb];
        for(int xb = alignDiff; xb < wb - (wb - alignDiff) % 8; xb += 8)
        {
          __m128i a = _mm_load_si128(reinterpret_cast<__m128i*>(&(data[(y - 1) * wb + xb])));
          __m128i b = _mm_load_si128(reinterpret_cast<__m128i*>(&(data[y * wb + xb])));
          a = _mm_add_epi16(a, b);
          _mm_store_si128(reinterpret_cast<__m128i*>(&(data[y * wb + xb])), a);
        }
        for(int xb = wb - (wb - alignDiff) % 8; xb < wb; ++xb)
          data[y * wb + xb] += data[(y - 1) * wb + xb];
      }
    }
  }
}