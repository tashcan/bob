#pragma once
#include "value_view.h"
#include <algorithm>
#include <cmath>
namespace mod_settings
{
class SliderSetting
{
public:
  SliderSetting(ValueDefinition<float> definition, float minimum, float maximum, float step,
                std::function<bool()> enabled)
      : state_(Checked(std::move(definition), minimum, maximum, enabled))
      , minimum_(minimum)
      , maximum_(maximum)
      , step_(step)
      , enabled_(std::move(enabled))
  {
    if (!std::isfinite(step) || step <= 0)
      throw std::invalid_argument("slider step");
  }
  ValueSetting<float>& state()
  { return state_; }
  float minimum() const
  { return minimum_; }
  float maximum() const
  { return maximum_; }
  bool enabled() const
  { return enabled_(); }
  float Snap(float value) const
  {
    return std::clamp(
        static_cast<float>(minimum_ + std::round((static_cast<double>(value) - minimum_) / step_) * step_), minimum_,
        maximum_);
  }

private:
  static ValueDefinition<float> Checked(ValueDefinition<float> definition, float minimum, float maximum,
                                        const std::function<bool()>& enabled)
  {
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || minimum >= maximum || !enabled || !definition.read
        || !definition.write)
      throw std::invalid_argument("slider definition");
    auto read       = std::move(definition.read);
    auto write      = std::move(definition.write);
    definition.read = [read = std::move(read), minimum, maximum] {
      auto value = read();
      if (value.known() && (!std::isfinite(*value.value) || *value.value < minimum || *value.value > maximum))
        return ValueReadResult<float>{};
      return value;
    };
    definition.write = [write = std::move(write), minimum, maximum, enabled](float value, std::uint64_t generation) {
      if (!enabled() || !std::isfinite(value) || value < minimum || value > maximum)
        return ApplyResult::Rejected;
      return write(value, generation);
    };
    return definition;
  }
  ValueSetting<float>   state_;
  float                 minimum_, maximum_, step_;
  std::function<bool()> enabled_;
};
} // namespace mod_settings
