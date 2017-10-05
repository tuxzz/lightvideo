#include "../intrafilter.h"
#include "privateutil.hpp"

#include "intrafilter_generic.hpp"

using namespace LightVideo;

void lvFilterSubTop8(uint8_t *data, int width, int height, uint8_t threshold)
{ return filterSubTop(data, width, height, threshold); }
void lvDefilterSubTop8(uint8_t *data, int width, int height, int bpp)
{ return defilterSubTop(data, width, height, bpp); }

void lvFilterSubLeft8(uint8_t *data, int width, int height, uint8_t threshold)
{ return filterSubLeft(data, width, height, threshold); }
void lvDefilterSubLeft8(uint8_t *data, int width, int height, int bpp)
{ return defilterSubLeft(data, width, height, bpp); }

void lvFilterSubAvg8(uint8_t *data, int width, int height, uint8_t threshold)
{ return filterSubAvg(data, width, height, threshold); }
void lvDefilterSubAvg8(uint8_t *data, int width, int height, int bpp)
{ return defilterSubAvg(data, width, height, bpp); }

void lvFilterSubPaeth8(uint8_t *data, int width, int height, uint8_t threshold)
{ return filterSubPaeth(data, width, height, threshold); }
void lvDefilterSubPaeth8(uint8_t *data, int width, int height, int bpp)
{ return defilterSubPaeth(data, width, height, bpp); }

void lvFilterSubTop16(uint16_t *data, int width, int height, uint16_t threshold)
{ return filterSubTop(data, width, height, threshold); }
void lvDefilterSubTop16(uint16_t *data, int width, int height, int bpp)
{ return defilterSubTop(data, width, height, bpp); }

void lvFilterSubLeft16(uint16_t *data, int width, int height, uint16_t threshold)
{ return filterSubLeft(data, width, height, threshold); }
void lvDefilterSubLeft16(uint16_t *data, int width, int height, int bpp)
{ return defilterSubLeft(data, width, height, bpp); }

void lvFilterSubAvg16(uint16_t *data, int width, int height, uint16_t threshold)
{ return filterSubAvg(data, width, height, threshold); }
void lvDefilterSubAvg16(uint16_t *data, int width, int height, int bpp)
{ return defilterSubAvg(data, width, height, bpp); }

void lvFilterSubPaeth16(uint16_t *data, int width, int height, uint16_t threshold)
{ return filterSubPaeth(data, width, height, threshold); }
void lvDefilterSubPaeth16(uint16_t *data, int width, int height, int bpp)
{ return defilterSubPaeth(data, width, height, bpp); }