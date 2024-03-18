#include "il2cpp_helper.h"

Il2CppObject* get_target(uint32_t handle)
{
  static auto il2cpp_gchandle_get_target =
      (il2cpp_gchandle_get_target_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_gchandle_get_target"));
  return il2cpp_gchandle_get_target(handle);
}