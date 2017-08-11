#include "../yuv.h"
#include "yuv_generic.hpp"
#include "yuv_vhfp.hpp"

using namespace LightVideo;

struct ConverterGroup
{
  void (*yuvp2rgbi8)(const uint8_t *y, const uint8_t *u, const uint8_t *v, uint8_t *rgb, int stride, int width, int height);
  void (*rgbi2yuvp8)(const uint8_t *rgb, int stride, uint8_t *y, uint8_t *u, uint8_t *v, int width, int height);
  void (*yuvap2rgbai8)(const uint8_t *y, const uint8_t *u, const uint8_t *v, const uint8_t *a, uint8_t *rgba, int stride, int width, int height);
  void (*rgbai2yuvap8)(const uint8_t *rgba, int stride, uint8_t *y, uint8_t *u, uint8_t *v, uint8_t *a, int width, int height);

  void (*yuvp2rgbi16)(const uint16_t *y, const uint16_t *u, const uint16_t *v, uint16_t *rgb, int stride, int width, int height);
  void (*rgbi2yuvp16)(const uint16_t *rgb, int stride, uint16_t *y, uint16_t *u, uint16_t *v, int width, int height);
  void (*yuvap2rgbai16)(const uint16_t *y, const uint16_t *u, const uint16_t *v, const uint16_t *a, uint16_t *rgba, int stride, int width, int height);
  void (*rgbai2yuvap16)(const uint16_t *rgba, int stride, uint16_t *y, uint16_t *u, uint16_t *v, uint16_t *a, int width, int height);
};

static ConverterGroup g_generic = {
  yuvp2rgbi8_generic,
  rgbi2yuvp8_generic,
  yuvap2rbgai8_generic,
  rgbai2yuvap8_generic,
  yuvp2rgbi16_generic,
  rgbi2yuvp16_generic,
  yuvap2rbgai16_generic,
  rgbai2yuvap16_generic
};

static ConverterGroup g_vhfp = {
  yuvp2rgbi8_vhfp,
  rgbi2yuvp8_vhfp,
  yuvap2rbgai8_vhfp,
  rgbai2yuvap8_vhfp,
  yuvp2rgbi16_vhfp,
  rgbi2yuvp16_vhfp,
  yuvap2rbgai16_vhfp,
  rgbai2yuvap16_vhfp
};

static ConverterGroup *g_converterGroup = &g_generic;

void lvYUVP2RGBI8(const uint8_t *y, const uint8_t *u, const uint8_t *v, uint8_t *rgb, int stride, int width, int height)
{ g_converterGroup->yuvp2rgbi8(y, u, v, rgb, stride, width, height); }
void lvRGBI2YUVP8(const uint8_t *rgb, int stride, uint8_t *y, uint8_t *u, uint8_t *v, int width, int height)
{ g_converterGroup->rgbi2yuvp8(rgb, stride, y, u, v, width, height); }
void lvYUVAP2RGBAI8(const uint8_t *y, const uint8_t *u, const uint8_t *v, const uint8_t *a, uint8_t *rgba, int stride, int width, int height)
{ g_converterGroup->yuvap2rgbai8(y, u, v, a, rgba, stride, width, height); }
void lvRGBAI2YUVAP8(const uint8_t *rgba, int stride, uint8_t *y, uint8_t *u, uint8_t *v, uint8_t *a, int width, int height)
{ g_converterGroup->rgbai2yuvap8(rgba, stride, y, u, v, a, width, height); }

void lvYUVP2RGBI16(const uint16_t *y, const uint16_t *u, const uint16_t *v, uint16_t *rgb, int stride, int width, int height)
{ g_converterGroup->yuvp2rgbi16(y, u, v, rgb, stride, width, height); }
void lvRGBI2YUVP16(const uint16_t *rgb, int stride, uint16_t *y, uint16_t *u, uint16_t *v, int width, int height)
{ g_converterGroup->rgbi2yuvp16(rgb, stride, y, u, v, width, height); }
void lvYUVAP2RGBAI16(const uint16_t *y, const uint16_t *u, const uint16_t *v, const uint16_t *a, uint16_t *rgba, int stride, int width, int height)
{ g_converterGroup->yuvap2rgbai16(y, u, v, a, rgba, stride, width, height); }
void lvRGBAI2YUVAP16(const uint16_t *rgba, int stride, uint16_t *y, uint16_t *u, uint16_t *v, uint16_t *a, int width, int height)
{ g_converterGroup->rgbai2yuvap16(rgba, stride, y, u, v, a, width, height); }