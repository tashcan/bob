#pragma once

#include "EntityGroup.h"

#include <il2cpp/il2cpp_helper.h>

struct RepeatedField_EntityGroup {
public:
  __declspec(property(get = __get_Count)) int Count;

  EntityGroup* get_Item(int index)
  {
    static auto helper   = IL2CppClassHelper{((Il2CppObject*)(this))->klass};
    static auto get_Item = helper.GetMethod<EntityGroup*(void*, int)>("get_Item");
    return get_Item(this, index);
  }

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Google.Protobuf", "Google.Protobuf.Collections", "RepeatedField`1");
    return class_helper;
  }

public:
  int __get_Count()
  {
    static auto helper = IL2CppClassHelper{((Il2CppObject*)(this))->klass};
    static auto prop   = helper.GetField("count");
    return *(int*)((ptrdiff_t)this + prop.offset());
  };
};

struct ServiceResponse {
  __declspec(property(get = __get_EntityGroups)) RepeatedField_EntityGroup* EntityGroups;

private:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Models", "ServiceResponse");
    return class_helper;
  }

public:
  RepeatedField_EntityGroup* __get_EntityGroups()
  {
    static auto prop = get_class_helper().GetProperty("EntityGroups");
    return prop.GetRaw<RepeatedField_EntityGroup>(this);
  }
};