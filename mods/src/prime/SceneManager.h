#pragma once

#include <il2cpp/il2cpp_helper.h>

struct SceneManager {
public:
  static const Il2CppChar* GetSceneName(void* scene)
  {
    static auto GetSceneName = il2cpp_resolve_icall_typed<Il2CppString*(void*)>(
        "UnityEngine.SceneManagement.Scene::GetNameInternal(System.Int32)");
    auto name = GetSceneName(scene);
    return il2cpp_string_chars(name);
  }

  static void* GetActiveScene()
  {
    static auto GetActiveScene_Injected = il2cpp_resolve_icall_typed<void(void*)>(
        "UnityEngine.SceneManagement.SceneManager::GetActiveScene_Injected(UnityEngine.SceneManagement.Scene&)");
    void* scene = nullptr;
    GetActiveScene_Injected(&scene);
    return scene;
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine.SceneManagement", "SceneManager");
    return class_helper;
  }
};
