#include "../yuv.h"
#include "privateutil.hpp"

#include "yuv_generic.hpp"

using namespace LightVideo;

void lvYUVP2RGBI8(const uint8_t *y, const uint8_t *u, const uint8_t *v, uint8_t *rgb, int stride, int width, int height)
{ yuvp2rgbi(y, u, v, rgb, stride, width, height); }
void lvRGBI2YUVP8(const uint8_t *rgb, int stride, uint8_t *y, uint8_t *u, uint8_t *v, int width, int height)
{ rgbi2yuvp(rgb, stride, y, u, v, width, height); }
void lvYUVAP2RGBAI8(const uint8_t *y, const uint8_t *u, const uint8_t *v, const uint8_t *a, uint8_t *rgba, int stride, int width, int height)
{ yuvap2rgbai(y, u, v, a, rgba, stride, width, height); }
void lvRGBAI2YUVAP8(const uint8_t *rgba, int stride, uint8_t *y, uint8_t *u, uint8_t *v, uint8_t *a, int width, int height)
{ rgbai2yuvap(rgba, stride, y, u, v, a, width, height); }

void lvYUVP2RGBI16(const uint16_t *y, const uint16_t *u, const uint16_t *v, uint16_t *rgb, int stride, int width, int height)
{ yuvp2rgbi(y, u, v, rgb, stride, width, height); }
void lvRGBI2YUVP16(const uint16_t *rgb, int stride, uint16_t *y, uint16_t *u, uint16_t *v, int width, int height)
{ rgbi2yuvp(rgb, stride, y, u, v, width, height); }
void lvYUVAP2RGBAI16(const uint16_t *y, const uint16_t *u, const uint16_t *v, const uint16_t *a, uint16_t *rgba, int stride, int width, int height)
{ yuvap2rgbai(y, u, v, a, rgba, stride, width, height); }
void lvRGBAI2YUVAP16(const uint16_t *rgba, int stride, uint16_t *y, uint16_t *u, uint16_t *v, uint16_t *a, int width, int height)
{ rgbai2yuvap(rgba, stride, y, u, v, a, width, height); }