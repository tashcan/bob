#pragma once

#include <Windows.h>

#include "utils.h"

#include <libil2cpp/il2cpp-api-types.h>
#include <libil2cpp/il2cpp-class-internals.h>
#include <libil2cpp/il2cpp-config.h>
#include <libil2cpp/il2cpp-object-internals.h>
#include <libil2cpp/utils/Il2CppHashMap.h>

#include <xorstr.hpp>

#define DO_API(r, n, p) using n##_t = r(*) p;
#define DO_API_NO_RETURN(r, n, p) DO_API(r, n, p)
#include <libil2cpp/il2cpp-api-functions.h>
#undef DO_API
#undef DO_API_NO_RETURN

class IL2CppPropertyHelper
{
public:
  IL2CppPropertyHelper(Il2CppClass* cls, const PropertyInfo* propInfo)
  {
    this->cls      = cls;
    this->propInfo = propInfo;
  }

  template <typename T> void SetRaw(void* _this, T& v)
  {
    if (!this->propInfo) {
      return;
    }

    static auto il2cpp_property_get_set_method   = (il2cpp_property_get_set_method_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_property_get_set_method")));
    static auto il2cpp_runtime_invoke            = (il2cpp_runtime_invoke_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_runtime_invoke")));
    static auto il2cpp_object_unbox              = (il2cpp_object_unbox_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_object_unbox")));
    static auto il2cpp_object_get_virtual_method = (il2cpp_object_get_virtual_method_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_object_get_virtual_method")));

    auto set_method         = il2cpp_property_get_set_method((PropertyInfo*)this->propInfo);
    auto set_method_virtual = il2cpp_object_get_virtual_method((Il2CppObject*)_this, set_method);

    Il2CppException* exception = nullptr;
    void*            params[1] = {&v};

    il2cpp_runtime_invoke(set_method_virtual, _this, params, &exception);
  }

  template <typename T = void> T* GetRaw(void* _this)
  {
    if (!this->propInfo) {
      return nullptr;
    }

    static auto il2cpp_property_get_get_method   = (il2cpp_property_get_get_method_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_property_get_get_method")));
    static auto il2cpp_runtime_invoke            = (il2cpp_runtime_invoke_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_runtime_invoke")));
    static auto il2cpp_object_get_virtual_method = (il2cpp_object_get_virtual_method_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_object_get_virtual_method")));

    auto get_method         = il2cpp_property_get_get_method((PropertyInfo*)this->propInfo);
    auto get_method_virtual = il2cpp_object_get_virtual_method((Il2CppObject*)_this, get_method);

    Il2CppException* exception = nullptr;
    auto             result    = il2cpp_runtime_invoke(get_method_virtual, _this, nullptr, &exception);

    if (exception) {
      return nullptr;
    }

    return (T*)(result);
  }

  template <typename T> T* Get(void* _this)
  {
    static auto il2cpp_object_unbox = (il2cpp_object_unbox_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_object_unbox")));
    auto        r                   = GetRaw<Il2CppObject>(_this);
    return !r ? nullptr : (T*)(il2cpp_object_unbox(r));
  }

  template <typename T> T* GetUnboxedSelf(void* _this)
  {
    static auto il2cpp_object_unbox = (il2cpp_object_unbox_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_object_unbox")));
    auto        r                   = GetRaw<Il2CppObject>(il2cpp_object_unbox((Il2CppObject*)_this));
    return !r ? nullptr : (T*)(il2cpp_object_unbox(r));
  }

private:
  Il2CppClass*        cls;
  const PropertyInfo* propInfo;
};

class IL2CppFieldHelper
{
public:
  IL2CppFieldHelper(Il2CppClass* cls, FieldInfo* fieldInfo)
  {
    this->cls       = cls;
    this->fieldInfo = fieldInfo;
  }

  inline ptrdiff_t offset() const
  {
    return this->fieldInfo->offset;
  }

private:
  Il2CppClass* cls;
  FieldInfo*   fieldInfo;
};

class IL2CppStaticFieldHelper
{
public:
  IL2CppStaticFieldHelper(Il2CppClass* cls, FieldInfo* fieldInfo)
  {
    this->cls       = cls;
    this->fieldInfo = fieldInfo;
  }

  template <typename T> inline T Get() const
  {
    static auto il2cpp_field_static_get_value = (il2cpp_field_static_get_value_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_field_static_get_value")));
    T           v;
    il2cpp_field_static_get_value(this->fieldInfo, &v);
    return v;
  }

private:
  Il2CppClass* cls;
  FieldInfo*   fieldInfo;
};

#define GetMethodXor(name) GetMethod(xorstr_(name))

class IL2CppClassHelper
{
public:
  IL2CppClassHelper(Il2CppClass* cls)
  {
    this->cls = cls;
  }

  template <typename T> T* New()
  {
    static auto il2cpp_object_new = (il2cpp_object_new_t)(GetProcAddress(GetModuleHandle(xorstr_("GameAssembly.dll")),
                                                                         xorstr_("il2cpp_object_new")));
    return (T*)il2cpp_object_new(this->cls);
  }

  void* GetType()
  {
    static auto il2cpp_type_get_object = (il2cpp_type_get_object_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_type_get_object")));
    auto        obj                    = il2cpp_type_get_object(&this->cls->byval_arg);
    return obj;
  }

  template <typename T = void> T* GetMethod(const char* name, int arg_count = -1)
  {
    if (!this->cls) {
      return nullptr;
    }

    static auto il2cpp_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_method_from_name")));

    auto fn = il2cpp_class_get_method_from_name(this->cls, name, arg_count);

    return (T*)fn->methodPointer;
  }

  template <typename T = void> T* GetVirtualMethod(const char* name, int arg_count = -1)
  {
    if (!this->cls) {
      return nullptr;
    }

    static auto il2cpp_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_method_from_name")));
    static auto il2cpp_object_get_virtual_method  = (il2cpp_object_get_virtual_method_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_object_get_virtual_method")));

    auto fn = il2cpp_class_get_method_from_name(this->cls, name, arg_count);

    auto get_method_virtual = il2cpp_object_get_virtual_method((Il2CppObject*)this, fn);

    return (T*)get_method_virtual->methodPointer;
  }

  template <typename R, typename... Args> class InvokerMethod
  {

  public:
    InvokerMethod(const MethodInfo* fn)
        : fn(fn)
    {
    }

    R Invoke(void* _this, Args... args)
    {
      static auto il2cpp_runtime_invoke = (il2cpp_runtime_invoke_t)(GetProcAddress(
          GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_runtime_invoke")));
      static auto il2cpp_object_unbox   = (il2cpp_object_unbox_t)(GetProcAddress(
          GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_object_unbox")));

      Il2CppException* exception = nullptr;
      void*            params    = {};
      auto             result    = il2cpp_runtime_invoke(this->fn, _this, &params, &exception);

      return *(R*)il2cpp_object_unbox(result);
    }

    const MethodInfo* fn;
  };

  template <typename R, typename... Args>
  const InvokerMethod<R, Args...> GetInvokeMethod(const char* name, int arg_count = -1)
  {
    if (!this->cls) {
      return nullptr;
    }

    static auto il2cpp_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_method_from_name")));

    auto fn = il2cpp_class_get_method_from_name(this->cls, name, arg_count);

    return InvokerMethod<R, Args...>(fn);
  }

  const MethodInfo*
  GetMethodInfoSpecial(const char*                                                      name,
                       std::function<bool(int param_count, const ParameterInfo* param)> arg_filter = nullptr)
  {
    if (!this->cls) {
      return nullptr;
    }

    static auto il2cpp_class_get_methods = (il2cpp_class_get_methods_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_methods")));

    auto  flags = 0;
    void* iter  = NULL;
    while (const MethodInfo* method = il2cpp_class_get_methods(this->cls, &iter)) {
      if (method->name[0] == name[0] && !strcmp(name, method->name) && ((method->flags & flags) == flags)) {
        if (!arg_filter || arg_filter(method->parameters_count, method->parameters)) {
          return method;
        }
      }
    }
    return nullptr;
  }

  template <typename T = void>
  T* GetMethodSpecial(const char*                                                      name,
                      std::function<bool(int param_count, const ParameterInfo* param)> arg_filter = nullptr)
  {
    if (!this->cls) {
      return nullptr;
    }
    auto info = GetMethodInfoSpecial(name, arg_filter);
    if (info) {
      return (T*)info->methodPointer;
    }
    return nullptr;
  }

  template <typename T = void> T* GetMethodSpecial2(Il2CppObject* obj, const char* name)
  {
    if (!this->cls) {
      return nullptr;
    }

    for (auto i = 0; i < obj->klass->method_count; ++i) {
      auto method = obj->klass->klass->methods[i];
      if (method->name[0] == name[0] && !strcmp(name, method->name)) {
        return (T*)method->methodPointer;
      }
    }
    return nullptr;
  }

  const MethodInfo* GetMethodInfo(const char* name, int arg_count = -1)
  {
    if (!this->cls) {
      return nullptr;
    }

    static auto il2cpp_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_method_from_name")));

    return il2cpp_class_get_method_from_name(this->cls, name, arg_count);
  }

  inline IL2CppPropertyHelper GetProperty(const char* name)
  {
    static auto il2cpp_class_get_property_from_name = (il2cpp_class_get_property_from_name_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_property_from_name")));
    return IL2CppPropertyHelper{this->cls, il2cpp_class_get_property_from_name(this->cls, name)};
  }

  inline IL2CppFieldHelper GetField(const char* name)
  {
    static auto il2cpp_class_get_field_from_name = (il2cpp_class_get_field_from_name_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_field_from_name")));
    return IL2CppFieldHelper{this->cls, il2cpp_class_get_field_from_name(this->cls, name)};
  }

  inline IL2CppStaticFieldHelper GetStaticField(const char* name)
  {
    static auto il2cpp_class_get_field_from_name = (il2cpp_class_get_field_from_name_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_field_from_name")));
    return IL2CppStaticFieldHelper{this->cls, il2cpp_class_get_field_from_name(this->cls, name)};
  }

  inline IL2CppClassHelper GetParent(const char* name)
  {
    static auto  il2cpp_class_get_parent = (il2cpp_class_get_parent_t)(GetProcAddress(
        GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_get_parent")));
    Il2CppClass* pcls                    = this->cls;
    if (pcls) {
      do {
        if (pcls->name[0] == name[0] && !strcmp(name, pcls->name)) {
          return IL2CppClassHelper{pcls};
        }

        pcls = il2cpp_class_get_parent(pcls);
      } while (pcls);
    }
    return IL2CppClassHelper{nullptr};
  }

  inline IL2CppClassHelper GetNestedType(const char* name)
  {
    for (int i = 0; i < this->cls->nested_type_count; ++i) {
      auto type = this->cls->nestedTypes[i];
      if (strcmp(type->name, name) == 0) {
        return type;
      }
    }

    return nullptr;
  }

private:
  Il2CppClass* cls;
};

#define il2cpp_get_class_helper(assembly, namespacez, name)                                                            \
  il2cpp_get_class_helper_impl(xorstr_(assembly), xorstr_(namespacez), xorstr_(name))

inline IL2CppClassHelper il2cpp_get_class_helper_impl(const char* assembly, const char* namespacez, const char* name)
{
  static auto il2cpp_domain_get =
      (il2cpp_domain_get_t)(GetProcAddress(GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_domain_get")));
  static auto il2cpp_domain_assembly_open  = (il2cpp_domain_assembly_open_t)(GetProcAddress(
      GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_domain_assembly_open")));
  static auto il2cpp_domain_get_assemblies = (il2cpp_domain_get_assemblies_t)(GetProcAddress(
      GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_domain_get_assemblies")));
  static auto il2cpp_class_from_name       = (il2cpp_class_from_name_t)(GetProcAddress(
      GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_class_from_name")));
  static auto il2cpp_assembly_get_image    = (il2cpp_assembly_get_image_t)(GetProcAddress(
      GetModuleHandle(xorstr_("GameAssembly.dll")), xorstr_("il2cpp_assembly_get_image")));

  auto domain    = il2cpp_domain_get();
  auto assemblyT = il2cpp_domain_assembly_open(domain, assembly);
  auto image     = il2cpp_assembly_get_image(assemblyT);

  auto cls = il2cpp_class_from_name(image, namespacez, name);

  return IL2CppClassHelper{cls};
}

template <typename T> inline T* il2cpp_get_array_element(Il2CppArray* array, size_t index)
{
  Il2CppArraySize* n = (Il2CppArraySize*)(array);
  return (T*)n->vector[index];
}

template <typename T> class ObjectFinder
{
public:
  static T* Get()
  {
    static auto FindObjectsOfType =
        il2cpp_resolve_icall<Il2CppArray*(void*)>("UnityEngine.Object::FindObjectsOfType(System.Type)");
    static auto type    = T::get_class_helper().GetType();
    auto        objects = FindObjectsOfType(type);
    return il2cpp_get_array_element<T>(objects, objects->max_length - 1);
  }

  static Il2CppArray* GetAll()
  {
    static auto FindObjectsOfType =
        il2cpp_resolve_icall<Il2CppArray*(void*)>("UnityEngine.Object::FindObjectsOfType(System.Type)");
    static auto type    = T::get_class_helper().GetType();
    auto        objects = FindObjectsOfType(type);
    return objects;
  }
};