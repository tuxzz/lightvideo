#include "../intrafilter.h"
#include "privateutil.hpp"
#include "intrafilter_generic.hpp"

using namespace LightVideo;

struct FilterGroup
{
  void (*filterSubTop8)(uint8_t*, int, int, uint8_t);
  void (*defilterSubTop8)(uint8_t*, int, int);
  void (*filterSubLeft8)(uint8_t*, int, int, uint8_t);
  void (*defilterSubLeft8)(uint8_t*, int, int);
  void (*filterSubAvg8)(uint8_t*, int, int, uint8_t, uint8_t*);
  void (*defilterSubAvg8)(uint8_t*, int, int);
  void (*filterSubPaeth8)(uint8_t*, int, int, uint8_t, uint8_t*);
  void (*defilterSubPaeth8)(uint8_t*, int, int);

  void(*filterSubTop16)(uint16_t*, int, int, uint16_t);
  void(*defilterSubTop16)(uint16_t*, int, int);
  void(*filterSubLeft16)(uint16_t*, int, int, uint16_t);
  void(*defilterSubLeft16)(uint16_t*, int, int);
  void(*filterSubAvg16)(uint16_t*, int, int, uint16_t, uint16_t*);
  void(*defilterSubAvg16)(uint16_t*, int, int);
  void(*filterSubPaeth16)(uint16_t*, int, int, uint16_t, uint16_t*);
  void(*defilterSubPaeth16)(uint16_t*, int, int);
};

static FilterGroup g_generic = {
  lvFilterSubTop8_generic,
  lvDefilterSubTop8_generic,
  lvFilterSubLeft8_generic,
  lvDefilterSubLeft8_generic,
  lvFilterSubAvg8_generic,
  lvDefilterSubAvg8_generic,
  lvFilterSubPaeth8_generic,
  lvDefilterSubPaeth8_generic,
  lvFilterSubTop16_generic,
  lvDefilterSubTop16_generic,
  lvFilterSubLeft16_generic,
  lvDefilterSubLeft16_generic,
  lvFilterSubAvg16_generic,
  lvDefilterSubAvg16_generic,
  lvFilterSubPaeth16_generic,
  lvDefilterSubPaeth16_generic
};

static FilterGroup *g_filterGroup = &g_generic;

void lvFilterSubTop8(uint8_t *data, int width, int height, uint8_t threshold)
{ return g_filterGroup->filterSubTop8(data, width, height, threshold); }
void lvDefilterSubTop8(uint8_t *data, int width, int height)
{ return g_filterGroup->defilterSubTop8(data, width, height); }

void lvFilterSubLeft8(uint8_t *data, int width, int height, uint8_t threshold)
{ return g_filterGroup->filterSubLeft8(data, width, height, threshold); }
void lvDefilterSubLeft8(uint8_t *data, int width, int height)
{ return g_filterGroup->defilterSubLeft8(data, width, height); }

void lvFilterSubAvg8(uint8_t *data, int width, int height, uint8_t threshold, uint8_t *workMem)
{ return g_filterGroup->filterSubAvg8(data, width, height, threshold, workMem); }
void lvDefilterSubAvg8(uint8_t *data, int width, int height)
{ return g_filterGroup->defilterSubAvg8(data, width, height); }

void lvFilterSubPaeth8(uint8_t *data, int width, int height, uint8_t threshold, uint8_t *workMem)
{ return g_filterGroup->filterSubPaeth8(data, width, height, threshold, workMem); }
void lvDefilterSubPaeth8(uint8_t *data, int width, int height)
{ return g_filterGroup->defilterSubPaeth8(data, width, height); }

void lvFilterSubTop16(uint16_t *data, int width, int height, uint16_t threshold)
{ return g_filterGroup->filterSubTop16(data, width, height, threshold); }
void lvDefilterSubTop16(uint16_t *data, int width, int height)
{ return g_filterGroup->defilterSubTop16(data, width, height); }

void lvFilterSubLeft16(uint16_t *data, int width, int height, uint16_t threshold)
{ return g_filterGroup->filterSubLeft16(data, width, height, threshold); }
void lvDefilterSubLeft16(uint16_t *data, int width, int height)
{ return g_filterGroup->defilterSubLeft16(data, width, height); }

void lvFilterSubAvg16(uint16_t *data, int width, int height, uint16_t threshold, uint16_t *workMem)
{ return g_filterGroup->filterSubAvg16(data, width, height, threshold, workMem); }
void lvDefilterSubAvg16(uint16_t *data, int width, int height)
{ return g_filterGroup->defilterSubAvg16(data, width, height); }

void lvFilterSubPaeth16(uint16_t *data, int width, int height, uint16_t threshold, uint16_t *workMem)
{ return g_filterGroup->filterSubPaeth16(data, width, height, threshold, workMem); }
void lvDefilterSubPaeth16(uint16_t *data, int width, int height)
{ return g_filterGroup->defilterSubPaeth16(data, width, height); }