#pragma once

#include <il2cpp/il2cpp_helper.h>

#include "MonoSingleton.h"

struct BookmarksManager : MonoSingleton<BookmarksManager> {
  friend struct MonoSingleton<BookmarksManager>;

public:
  void LoadBookmarksFromCloud()
  {
    static auto LoadBookmarksFromCloudMethod = get_class_helper().GetMethod<void(BookmarksManager*)>(xorstr_("LoadBookmarksFromCloud"));
    if (!this->already_loaded) {
      this->already_loaded = true;
      LoadBookmarksFromCloudMethod(this);
    }
  }

  void ViewBookmarks()
  {
    static auto ViewBookmarksMethod = get_class_helper().GetMethod<void(BookmarksManager*)>(xorstr_("ViewBookmarks"));
    ViewBookmarksMethod(this);
  }

private:
  bool already_loaded = false;
  static IL2CppClassHelper& get_class_helper()
  {
    static auto class_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Bookmarks", "BookmarksManager");
    return class_helper;
  }
};