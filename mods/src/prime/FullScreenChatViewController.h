#pragma once

#include "ChatMessageListLocalViewController.h"
#include "TabBarViewController.h"

class FullScreenChatViewController
{
public:
  __declspec(property(get = __get__messageList)) ChatMessageListLocalViewController* _messageList;
  __declspec(property(get = __get__categoriesTabBarViewController))
      TabBarViewController* _categoriesTabBarViewController;

private:
  friend class ObjectFinder<FullScreenChatViewController>;

public:
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper =
        il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Chat", "FullScreenChatViewController");
    return class_helper;
  }

  ChatMessageListLocalViewController* __get__messageList()
  {
    static auto field = get_class_helper().GetField("_messageList").offset();
    return *(ChatMessageListLocalViewController**)((uintptr_t)this + field);
  }

  TabBarViewController* __get__categoriesTabBarViewController()
  {
    static auto field = get_class_helper().GetField("_categoriesTabBarViewController").offset();
    return *(TabBarViewController**)((uintptr_t)this + field);
  }
};