#pragma once

#include "boolean_settings.h"
#include <algorithm>
#include <string_view>
#include <vector>

namespace mod_settings
{
// Presentation only. Settings keep their own identity, state and persistence.
// Own alongside the settings on the UI thread; never retain managed page objects.
class PageCatalog
{
public:
  struct Page {
    std::string                  id, label, parent;
    std::vector<BooleanSetting*> booleans;
  };

  explicit PageCatalog(std::string root_id, std::string root_label)
  {
    if (root_id.empty() || root_label.empty())
      throw std::invalid_argument("settings root identity");
    pages_.push_back({std::move(root_id), std::move(root_label), {}, {}});
  }
  PageCatalog(const PageCatalog&)            = delete;
  PageCatalog& operator=(const PageCatalog&) = delete;

  Registration AddPage(std::string id, std::string label, std::string_view parent)
  {
    CheckThread();
    if (frozen_)
      return Registration::Frozen;
    if (id.empty() || label.empty() || !FindPage(parent))
      return Registration::Invalid;
    if (FindPage(id))
      return Registration::Duplicate;
    // Parents must already exist. Cycles and dangling parent IDs cannot be registered.
    pages_.push_back({std::move(id), std::move(label), std::string(parent), {}});
    return Registration::Added;
  }

  Registration AddBoolean(std::string_view page_id, BooleanSetting& setting)
  {
    CheckThread();
    if (frozen_)
      return Registration::Frozen;
    auto* page = FindPage(page_id);
    if (!page || setting.id().empty() || setting.label().empty())
      return Registration::Invalid;
    // Multiple views of the same setting are allowed on different pages, but
    // one ID cannot silently acquire a different state/persistence owner.
    for (const auto& existing : pages_)
      for (const auto* item : existing.booleans)
        if (item->id() == setting.id() && item != &setting)
          return Registration::Invalid;
    for (const auto* item : page->booleans)
      if (item->id() == setting.id())
        return Registration::Duplicate;
    page->booleans.push_back(&setting);
    return Registration::Added;
  }

  // A fresh plan for each settings context. Stable IDs and parent-first order
  // let the native adapter rebuild without caching addresses from a previous visit.
  // Empty branches disappear. This does not read settings or trigger any writes.
  std::vector<Page> Build()
  {
    CheckThread();
    frozen_     = true;
    auto result = pages_;
    for (std::size_t i = result.size(); i-- > 0;) {
      if (!result[i].booleans.empty())
        continue;
      const bool has_child = std::any_of(result.begin() + i + 1, result.end(),
                                         [&](const Page& page) { return page.parent == result[i].id; });
      if (!has_child)
        result.erase(result.begin() + i);
    }
    return result;
  }

private:
  Page* FindPage(std::string_view id)
  {
    auto found = std::find_if(pages_.begin(), pages_.end(), [&](const Page& page) { return page.id == id; });
    return found == pages_.end() ? nullptr : &*found;
  }
  void CheckThread() const
  {
    if (std::this_thread::get_id() != thread_)
      throw std::logic_error("settings catalog thread mismatch");
  }
  const std::thread::id thread_ = std::this_thread::get_id();
  bool                  frozen_ = false;
  std::vector<Page>     pages_;
};
} // namespace mod_settings
