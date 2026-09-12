#pragma once

#include "boolean_settings.h"
#include "choice_setting.h"
#include "slider_setting.h"
#include <algorithm>
#include <ranges>
#include <string_view>
#include <variant>
#include <vector>

namespace mod_settings
{
// Presentation only. Settings keep their identity, live state and persistence.
class PageCatalog
{
public:
  struct Heading {
    std::string id, label;
  };
  using Item = std::variant<Heading, BooleanSetting*, ChoiceSetting*, SliderSetting*>;
  struct Page {
    std::string                id, label, parent;
    std::vector<Item>          items; // Registration order is visual order, including headings.
    template <typename T> auto Controls() const
    {
      return items | std::views::filter([](const Item& item) { return std::holds_alternative<T*>(item); })
             | std::views::transform([](const Item& item) { return std::get<T*>(item); });
    }
    std::size_t ControlRows() const
    {
      std::size_t count = 0;
      for (const auto& item : items)
        std::visit(
            [&](const auto& value) {
              using T = std::decay_t<decltype(value)>;
              if constexpr (std::is_same_v<T, ChoiceSetting*>)
                count += value->labels().size();
              else if constexpr (!std::is_same_v<T, Heading>)
                ++count;
            },
            item);
      return count;
    }
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
    // Parents already exist: no orphan or cyclic registrations.
    pages_.push_back({std::move(id), std::move(label), std::string(parent), {}});
    return Registration::Added;
  }
  Registration AddBoolean(std::string_view page, BooleanSetting& setting)
  { return AddControl(page, setting); }
  Registration AddChoice(std::string_view page, ChoiceSetting& setting)
  { return AddControl(page, setting); }
  Registration AddSlider(std::string_view page, SliderSetting& setting)
  { return AddControl(page, setting); }
  Registration AddHeading(std::string_view page_id, std::string id, std::string label)
  {
    CheckThread();
    if (frozen_)
      return Registration::Frozen;
    auto* page = FindPage(page_id);
    if (!page || id.empty() || label.empty())
      return Registration::Invalid;
    for (const auto& existing : pages_)
      for (const auto& item : existing.items)
        if (Id(item) == id)
          return Registration::Duplicate;
    page->items.emplace_back(Heading{std::move(id), std::move(label)});
    return Registration::Added;
  }

  // Freeze one immutable plan. Each native settings context gets fresh objects.
  // Heading-only pages are empty; building never reads or writes a setting.
  std::vector<Page> Build()
  {
    CheckThread();
    frozen_     = true;
    auto result = pages_;
    for (std::size_t i = result.size(); i-- > 0;) {
      if (result[i].ControlRows())
        continue;
      const bool has_child = std::any_of(result.begin() + i + 1, result.end(),
                                         [&](const Page& page) { return page.parent == result[i].id; });
      if (!has_child)
        result.erase(result.begin() + i);
    }
    return result;
  }

private:
  static const std::string& Id(const Item& item)
  {
    return std::visit(
        [](const auto& value) -> const std::string& {
          using T = std::decay_t<decltype(value)>;
          if constexpr (std::is_same_v<T, Heading>)
            return value.id;
          else if constexpr (std::is_same_v<T, BooleanSetting*>)
            return value->id();
          else
            return value->state().id();
        },
        item);
  }
  template <typename T> Registration AddControl(std::string_view page_id, T& setting)
  {
    CheckThread();
    if (frozen_)
      return Registration::Frozen;
    auto* page = FindPage(page_id);
    if (!page)
      return Registration::Invalid;
    const Item candidate = &setting;
    for (const auto& existing : pages_)
      for (const auto& item : existing.items) {
        if (Id(item) != Id(candidate))
          continue;
        auto* owner = std::get_if<T*>(&item);
        if (!owner || *owner != &setting)
          return Registration::Invalid;
        if (existing.id == page_id)
          return Registration::Duplicate;
      }
    page->items.push_back(candidate);
    return Registration::Added;
  }
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
