#include "../error.hpp"

#define _LV_IMPL_ERROR(name, base) \
  name :: name (const char *what_arg) : base (what_arg) {} \
  name :: ~ name () {}
namespace LightVideoDecoder
{
  _LV_IMPL_ERROR(RuntimeError, std::runtime_error);
  _LV_IMPL_ERROR(IOError, RuntimeError);
  _LV_IMPL_ERROR(DataError, RuntimeError);
  _LV_IMPL_ERROR(EOFError, RuntimeError);
} // namespace LightVideoDecoder
#undef _LV_IMPL_ERROR