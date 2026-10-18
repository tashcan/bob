#include "config.h"
#include "errormsg.h"

#include <il2cpp/il2cpp_helper.h>

#include <prime/Hub.h>

#include <spdlog/spdlog.h>
#include <spud/detour.h>

#include <cmath>
#include <cstring>
#include <unordered_map>

// Station-interior zoom-out.
//
// The interior view is a BlendFrameProvider between a StaticFrameProvider (zoomed-in) and an
// OrbitFrameProvider pinned at a fixed radius (zoomed-out, min==max); zoom slides the blend ratio, so
// the far stop is that orbit endpoint's radius. We push that pinned radius outward to extend zoom-out.
// That same OrbitFrameProvider instance IS the exterior station camera (the interior's fully-zoomed-out
// end is the exterior view), so the hook widens it only while the section is Starbase_Interior and
// restores the native radius otherwise -- leaving the exterior view (and its component-level labels)
// stock. See fix_pan.cc, which drives the same provider for the exterior arrow-key rotate.
//
// graphics.interior_zoom_scale is a multiplier of the native endpoint (1.0 = native), not an absolute
// distance like zoom/haven_zoom: the native stop is a single pinned radius, so a factor is
// resolution-independent and self-heals if the game changes that radius. The config-knob and
// Constraint.FloatData field-resolution pattern follow haven_zoom.cc.
namespace
{
FieldInfo *provider_constraint = nullptr; // AbstractOrbitFrameProvider._constraint -> OrbitConstraint
FieldInfo *constraint_radius   = nullptr; // OrbitConstraint._radius -> Constraint.FloatData
FieldInfo *radius_enabled      = nullptr; // Constraint.FloatData._enabled
FieldInfo *radius_min          = nullptr; // Constraint.FloatData._min
FieldInfo *radius_max          = nullptr; // Constraint.FloatData._max
bool       fields_ok           = false;

struct NativeRange {
  float min, max;
};
// Native (pre-widen) radius range per live interior OrbitFrameProvider. The pointer is ONLY a map key,
// never dereferenced from the stored entry, so a retired scene instance cannot cause a stale-object
// access. Keyed by provider; the interior has a single orbit endpoint so this stays tiny.
std::unordered_map<Il2CppObject *, NativeRange> g_native;
bool                                            g_reported = false;

float InteriorScale()
{
  const auto scale = Config::Get().interior_zoom_scale;
  return (std::isfinite(scale) && scale > 1.0f) ? scale : 1.0f;
}

void OrbitFrameProvider_UpdateConstrains_Hook(auto original, Il2CppObject *provider, void *camera)
{
  if (!fields_ok || provider == nullptr) {
    original(provider, camera);
    return;
  }

  Il2CppObject *constraint = nullptr, *radius = nullptr;
  il2cpp_field_get_value(provider, provider_constraint, &constraint);
  if (constraint != nullptr) {
    il2cpp_field_get_value(constraint, constraint_radius, &radius);
  }
  if (radius == nullptr) {
    original(provider, camera);
    return;
  }

  auto      *section_manager = Hub::get_SectionManager();
  const bool in_interior =
      section_manager != nullptr && section_manager->CurrentSection == SectionID::Starbase_Interior;
  const float scale = InteriorScale();

  if (in_interior && scale > 1.0f) {
    // Capture the native range once (before any widening), keyed by this provider.
    auto it = g_native.find(provider);
    if (it == g_native.end()) {
      NativeRange nr{0.0f, 0.0f};
      il2cpp_field_get_value(radius, radius_min, &nr.min);
      il2cpp_field_get_value(radius, radius_max, &nr.max);
      it = g_native.emplace(provider, nr).first;
      if (!g_reported) {
        bool enabled = false;
        if (radius_enabled) {
          il2cpp_field_get_value(radius, radius_enabled, &enabled);
        }
        spdlog::info("[InteriorZoom] interior orbit endpoint native radius min={} max={} enabled={} (scale={})", nr.min,
                     nr.max, enabled, scale);
        g_reported = true;
      }
    }

    // Widen relative to the captured native range. Idempotent: always derived from the stored native,
    // so reasserting each frame cannot compound. Both ends scale together so a pinned endpoint
    // (min==max) simply relocates farther out; a real range stretches outward.
    float new_min = it->second.min * scale;
    float new_max = it->second.max * scale;
    il2cpp_field_set_value(radius, radius_min, &new_min);
    il2cpp_field_set_value(radius, radius_max, &new_max);
  } else {
    // Not the interior (or feature off): the far endpoint is SHARED with the exterior station view, so
    // restore the native radius on any provider we previously widened. Without this the exterior stays
    // zoomed out and its component-level labels drop out of LOD range.
    auto it = g_native.find(provider);
    if (it != g_native.end()) {
      float native_min = it->second.min, native_max = it->second.max;
      il2cpp_field_set_value(radius, radius_min, &native_min);
      il2cpp_field_set_value(radius, radius_max, &native_max);
    }
  }

  original(provider, camera); // native clamp now runs against the (widened or restored) range
}
} // namespace

void InstallInteriorZoomHooks()
{
  auto orbit = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.CameraController", "OrbitFrameProvider");
  auto abstract_orbit =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.CameraController", "AbstractOrbitFrameProvider");
  auto orbit_constraint =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.CameraController", "OrbitConstraint");
  auto constraint = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.CameraController", "Constraint");

  if (!orbit.isValidHelper()) {
    ErrorMsg::MissingHelper("CameraController", "OrbitFrameProvider");
    return;
  }

  if (abstract_orbit.isValidHelper()) {
    provider_constraint = abstract_orbit.GetField("_constraint").get_info();
  }
  if (orbit_constraint.isValidHelper()) {
    constraint_radius = orbit_constraint.GetField("_radius").get_info();
  }
  if (constraint.isValidHelper()) {
    Il2CppClass *float_data = nullptr;
    void        *iterator   = nullptr;
    while (auto *nested = il2cpp_class_get_nested_types(constraint.get_cls(), &iterator)) {
      if (std::strcmp(il2cpp_class_get_name(nested), "FloatData") == 0) {
        float_data = nested;
        break;
      }
    }
    if (float_data != nullptr) {
      IL2CppClassHelper fd(float_data);
      radius_enabled = fd.GetField("_enabled").get_info();
      radius_min     = fd.GetField("_min").get_info();
      radius_max     = fd.GetField("_max").get_info();
    }
  }

  fields_ok =
      provider_constraint != nullptr && constraint_radius != nullptr && radius_min != nullptr && radius_max != nullptr;
  if (!fields_ok) {
    spdlog::warn("[InteriorZoom] orbit-constraint fields not resolved; interior zoom disabled, native behavior kept");
    return;
  }

  auto ptr_update_constrains = orbit.GetMethod("UpdateConstrains");
  if (ptr_update_constrains == nullptr) {
    ErrorMsg::MissingMethod("OrbitFrameProvider", "UpdateConstrains");
    return;
  }
  if (SPUD_STATIC_DETOUR(ptr_update_constrains, OrbitFrameProvider_UpdateConstrains_Hook)) {
    spdlog::info("[InteriorZoom] installed interior orbit-endpoint hook (scale={})", InteriorScale());
  } else {
    spdlog::warn("[InteriorZoom] OrbitFrameProvider.UpdateConstrains detour failed; native behavior kept");
  }
}
