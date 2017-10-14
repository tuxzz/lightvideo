#ifndef ASTC_H
#define ASTC_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>

#ifdef BUILDING_DLL
  #define ASTC_EXPORT __declspec(dllexport)
#else
  #define ASTC_EXPORT
#endif // BUILDING_DLL

typedef struct
{
  int width, height;
  int blockWidth, blockHeight;
} astc_header_info;

ASTC_EXPORT int astc_main(int argc, char **argv, int width, int height, void *inputData, void *out_data);
ASTC_EXPORT int astc_calc_compressed_size(int width, int height, int blockWidth, int blockHeight);
ASTC_EXPORT astc_header_info astc_get_header_info(void *inputData);
ASTC_EXPORT uint16_t astc_float_to_half(float f32);
ASTC_EXPORT float astc_half_to_float(uint16_t f16);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ASTC_H