#include "../intrafilter.h"
#include "privateutil.hpp"

#include "intrafilter_generic.hpp"

using namespace LightVideo;

#ifdef __SSE2__
#include "intrafilter_sse.hpp"
void lvDefilterSubTop8(uint8_t *data, int width, int height, int bpp)
{ return defilter_subtop_nbpp_u8(data, width, height, bpp); }
void lvDefilterSubLeft8(uint8_t *data, int width, int height, int bpp)
{ return defilter_subleft_nbpp_u8(data, width, height, bpp); }
void lvDefilterSubAvg8(uint8_t *data, int width, int height, int bpp)
{ return defilter_subavg_nbpp_u8(data, width, height, bpp); }
#else // __SSE2__
void lvDefilterSubTop8(uint8_t *data, int width, int height, int bpp)
{ return defilterSubTop(data, width, height, bpp); }
void lvDefilterSubLeft8(uint8_t *data, int width, int height, int bpp)
{ return defilterSubLeft(data, width, height, bpp); }
void lvDefilterSubAvg8(uint8_t *data, int width, int height, int bpp)
{ return defilterSubAvg(data, width, height, bpp); }
#endif // __SSE2__
void lvDefilterSubPaeth8(uint8_t *data, int width, int height, int bpp)
{ return defilterSubPaeth(data, width, height, bpp); }
void lvDefilterSubPaeth16(uint16_t *data, int width, int height, int bpp)
{ return defilterSubPaeth(data, width, height, bpp); }
void lvDefilterSubTop16(uint16_t *data, int width, int height, int bpp)
{ return defilterSubTop(data, width, height, bpp); }
void lvDefilterSubLeft16(uint16_t *data, int width, int height, int bpp)
{ return defilterSubLeft(data, width, height, bpp); }
void lvDefilterSubAvg16(uint16_t *data, int width, int height, int bpp)
{ return defilterSubAvg(data, width, height, bpp); }

void lvFilterSubTop8(uint8_t *data, int width, int height, uint8_t threshold)
{ return filterSubTop(data, width, height, threshold); }
void lvFilterSubLeft8(uint8_t *data, int width, int height, uint8_t threshold)
{ return filterSubLeft(data, width, height, threshold); }
void lvFilterSubAvg8(uint8_t *data, int width, int height, uint8_t threshold)
{ return filterSubAvg(data, width, height, threshold); }
void lvFilterSubPaeth8(uint8_t *data, int width, int height, uint8_t threshold)
{ return filterSubPaeth(data, width, height, threshold); }
void lvFilterSubTop16(uint16_t *data, int width, int height, uint16_t threshold)
{ return filterSubTop(data, width, height, threshold); }
void lvFilterSubLeft16(uint16_t *data, int width, int height, uint16_t threshold)
{ return filterSubLeft(data, width, height, threshold); }
void lvFilterSubAvg16(uint16_t *data, int width, int height, uint16_t threshold)
{ return filterSubAvg(data, width, height, threshold); }
void lvFilterSubPaeth16(uint16_t *data, int width, int height, uint16_t threshold)
{ return filterSubPaeth(data, width, height, threshold); }