#pragma once
#include <stdexcept>

#define _LV_DEF_ERROR(name, base) \
  class name : public base \
  { \
  public: \
    name (const char *what_arg); \
    virtual ~ name (); \
  }

namespace LightVideoDecoder
{
  _LV_DEF_ERROR(RuntimeError, std::runtime_error);
  _LV_DEF_ERROR(IOError, RuntimeError);
  _LV_DEF_ERROR(DataError, RuntimeError);
  _LV_DEF_ERROR(EOFError, RuntimeError);
} // namespace LightVideoDecoder
#undef _LV_DEF_ERROR