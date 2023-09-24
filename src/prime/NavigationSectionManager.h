#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "NavigationManager.h"
#include "Hub.h"

#include <cstdint>

struct NavigationSectionManager {
public:
  __declspec(property(get = __get_NavigationManager)) NavigationManager* SNavigationManager;

  static void ChangeNavigationSection(SectionID sectionId, int64_t systemId = -1) {
    auto ChangeNavigationSection = get_class_helper().GetMethod<void(SectionID, int64_t)>("ChangeNavigationSection", 2);
    ChangeNavigationSection(sectionId, systemId);
  }

  static NavigationSectionManager* Instance()
  {
    static auto p = get_class_helper().GetProperty("Instance");
    return p.GetRaw<NavigationSectionManager>(nullptr);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Navigation", "NavigationSectionManager");
    return class_helper;
  }

public:
  NavigationManager* __get_NavigationManager()
  {
    static auto field = get_class_helper().GetProperty("NavigationManager");
    return field.GetRaw<NavigationManager>(this);
  }
};