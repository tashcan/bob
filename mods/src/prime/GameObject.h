#pragma once

struct GameObject {
public:
  __declspec(property(get = __get_activeInHierarchy)) bool activeInHierarchy;
  __declspec(property(get = __get_scene)) void* scene;

  template <typename T> T* GetComponentFastPath2()
  {
      static auto get_component = get_class_helper().GetMethodInfoSpecial("GetComponent", [](auto count, auto params) {
      if (count != 1) {
        return false;
      }
      auto p1 = params[0]->type;
      if (p1 == IL2CPP_TYPE_CLASS) {
        return true;
      }
      return false;
    });

    static auto type = T::get_class_helper().GetType();

    Il2CppException* exception = nullptr;
    void*            params[1] = {type};
    auto             result    = il2cpp_runtime_invoke(get_component, this, params, &exception);

    if (exception) {
      return nullptr;
    } else {
      return (T*)result;
    }
  }

  template <typename T> T* GetComponentFastPath()
  {
    static auto get_component =
        get_class_helper().GetMethodSpecial<T*(GameObject*, void*)>("GetComponent", [](auto count, auto params) {
          if (count != 1) {
            return false;
          }
          auto p1 = params[0].parameter_type->type;
          if (p1 == IL2CPP_TYPE_CLASS) {
            return true;
          }
          return false;
        });

    static auto type = T::get_class_helper().GetType();

    return get_component(this, type);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "GameObject");
    return class_helper;
  }

public:
  bool __get_activeInHierarchy()
  {
    static auto field = get_class_helper().GetProperty("activeInHierarchy");
    return field.Get<bool>(this);
  }
  void* __get_scene()
  {
    static auto field = get_class_helper().GetProperty("scene");
    return *field.Get<void*>(this);
  }
};