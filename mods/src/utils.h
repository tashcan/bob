#pragma once

#include <string>

bool set_import(const std::string module, const std::string &name, uintptr_t func);

void * il2cpp_resolve_icall_internal(const char*);

template<typename T>
T *il2cpp_resolve_icall(const char* name) {
    return (T*)il2cpp_resolve_icall_internal(name);
}
