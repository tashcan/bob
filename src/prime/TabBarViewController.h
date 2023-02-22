#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "IList.h"

class BaseListContainer
{
public:
  __declspec(property(get = __get__data)) IList* _data;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "BaseListContainer");
    return class_helper;
  }

public:
  IList* __get__data()
  {
    static auto field = get_class_helper().GetField("_data").offset();
    return *(IList**)((uintptr_t)this + field);
  }
};

class TabBar
{
public:
  __declspec(property(get = __get__defaultOptions)) Il2CppArray* _defaultOptions;
  __declspec(property(get = __get__listContainer)) BaseListContainer* _listContainer;
  __declspec(property(get = __get__data)) IList* _data;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "TabBar");
    return class_helper;
  }

public:
  Il2CppArray* __get__defaultOptions()
  {
    static auto field = get_class_helper().GetField("_defaultOptions").offset();
    return *(Il2CppArray**)((uintptr_t)this + field);
  }

  BaseListContainer* __get__listContainer()
  {
    static auto field = get_class_helper().GetField("_listContainer").offset();
    return *(BaseListContainer**)((uintptr_t)this + field);
  }

  IList* __get__data()
  {
    static auto field = get_class_helper().GetField("_data").offset();
    return *(IList**)((uintptr_t)this + field);
  }
};

class TabBarViewController
{
public:
  __declspec(property(get = __get__tabBar)) TabBar* _tabBar;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "TabBarViewController");
    return class_helper;
  }

public:
  TabBar* __get__tabBar()
  {
    static auto field = get_class_helper().GetField("_tabBar").offset();
    return *(TabBar**)((uintptr_t)this + field);
  }
};