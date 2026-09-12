#pragma once
#include "boolean_view.h"
#include "choice_setting.h"

namespace mod_settings
{
// Both native widgets expose a bool click. Selection rows retain the complete
// integer snapshot so changing between two other choices still makes a row stale.
class NativeViewState
{
public:
  explicit NativeViewState(BooleanSetting& setting)
      : boolean_(std::in_place, setting)
  {
  }
  NativeViewState(ChoiceSetting& setting, int index)
      : choice_(std::in_place, setting.state())
      , group_(&setting)
      , index_(index)
  {
  }
  void Bind()
  {
    if (boolean_)
      boolean_->Bind();
    else
      choice_->Bind();
  }
  void Unbind()
  {
    if (boolean_)
      boolean_->Unbind();
    else
      choice_->Unbind();
  }
  void Invalidate()
  {
    if (boolean_)
      boolean_->Invalidate();
    else
      choice_->Invalidate();
  }
  bool failed() const
  { return boolean_ ? boolean_->failed() : choice_->failed(); }
  std::optional<bool> value() const
  {
    if (boolean_)
      return boolean_->value();
    auto selected = choice_->value();
    return selected ? std::optional<bool>(*selected == index_) : std::nullopt;
  }
  int selected() const
  { return choice_ ? choice_->value().value_or(-1) : -1; }
  Outcome Request(bool desired)
  {
    if (boolean_)
      return boolean_->Request(desired).outcome;
    return desired ? choice_->Request(index_).outcome : Outcome::Unchanged;
  }
  std::string id() const
  { return boolean_ ? boolean_->setting().id() : group_->item_id(index_); }
  std::string label() const
  { return boolean_ ? boolean_->setting().label() : group_->labels().at(index_); }
  class RenderScope
  {
  public:
    explicit RenderScope(NativeViewState& state)
    {
      if (state.boolean_)
        boolean_.emplace(state.boolean_->setting());
      else
        choice_.emplace(state.choice_->setting());
    }

  private:
    std::optional<BooleanSetting::RenderScope>    boolean_;
    std::optional<ValueSetting<int>::RenderScope> choice_;
  };

private:
  std::optional<BooleanView>    boolean_;
  std::optional<ValueView<int>> choice_;
  ChoiceSetting*                group_ = nullptr;
  int                           index_ = 0;
};
} // namespace mod_settings
