#include "fc_confirmation_reset.h"
#include "settings/boolean_view.h"
#include "settings/forbidden_tech.h"
#include "settings/mod_pages.h"
#include "settings/native_view_state.h"

// Native extents are checked against Windows unwind records. Other platforms
// omit the native UI until equivalent hook evidence is available.
#if defined(_WIN32) && defined(_M_X64)
#include "prime/Color.h"
#include "prime/Vector3.h"
#include "settings/native_boolean_callback.h"
#include <Windows.h>
#include <array>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <il2cpp/il2cpp_helper.h>
#include <spdlog/spdlog.h>
#include <spud/detour.h>
#include <stdexcept>
#include <thread>

namespace
{
using namespace mod_settings;
constexpr const char* CategoryKey = "game_settings_category_7";

bool                           active     = false;
bool                           installing = false;
bool                           warned     = false;
std::thread::id                uiThread;
NativeCallback<bool>           getter;
NativeCallback<void, bool>     setter;
NativeCallback<int>            query;
NativeCallback<int>            selectionGetter;
NativeCallback<void, int>      selectionSetter;
bool                           selectionActive = false;
NativeCallback<float>          sliderGetter;
NativeCallback<void, float>    sliderSetter;
bool                           sliderActive = false;
std::vector<PageCatalog::Page> pages;
bool                           pagesActive = false;
bool                           HasLabel(Il2CppObject* row, const char* id);
BooleanSetting*                SettingFor(Il2CppObject* context)
{
  auto& fc = FleetCommanderConfirmationSetting();
  if (HasLabel(context, fc.id().c_str()))
    return &fc;
  auto& ft = ForbiddenTechConfirmationSetting();
  if (HasLabel(context, ft.id().c_str()))
    return &ft;
  for (const auto& page : pages)
    for (auto* setting : page.Controls<BooleanSetting>())
      if (HasLabel(context, setting->id().c_str()))
        return setting;
  return nullptr;
}

void Warn(const char* reason = "native control unavailable")
{
  if (!warned) {
    warned = true;
    spdlog::warn("[ModSettings] {}", reason);
  }
}

struct Root {
  Il2CppGCHandle handle = nullptr;
  explicit Root(Il2CppObject* object, bool weak = false)
  {
    if (object)
      handle = weak ? il2cpp_gchandle_new_weakref(object, false) : il2cpp_gchandle_new(object, false);
    if (object && !handle)
      throw std::runtime_error("settings root");
  }
  ~Root()
  {
    if (handle)
      il2cpp_gchandle_free(handle);
  }
  Root(const Root&) = delete;
  Il2CppObject* get() const
  { return handle ? il2cpp_gchandle_get_target(handle) : nullptr; }
};

bool Type(const Il2CppType* type, int expected)
{ return type && !type->byref && type->type == expected; }
bool Instance(const MethodInfo* method, int count, int result)
{
  return method && method->methodPointer && method->invoker_method && !(method->flags & METHOD_ATTRIBUTE_STATIC)
         && method->parameters_count == count && Type(method->return_type, result)
         && !method->has_full_generic_sharing_signature;
}
bool Reference(const Il2CppType* type)
{
  return Type(type, IL2CPP_TYPE_CLASS) || Type(type, IL2CPP_TYPE_GENERICINST) || Type(type, IL2CPP_TYPE_OBJECT)
         || Type(type, IL2CPP_TYPE_STRING);
}
FieldInfo* Field(Il2CppClass* cls, const char* name)
{
  auto* field = cls ? il2cpp_class_get_field_from_name(cls, name) : nullptr;
  if (!field || !Reference(field->type) || (field->type->attrs & FIELD_ATTRIBUTE_STATIC))
    throw std::runtime_error("settings reference field");
  return field;
}
Il2CppObject* ReadField(Il2CppObject* object, FieldInfo* field)
{
  Il2CppObject* value = nullptr;
  if (object)
    il2cpp_field_get_value(object, field, &value);
  return value;
}
Il2CppObject* Invoke(const MethodInfo* method, Il2CppObject* object, void** args = nullptr)
{
  if (!method || !object)
    throw std::runtime_error("settings invocation");
  Il2CppException* error  = nullptr;
  auto*            result = il2cpp_runtime_invoke(method, object, args, &error);
  if (error)
    throw std::runtime_error("settings managed exception");
  return result;
}
// Bounded discovery helpers used only while opening a page or binding a row.
Il2CppObject* Call(Il2CppObject* object, const char* name, int count = 0, void** args = nullptr)
{ return Invoke(object ? il2cpp_class_get_method_from_name(object->klass, name, count) : nullptr, object, args); }
bool Boolean(Il2CppObject* boxed)
{
  if (!boxed || !Type(il2cpp_class_get_type(boxed->klass), IL2CPP_TYPE_BOOLEAN))
    throw std::runtime_error("settings boolean result");
  return *static_cast<bool*>(il2cpp_object_unbox(boxed));
}
bool Equals(Il2CppObject* value, const char* ascii)
{
  if (!value || !Type(il2cpp_class_get_type(value->klass), IL2CPP_TYPE_STRING))
    return false;
  auto*      text   = reinterpret_cast<Il2CppString*>(value);
  const auto length = std::strlen(ascii);
  if (il2cpp_string_length(text) != length)
    return false;
  auto* chars = il2cpp_string_chars(text);
  for (std::size_t i = 0; i < length; ++i)
    if (chars[i] != static_cast<unsigned char>(ascii[i]))
      return false;
  return true;
}

struct Metadata {
  bool selection, slider;
  explicit Metadata(bool selection = false, bool slider = false)
      : selection(selection)
      , slider(slider)
  {
  }
  IL2CppClassHelper director =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "SettingsSectionDirector");
  IL2CppClassHelper widget  = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings",
                                                      slider      ? "SliderOptionWidget"
                                                      : selection ? "SelectionItemOptionWidget"
                                                                  : "ToggleOptionWidget");
  IL2CppClassHelper context = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "SettingsContext");
  IL2CppClassHelper row     = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings",
                                                      slider      ? "SliderOptionContext"
                                                      : selection ? "SelectionItemOptionContext"
                                                                  : "ToggleOptionContext");
  IL2CppClassHelper prefs =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.PersistentPrefs", "PersistentPrefsManager");
  const MethodInfo* addGeneral  = director.GetMethodInfo("AddGeneralSettings", 1);
  const MethodInfo* addToggle   = context.GetMethodInfo("AddToggle", 4);
  const MethodInfo* refresh     = widget.GetMethodInfo("SetWidgetData", 0);
  const MethodInfo* changed     = widget.GetMethodInfo(slider ? "OnSliderValueChanged" : "OnToggleValueChanged", 1);
  const MethodInfo* release     = widget.GetMethodInfo("OnAboutToReleaseContext", 0);
  const MethodInfo* reload      = prefs.GetMethodInfo("RegisterEvents", 0);
  const MethodInfo* session     = prefs.GetMethodInfo("GameSessionStartedEventHandler", 0);
  const MethodInfo* load        = prefs.GetMethodInfo("LoadPersistentPrefsFromCloud", 0);
  const MethodInfo* getContext  = widget.GetMethodInfo("get_Context", 0);
  const MethodInfo* querySetter = row.GetMethodInfo("set_QueryOptionState", 1);
  FieldInfo*        queryField  = Field(row.get_cls(), "<QueryOptionState>k__BackingField");
  FieldInfo*        labelField  = Field(widget.get_cls(), "_label");
  FieldInfo*        toggleField = Field(widget.get_cls(), slider ? "_slider" : "_toggle");
  FieldInfo*        stateField  = Field(widget.get_cls(), slider ? "_valueLabel" : "_toggleStateAnimator");
};
Metadata& Meta()
{
  static Metadata metadata;
  return metadata;
}
Metadata& SelectionMeta()
{
  static Metadata metadata(true);
  return metadata;
}
Metadata& SliderMeta()
{
  static Metadata metadata(false, true);
  return metadata;
}
Metadata& WidgetMeta(Il2CppObject* widget)
{
  if (sliderActive && widget && widget->klass == SliderMeta().widget.get_cls())
    return SliderMeta();
  return selectionActive && widget && widget->klass == SelectionMeta().widget.get_cls() ? SelectionMeta() : Meta();
}
std::pair<ChoiceSetting*, int> ChoiceFor(Il2CppObject* context)
{
  for (const auto& page : pages)
    for (auto* choice : page.Controls<ChoiceSetting>())
      for (int i = 0; i < static_cast<int>(choice->labels().size()); ++i)
        if (HasLabel(context, choice->item_id(i).c_str()))
          return {choice, i};
  return {nullptr, 0};
}

SliderSetting* SliderFor(Il2CppObject* context)
{
  for (const auto& page : pages)
    for (auto* setting : page.Controls<SliderSetting>())
      if (HasLabel(context, setting->state().id().c_str()))
        return setting;
  return nullptr;
}

Il2CppObject* Target(Il2CppGCHandle handle)
{ return handle ? il2cpp_gchandle_get_target(handle) : nullptr; }
void Free(Il2CppGCHandle& handle)
{
  if (handle)
    il2cpp_gchandle_free(handle);
  handle = nullptr;
}

// Cosmetic changes belong to the bound row. Restore before native refresh or
// pooling; never change a shared material or search child controls for an image.
struct RowTint {
  Il2CppGCHandle image = nullptr;
  color          before{};
};
void RestoreTint(RowTint& tint)
{
  try {
    if (auto* image = Target(tint.image)) {
      void* args[] = {&tint.before};
      Call(image, "set_color", 1, args);
    }
  } catch (...) {
    Warn("settings row tint restoration unavailable");
  }
  Free(tint.image);
}
// The build261 settings prefabs put backgrounds on a direct BG child (category
// rows use Background). Do not search arbitrary descendants such as a checkbox.
Il2CppObject* RowImage(Il2CppObject* widget, const char* child = "BG")
{
  static auto images  = il2cpp_get_class_helper("UnityEngine.UI", "UnityEngine.UI", "Image");
  static auto objects = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "GameObject");
  static auto get     = objects.GetMethodInfoSpecial("GetComponent", [](auto count, auto params) {
    return count == 1 && Type(params[0], IL2CPP_TYPE_CLASS)
           && std::strcmp(il2cpp_class_get_name(il2cpp_class_from_type(params[0])), "Type") == 0;
  });
  Root        transform(Call(widget, "get_transform"));
  Root        name(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(child)));
  void*       findArgs[] = {name.get()};
  Root        background(Call(transform.get(), "Find", 1, findArgs));
  if (!background.get())
    return nullptr;
  Root  object(Call(background.get(), "get_gameObject"));
  void* args[] = {images.GetType()};
  return Invoke(get, object.get(), args);
}
void TintImage(RowTint& tint, Il2CppObject* image, color value, bool multiply = true)
{
  RestoreTint(tint);
  if (!image)
    return;
  static auto colors = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "Color");
  Root        boxed(Call(image, "get_color"));
  const auto* set = il2cpp_class_get_method_from_name(image->klass, "set_color", 1);
  if (!boxed.get() || boxed.get()->klass != colors.get_cls() || !Instance(set, 1, IL2CPP_TYPE_VOID)
      || set->parameters[0]->byref || il2cpp_class_from_type(set->parameters[0]) != colors.get_cls())
    return;
  tint.before = *static_cast<color*>(il2cpp_object_unbox(boxed.get()));
  tint.image  = il2cpp_gchandle_new_weakref(image, false);
  if (!tint.image)
    return;
  if (multiply) {
    value.r *= tint.before.r;
    value.g *= tint.before.g;
    value.b *= tint.before.b;
  }
  value.a      = tint.before.a;
  void* args[] = {&value};
  Invoke(set, image, args);
}
void TintRow(RowTint& tint, Il2CppObject* widget, color multiplier)
{
  try {
    Root image(RowImage(widget));
    TintImage(tint, image.get(), multiplier);
  } catch (...) {
    RestoreTint(tint);
    Warn("settings row tint unavailable");
  }
}
// Reuse sprites already rendered by these native settings widgets. Weak handles
// do not keep a scene/bundle alive. No asset loading, animation sampling or polling.
Il2CppGCHandle normalChoiceSprite = nullptr, pressedChoiceSprite = nullptr;
void           RememberChoiceSprite(Il2CppObject* background)
{
  if (!background || (Target(normalChoiceSprite) && Target(pressedChoiceSprite)))
    return;
  Root sprite(Call(background, "get_sprite"));
  if (!sprite.get())
    return;
  Root  name(Call(sprite.get(), "get_name"));
  auto& handle = Equals(name.get(), "SelectedBG_raw") ? pressedChoiceSprite : normalChoiceSprite;
  if (!Target(handle)) {
    Free(handle);
    handle = il2cpp_gchandle_new_weakref(sprite.get(), false);
  }
}

// Stable weak records, sized once from registered controls before pages can bind: no page, context, delegate, or
// account is retained by UI bookkeeping. Records are released on native unbind and reclaimed on next bind if Unity
// destroys a widget without sending that notification.
struct View {
  RowTint                        tint, checkTint;
  Il2CppGCHandle                 choiceBackground = nullptr, choiceOverrideBefore = nullptr, choiceCheck = nullptr;
  bool                           choiceStyled = false, pressed = false;
  Il2CppGCHandle                 widget = nullptr, context = nullptr, label = nullptr;
  Il2CppGCHandle                 selectionControl = nullptr;
  std::array<Il2CppGCHandle, 2>  indicators{};
  std::array<bool, 2>            activeBefore{};
  bool                           overridden          = false;
  bool                           hidden              = false;
  bool                           disabled            = false;
  bool                           interactableBefore  = false;
  bool                           rendering           = false;
  bool                           binding             = false;
  bool                           requesting          = false;
  bool                           clearing            = false;
  bool                           preserveNextRefresh = false;
  std::optional<NativeViewState> state;
};
std::deque<View>& Views()
{
  static std::deque<View> views(8);
  return views;
}
View* renderingView = nullptr;
View* bindingView   = nullptr;
void  SetActive(Il2CppObject* object, bool value)
{
  void* args[] = {&value};
  Call(object, "SetActive", 1, args);
}
void Restore(View& view)
{
  RestoreTint(view.tint);
  if (view.disabled) {
    if (auto* control = Target(view.selectionControl)) {
      void* args[] = {&view.interactableBefore};
      Call(control, "set_interactable", 1, args);
    }
    view.disabled = false;
  }
  if (view.overridden) {
    if (auto* label = Target(view.label))
      Call(label, "ClearTextOverride");
    view.overridden = false;
  }
  if (view.hidden) {
    for (std::size_t i = 0; i < view.indicators.size(); ++i)
      if (auto* object = Target(view.indicators[i]))
        SetActive(object, view.activeBefore[i]);
    view.hidden = false;
  }
}
void ClearChoiceStyle(View& view)
{
  RestoreTint(view.checkTint);
  try {
    if (view.choiceStyled) {
      if (auto* background = Target(view.choiceBackground)) {
        Root  before(Target(view.choiceOverrideBefore));
        void* args[] = {before.get()};
        Call(background, "set_overrideSprite", 1, args);
      }
    }
  } catch (...) {
    Warn("settings selection style restoration unavailable");
  }
  Free(view.choiceBackground);
  Free(view.choiceOverrideBefore);
  Free(view.choiceCheck);
  view.choiceStyled = view.pressed = false;
}
void Clear(View& view)
{
  if (view.clearing)
    return;
  struct Scope {
    View& view;
    explicit Scope(View& view)
        : view(view)
    { view.clearing = true; }
    ~Scope()
    { view.clearing = false; }
  } scope(view);
  try {
    Restore(view);
  } catch (...) {
    Warn();
  }
  // Restoring interactability synchronously calls DoStateTransition. Suppress
  // presentation reentry until every override is restored and the slot detached.
  ClearChoiceStyle(view);
  Free(view.widget);
  Free(view.context);
  Free(view.label);
  Free(view.selectionControl);
  for (auto& handle : view.indicators)
    Free(handle);
  if (view.state)
    view.state->Unbind();
  // Keep this object alive through reentrant release during read/write.
  // Track replaces it only after its rendering/request scope has returned.
  view.overridden = view.hidden = view.disabled = view.preserveNextRefresh = false;
}
View* Find(Il2CppObject* widget)
{
  for (auto& view : Views())
    if (Target(view.widget) == widget)
      return &view;
  return nullptr;
}
bool Owned(Il2CppObject* context)
{
  if (!context)
    return false;
  const bool selection = selectionActive && context->klass == SelectionMeta().row.get_cls();
  const bool slider    = sliderActive && context->klass == SliderMeta().row.get_cls();
  if (!selection && !slider && context->klass != Meta().row.get_cls())
    return false;
  auto& meta     = slider ? SliderMeta() : selection ? SelectionMeta() : Meta();
  auto* callback = reinterpret_cast<Il2CppDelegate*>(ReadField(context, meta.queryField));
  return callback && callback->method == query.method() && callback->method_ptr == query.method()->methodPointer
         && (slider      ? SliderFor(context) != nullptr
             : selection ? ChoiceFor(context).first != nullptr
                         : SettingFor(context) != nullptr);
}
bool ChildOf(Il2CppObject* transform, Il2CppObject* parent)
{
  void* args[] = {parent};
  return Boolean(Call(transform, "IsChildOf", 1, args));
}
View& Track(Il2CppObject* widget, Il2CppObject* context)
{
  for (auto& view : Views()) {
    if (Target(view.widget) || view.rendering || view.binding || view.requesting || view.clearing)
      continue;
    Clear(view);
    auto&                        metadata = WidgetMeta(widget);
    Root                         label(ReadField(widget, metadata.labelField));
    Root                         widgetTransform(Call(widget, "get_transform"));
    Root                         labelTransform(Call(label.get(), "get_transform"));
    std::array<Il2CppObject*, 2> indicators{ReadField(widget, metadata.toggleField),
                                            ReadField(widget, metadata.stateField)};
    Root                         first(Call(indicators[0], "get_gameObject"));
    Root                         second(Call(indicators[1], "get_gameObject"));
    indicators = {first.get(), second.get()};
    Root selectionControl(metadata.selection || metadata.slider ? ReadField(widget, metadata.toggleField) : nullptr);
    if (metadata.selection) {
      // Selection prefabs can put the toggle/animator on the entire row. Never
      // hide those containers: unknown selection renders -1 and disables input.
      Root transform(Call(selectionControl.get(), "get_transform"));
      if (!ChildOf(transform.get(), widgetTransform.get()) || !ChildOf(labelTransform.get(), widgetTransform.get()))
        throw std::runtime_error("selection control hierarchy");
      (void)Boolean(Call(selectionControl.get(), "get_interactable"));
    } else {
      // Boolean rows suppress unknown ON/OFF by hiding only detached indicators.
      for (auto* indicator : indicators) {
        Root transform(Call(indicator, "get_transform"));
        if (transform.get() == widgetTransform.get() || !ChildOf(transform.get(), widgetTransform.get())
            || ChildOf(labelTransform.get(), transform.get()))
          throw std::runtime_error("settings indicator hierarchy");
      }
    }
    try {
      auto weak = [](Il2CppObject* object) {
        auto handle = il2cpp_gchandle_new_weakref(object, false);
        if (!handle)
          throw std::runtime_error("settings weak root");
        return handle;
      };
      view.widget  = weak(widget);
      view.context = weak(context);
      if (metadata.slider) {
        auto* setting = SliderFor(context);
        if (!setting)
          throw std::runtime_error("slider owner missing");
        view.state.emplace(*setting);
      } else if (metadata.selection) {
        auto [setting, index] = ChoiceFor(context);
        if (!setting)
          throw std::runtime_error("selection owner missing");
        view.state.emplace(*setting, index);
      } else {
        auto* setting = SettingFor(context);
        if (!setting)
          throw std::runtime_error("settings owner missing");
        view.state.emplace(*setting);
      }
      view.label = weak(label.get());
      if (metadata.selection || metadata.slider)
        view.selectionControl = weak(selectionControl.get());
      if (!metadata.selection)
        for (std::size_t i = 0; i < indicators.size(); ++i)
          view.indicators[i] = weak(indicators[i]);
    } catch (...) {
      Clear(view);
      throw;
    }
    return view;
  }
  throw std::runtime_error("settings view capacity");
}

bool GetEnabled(Il2CppObject*, const MethodInfo*)
{
  // Native bool signatures cannot express unknown. Only the owned render scope
  // consumes this placeholder; its indicators are suppressed when value is empty.
  return renderingView && renderingView->state ? renderingView->state->value().value_or(false) : false;
}
int GetSelected(Il2CppObject*, const MethodInfo*)
{ return renderingView && renderingView->state ? renderingView->state->selected() : -1; }
void  SetSelected(Il2CppObject*, int, const MethodInfo*) {}
float GetNumber(Il2CppObject*, const MethodInfo*)
{ return renderingView && renderingView->state ? renderingView->state->number() : 0.0f; }
void SetNumber(Il2CppObject*, float, const MethodInfo*) {}
void SetEnabled(Il2CppObject*, bool, const MethodInfo*)
{
  // Deliberately inert. Only OnToggleValueChanged with a live view snapshot can
  // authorize a write; rendering and reflection cannot mutate game preferences.
}
int QueryState(Il2CppObject*, const MethodInfo*)
{ return 0; }

Il2CppObject* MakeDelegate(Il2CppClass* cls, Il2CppObject* director, const MethodInfo* method)
{
  const auto* ctor = cls ? il2cpp_class_get_method_from_name(cls, ".ctor", 2) : nullptr;
  if (!Instance(ctor, 2, IL2CPP_TYPE_VOID) || !Reference(ctor->parameters[0])
      || !Type(ctor->parameters[1], IL2CPP_TYPE_I) || !method)
    throw std::runtime_error("settings delegate constructor");
  const auto* invoke = il2cpp_class_get_method_from_name(cls, "Invoke", method->parameters_count);
  auto*       parent = il2cpp_class_get_parent(cls);
  if (!parent || std::strcmp(il2cpp_class_get_name(parent), "MulticastDelegate") != 0
      || std::strcmp(il2cpp_class_get_namespace(parent), "System") != 0 || !invoke || invoke->return_type->byref
      || (invoke->flags & METHOD_ATTRIBUTE_STATIC)
      || il2cpp_class_from_type(invoke->return_type) != il2cpp_class_from_type(method->return_type)
      || !il2cpp_class_is_assignable_from(method->klass, director->klass))
    throw std::runtime_error("settings delegate signature");
  for (int i = 0; i < method->parameters_count; ++i)
    if (invoke->parameters[i]->byref
        || il2cpp_class_from_type(invoke->parameters[i]) != il2cpp_class_from_type(method->parameters[i]))
      throw std::runtime_error("settings delegate parameter");
  Root  object(il2cpp_object_new(cls));
  void* args[] = {director, &method};
  Invoke(ctor, object.get(), args);
  auto* delegate = reinterpret_cast<Il2CppDelegate*>(object.get());
  if (delegate->target != director || delegate->invoke_impl_this != director || delegate->method != method)
    throw std::runtime_error("settings closed delegate");
  delegate->method_ptr  = method->methodPointer;
  delegate->invoke_impl = method->methodPointer;
  return object.get();
}

int Count(Il2CppObject* list)
{
  Root value(Call(list, "get_Count"));
  if (!value.get() || !Type(il2cpp_class_get_type(value.get()->klass), IL2CPP_TYPE_I4))
    throw std::runtime_error("settings child count");
  const int count = *static_cast<int*>(il2cpp_object_unbox(value.get()));
  if (count < 0 || count > 128)
    throw std::runtime_error("settings child bound");
  return count;
}
Il2CppObject* Item(Il2CppObject* list, int index)
{
  void* args[] = {&index};
  return Call(list, "get_Item", 1, args);
}
bool HasLabel(Il2CppObject* row, const char* id)
{
  Root label(Call(row, "get_LabelContext"));
  return label.get() && Equals(Call(label.get(), "get_Identifier"), id);
}
Il2CppObject* Category(Il2CppObject* container, int depth, int& remaining)
{
  if (!container || depth > 3 || --remaining < 0)
    return nullptr;
  if (HasLabel(container, CategoryKey))
    return container;
  if (!il2cpp_class_get_method_from_name(container->klass, "get_Children", 0))
    return nullptr;
  Root children(Call(container, "get_Children"));
  for (int i = 0, count = Count(children.get()); i < count && remaining > 0; ++i)
    if (auto* found = Category(Item(children.get(), i), depth + 1, remaining))
      return found;
  return nullptr;
}
void AddBooleanRow(Il2CppObject* director, Il2CppObject* context, Il2CppObject* category, BooleanSetting& setting)
{
  if (setting.Observe().state.availability == Availability::Unsupported)
    return;
  Root      children(Call(category, "get_Children"));
  const int before = Count(children.get());
  for (int i = 0; i < before; ++i)
    if (HasLabel(Item(children.get(), i), setting.id().c_str()))
      return;
  if (before == 128)
    throw std::runtime_error("settings category full");
  auto& m = Meta();
  Root  get(MakeDelegate(il2cpp_class_from_type(m.addToggle->parameters[2]), director, getter.method()));
  Root  set(MakeDelegate(il2cpp_class_from_type(m.addToggle->parameters[3]), director, setter.method()));
  Root  state(MakeDelegate(il2cpp_class_from_type(m.querySetter->parameters[0]), director, query.method()));
  Root  label(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(setting.id().c_str())));
  void* args[] = {category, label.get(), get.get(), set.get()};
  Invoke(m.addToggle, context, args);
  if (Count(children.get()) != before + 1)
    throw std::runtime_error("settings row insertion");
  Root row(Item(children.get(), before));
  try {
    if (row.get()->klass != m.row.get_cls() || !HasLabel(row.get(), setting.id().c_str()))
      throw std::runtime_error("settings row identity");
    void* stateArgs[] = {state.get()};
    Invoke(m.querySetter, row.get(), stateArgs);
  } catch (...) {
    void* removeArgs[] = {row.get()};
    Call(category, "RemoveChild", 1, removeArgs);
    throw;
  }
}

void AddRow(Il2CppObject* director, Il2CppObject* context)
{
  Root root(Call(context, "get_RootOption"));
  int  remaining = 128;
  Root category(Category(root.get(), 0, remaining));
  if (!category.get())
    throw std::runtime_error("settings confirmation category");
  AddBooleanRow(director, context, category.get(), FleetCommanderConfirmationSetting());
  AddBooleanRow(director, context, category.get(), ForbiddenTechConfirmationSetting());
}
void StyleChoice(View& view)
{
  auto* widget = Target(view.widget);
  if (!widget || !view.state || !WidgetMeta(widget).selection)
    return;
  if (!view.choiceStyled) {
    Root background(RowImage(widget));
    Root check(RowImage(widget, "Arrow"));
    if (!background.get() || !check.get())
      return;
    RememberChoiceSprite(background.get());
    if (!Target(normalChoiceSprite))
      return;
    Root before(ReadField(background.get(), Field(background.get()->klass, "m_OverrideSprite")));
    view.choiceBackground     = il2cpp_gchandle_new_weakref(background.get(), false);
    view.choiceCheck          = il2cpp_gchandle_new_weakref(check.get(), false);
    view.choiceOverrideBefore = before.get() ? il2cpp_gchandle_new_weakref(before.get(), false) : nullptr;
    if (!view.choiceBackground || !view.choiceCheck || (before.get() && !view.choiceOverrideBefore))
      throw std::runtime_error("settings selection style roots");
    view.choiceStyled = true;
  }
  Root background(Target(view.choiceBackground));
  // Selection animation still updates sprite, geometry and the actual checkmark.
  // Image.overrideSprite changes only the drawn background, so the native isOn
  // animation can keep running. Pointer events provide transient pressed feedback.
  RememberChoiceSprite(background.get());
  Root sprite(Target(view.pressed ? pressedChoiceSprite : normalChoiceSprite));
  if (!sprite.get())
    return;
  void* args[] = {sprite.get()};
  Call(background.get(), "set_overrideSprite", 1, args);
  Root check(Target(view.choiceCheck));
  TintImage(view.checkTint, check.get(), view.pressed ? color{0.22f, 0.22f, 0.22f, 1} : color{0.88f, 0.95f, 0.97f, 1},
            false);
  std::string text = view.state->label();
  if (!view.state->known())
    text += " — Reopen to retry";
  else if (view.state->failed())
    text += " — Retry";
  if (view.state->value().value_or(false))
    text = "<b>" + text + "</b>";
  text = std::string(view.pressed ? "<color=#383838>" : "<color=#E1F2F7>") + text + "</color>";
  Root  label(Target(view.label));
  Root  message(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(text.c_str())));
  void* textArgs[] = {message.get()};
  Call(label.get(), "OverrideLocalizedText", 1, textArgs);
  view.overridden = true;
}

void TryStyleChoice(View& view)
{
  try {
    StyleChoice(view);
  } catch (...) {
    ClearChoiceStyle(view);
    Warn("settings selection presentation unavailable");
  }
}

void Render(View& view, auto original, Il2CppObject* widget)
{
  if (view.rendering)
    return;
  Root boundContext(Target(view.context));
  struct Scope {
    View&                        view;
    View*                        previous;
    NativeViewState::RenderScope suppress;
    Scope(View& view)
        : view(view)
        , previous(renderingView)
        , suppress(*view.state)
    {
      view.rendering = true;
      renderingView  = &view;
    }
    ~Scope()
    {
      renderingView  = previous;
      view.rendering = false;
    }
  } scope(view);
  Restore(view);
  original(widget);
  if (Target(view.widget) != widget || Target(view.context) != boundContext.get())
    return;
  Root        label(Target(view.label));
  std::string text = view.state->label();
  // The native row has limited label width: "Change not applied; try again" was
  // visibly truncated after "; tr" alongside the FC label. Keep these suffixes
  // short; recheck the full label at supported UI scales when changing wording.
  if (!view.state->known())
    text += " — Reopen to retry";
  else if (view.state->failed())
    text += " — Retry";
  else if (!view.state->enabled())
    text += " — Select Threshold";
  // An enabled slider belongs to the selected mode above it. Use a quiet cyan
  // accent, not the native white selection fill (the slider is not a choice).
  if (WidgetMeta(widget).slider && view.state->enabled()) {
    text = "<color=#A8E5EE>" + text + "</color>";
    TintRow(view.tint, widget, {0.70f, 1.0f, 1.0f, 1.0f});
  }
  Root  message(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(text.c_str())));
  void* args[] = {message.get()};
  Call(label.get(), "OverrideLocalizedText", 1, args);
  view.overridden = true;
  TryStyleChoice(view);
  if (!view.state->enabled()) {
    if (auto* control = Target(view.selectionControl)) {
      view.interactableBefore = Boolean(Call(control, "get_interactable"));
      view.disabled           = true;
      bool  interactable      = false;
      void* controlArgs[]     = {&interactable};
      Call(control, "set_interactable", 1, controlArgs);
      if (WidgetMeta(widget).selection || view.state->known())
        return;
    }
    // Capture all native values first (the two components may share a node).
    for (std::size_t i = 0; i < view.indicators.size(); ++i)
      view.activeBefore[i] = Boolean(Call(Target(view.indicators[i]), "get_activeSelf"));
    view.hidden = true;
    for (auto handle : view.indicators)
      SetActive(Target(handle), false);
  }
}
void HideUnsupported(Il2CppObject* widget)
{
  try {
    Root object(Call(widget, "get_gameObject"));
    SetActive(object.get(), false);
  } catch (...) {
  }
  Warn();
}
bool OnThread()
{ return active && std::this_thread::get_id() == uiThread; }

struct PageMetadata {
  IL2CppClassHelper category =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "CategoryOptionContext");
  IL2CppClassHelper categoryWidget =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "CategoryOptionWidget");
  IL2CppClassHelper controller =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "GameSettingsViewController");
  const MethodInfo* add       = Meta().context.GetMethodInfo("AddCategory", 4);
  const MethodInfo* bind      = categoryWidget.GetMethodInfo("OnDidBindContext", 0);
  const MethodInfo* release   = categoryWidget.GetMethodInfo("OnAboutToReleaseContext", 0);
  const MethodInfo* selected  = controller.GetMethodInfo("OnCategorySelected", 1);
  const MethodInfo* destroyed = controller.GetMethodInfo("OnDestroy", 0);
  FieldInfo*        label     = Field(categoryWidget.get_cls(), "_label");
  FieldInfo*        title     = Field(controller.get_cls(), "_title");
  FieldInfo*        panel     = Field(controller.get_cls(), "_optionTabPanel");
};
PageMetadata& PageMeta()
{
  static PageMetadata metadata;
  return metadata;
}
const PageCatalog::Page* PageFor(Il2CppObject* context)
{
  if (!context)
    return nullptr;
  for (const auto& page : pages)
    if (HasLabel(context, page.id.c_str()))
      return &page;
  return nullptr;
}

// One open page, with visit-local expansion state. The native context retains
// every child; only the list's presentation is filtered. Back and save ownership
// remain native, and a fresh page visit starts collapsed.
struct SectionPage {
  Il2CppGCHandle           controller = nullptr, context = nullptr;
  std::vector<std::string> collapsed;
  bool                     refreshing = false;
} sectionPage;
struct SectionRefreshScope {
  SectionRefreshScope()
  { sectionPage.refreshing = true; }
  ~SectionRefreshScope()
  { sectionPage.refreshing = false; }
};
void ClearSectionPage()
{
  Free(sectionPage.controller);
  Free(sectionPage.context);
  sectionPage.collapsed.clear();
}
const PageCatalog::Heading* CollapsibleHeadingFor(Il2CppObject* context)
{
  if (!context || context->klass != PageMeta().category.get_cls())
    return nullptr;
  auto* callback = reinterpret_cast<Il2CppDelegate*>(ReadField(context, Meta().queryField));
  if (!callback || callback->method != query.method() || callback->method_ptr != query.method()->methodPointer)
    return nullptr;
  Root parent(Call(context, "get_Parent"));
  if (const auto* page = PageFor(parent.get()))
    for (const auto& item : page->items)
      if (const auto* heading = std::get_if<PageCatalog::Heading>(&item);
          heading && heading->collapsible && HasLabel(context, heading->id.c_str()))
        return heading;
  return nullptr;
}
bool Collapsed(const PageCatalog::Heading& heading)
{
  return std::find(sectionPage.collapsed.begin(), sectionPage.collapsed.end(), heading.id)
         != sectionPage.collapsed.end();
}
void ShowSections(Il2CppObject* controller, Il2CppObject* context, const PageCatalog::Page& page)
{
  Root                       children(Call(context, "get_Children"));
  std::vector<Il2CppObject*> visible;
  for (int i = 0, count = Count(children.get()); i < count; ++i) {
    auto*                       row     = Item(children.get(), i); // Rooted by the unchanged native children.
    const PageCatalog::Heading* section = nullptr;
    if (Owned(row)) {
      if (const auto choice = ChoiceFor(row); choice.first)
        section = page.SectionFor(choice.first->state().id());
      else if (auto* slider = SliderFor(row))
        section = page.SectionFor(slider->state().id());
      else if (auto* setting = SettingFor(row))
        section = page.SectionFor(setting->id());
    }
    if (!section || !Collapsed(*section))
      visible.push_back(row);
  }
  auto options = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "OptionContext");
  Root list(reinterpret_cast<Il2CppObject*>(il2cpp_array_new(options.get_cls(), visible.size())));
  if (!list.get())
    throw std::runtime_error("settings section list allocation");
  for (std::size_t i = 0; i < visible.size(); ++i) {
    auto* array = reinterpret_cast<Il2CppArraySize*>(list.get());
    il2cpp_gc_wbarrier_set_field(list.get(), reinterpret_cast<void**>(&array->vector[i]), visible[i]);
  }
  Root        panel(ReadField(controller, PageMeta().panel));
  static auto widgets = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "Widget");
  static auto panels  = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "OptionTabPanelWidget");
  static const auto* schema = widgets.GetMethodInfo("BindDataContext", 2);
  auto*              lists  = il2cpp_class_from_name(il2cpp_get_corlib(), "System.Collections", "IList");
  if (!panel.get() || panel.get()->klass != panels.get_cls() || !Instance(schema, 2, IL2CPP_TYPE_VOID)
      || !(schema->flags & METHOD_ATTRIBUTE_VIRTUAL) || !Reference(schema->parameters[0])
      || !Type(schema->parameters[1], IL2CPP_TYPE_OBJECT) || !lists
      || !il2cpp_class_is_assignable_from(lists, list.get()->klass))
    throw std::runtime_error("settings section list schema");
  // Resolve the non-generic Widget virtual slot, not the same-arity typed
  // Widget<IList> overload. This is the object overload used by the game itself.
  const auto* bind = il2cpp_object_get_virtual_method(panel.get(), schema);
  if (!Instance(bind, 2, IL2CPP_TYPE_VOID) || bind->slot != schema->slot
      || !Type(bind->parameters[1], IL2CPP_TYPE_OBJECT)
      || il2cpp_class_from_type(bind->parameters[0]) != il2cpp_class_from_type(schema->parameters[0]))
    throw std::runtime_error("settings section virtual binding");
  // The same provider/null + IList bind used by native OnCategorySelected.
  // Native release/bind owns pooled widgets and their event subscriptions.
  void* args[] = {nullptr, list.get()};
  Invoke(bind, panel.get(), args);
}

struct PageText {
  Il2CppGCHandle owner = nullptr, label = nullptr;
  RowTint        tint;
  Il2CppGCHandle arrow = nullptr;
  Vector3        arrowBefore{};
};
std::vector<PageText> pageText;
void                  ClearPageText(Il2CppObject* owner)
{
  for (auto it = pageText.begin(); it != pageText.end();) {
    auto* live = Target(it->owner);
    if (live && live != owner) {
      ++it;
      continue;
    }
    RestoreTint(it->tint);
    try {
      if (auto* arrow = Target(it->arrow)) {
        void* args[] = {&it->arrowBefore};
        Call(arrow, "set_localEulerAngles", 1, args);
      }
    } catch (...) {
      Warn();
    }
    try {
      if (auto* label = Target(it->label))
        Call(label, "ClearTextOverride");
    } catch (...) {
      Warn();
    }
    Free(it->owner);
    Free(it->label);
    Free(it->arrow);
    it = pageText.erase(it);
  }
}
void SetPageText(Il2CppObject* owner, Il2CppObject* label, const std::string& text, bool heading = false,
                 std::optional<bool> expanded = {})
{
  if (!owner || !label)
    throw std::runtime_error("settings text missing");
  ClearPageText(owner);
  PageText record{il2cpp_gchandle_new_weakref(owner, false), il2cpp_gchandle_new_weakref(label, false)};
  try {
    if (!record.owner || !record.label)
      throw std::runtime_error("settings text weak root");
    if (heading) {
      Root background(RowImage(owner, expanded ? "Background" : "BG"));
      TintImage(record.tint, background.get(), {0.35f, 0.50f, 0.56f, 1.0f});
    }
    if (expanded) {
      Root image(RowImage(owner, "Arrow"));
      if (image.get()) {
        Root        arrow(Call(image.get(), "get_transform"));
        Root        rotation(Call(arrow.get(), "get_localEulerAngles"));
        auto        vectors = il2cpp_get_class_helper("UnityEngine.CoreModule", "UnityEngine", "Vector3");
        const auto* set     = il2cpp_class_get_method_from_name(arrow.get()->klass, "set_localEulerAngles", 1);
        if (rotation.get() && rotation.get()->klass == vectors.get_cls() && Instance(set, 1, IL2CPP_TYPE_VOID)
            && !set->parameters[0]->byref && il2cpp_class_from_type(set->parameters[0]) == vectors.get_cls()) {
          record.arrowBefore = *static_cast<Vector3*>(il2cpp_object_unbox(rotation.get()));
          record.arrow       = il2cpp_gchandle_new_weakref(arrow.get(), false);
          if (record.arrow) {
            auto value = record.arrowBefore;
            if (*expanded)
              value.z -= 90.0f;
            void* args[] = {&value};
            Invoke(set, arrow.get(), args);
          }
        }
      }
    }
    pageText.push_back(record);
  } catch (...) {
    RestoreTint(record.tint);
    if (auto* arrow = Target(record.arrow)) {
      try {
        void* args[] = {&record.arrowBefore};
        Call(arrow, "set_localEulerAngles", 1, args);
      } catch (...) {
        Warn();
      }
    }
    Free(record.arrow);
    Free(record.owner);
    Free(record.label);
    throw;
  }
  Root  message(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(text.c_str())));
  void* args[] = {message.get()};
  Call(label, "OverrideLocalizedText", 1, args);
}

void CategoryBindHook(auto original, Il2CppObject* widget)
{
  if (OnThread())
    ClearPageText(widget);
  original(widget);
  if (!OnThread() || !pagesActive)
    return;
  try {
    Root context(Call(widget, "get_Context"));
    if (auto* page = PageFor(context.get())) {
      try {
        Root background(RowImage(widget, "Background"));
        RememberChoiceSprite(background.get());
      } catch (...) {
        Warn("settings background unavailable");
      }
      Root label(ReadField(widget, PageMeta().label));
      SetPageText(widget, label.get(), page->label);
    } else if (const auto* heading = CollapsibleHeadingFor(context.get())) {
      Root label(ReadField(widget, PageMeta().label));
      SetPageText(widget, label.get(), "<b><size=115%><color=#9ADBE7>" + heading->label + "</color></size></b>", true,
                  !Collapsed(*heading));
    }
  } catch (...) {
    Warn();
  }
}
void CategoryReleaseHook(auto original, Il2CppObject* widget)
{
  if (OnThread())
    ClearPageText(widget);
  original(widget);
}
void PageSelectedHook(auto original, Il2CppObject* controller, Il2CppObject* context)
{
  if (OnThread() && pagesActive) {
    bool sectionClick = false;
    try {
      if (const auto* heading = CollapsibleHeadingFor(context)) {
        sectionClick = true;
        if (sectionPage.refreshing || Target(sectionPage.controller) != controller)
          return;
        Root parent(Call(context, "get_Parent"));
        Root canvas(Call(controller, "get_CanvasContext"));
        Root selected(Call(canvas.get(), "get_SelectedOption"));
        if (!parent.get() || parent.get() != Target(sectionPage.context) || selected.get() != parent.get())
          return; // An old pooled heading cannot navigate or change this page.
        SectionRefreshScope scope;
        const auto          before = sectionPage.collapsed;
        if (Collapsed(*heading))
          std::erase(sectionPage.collapsed, heading->id);
        else
          sectionPage.collapsed.push_back(heading->id);
        try {
          ShowSections(controller, parent.get(), *PageFor(parent.get()));
        } catch (...) {
          sectionPage.collapsed = before;
          try {
            ShowSections(controller, parent.get(), *PageFor(parent.get()));
          } catch (...) {
          }
          throw;
        }
        return; // A section click refreshes this page; it is not navigation.
      }
    } catch (const std::exception& error) {
      // Invoke converts managed failures to fixed messages, without game data.
      // Keep the concrete lookup/binding reason; a generic warning hid the
      // incorrect SetContext lookup that prevented sections from folding.
      Warn(error.what());
      if (sectionClick)
        return;
    } catch (...) {
      Warn("settings section unavailable");
      if (sectionClick)
        return;
    }
    ClearSectionPage();
    try {
      if (const auto* page = PageFor(context)) {
        sectionPage.controller = il2cpp_gchandle_new_weakref(controller, false);
        sectionPage.context    = il2cpp_gchandle_new_weakref(context, false);
        if (!sectionPage.controller || !sectionPage.context)
          ClearSectionPage();
        else
          for (const auto& item : page->items)
            if (const auto* heading = std::get_if<PageCatalog::Heading>(&item); heading && heading->collapsible)
              sectionPage.collapsed.push_back(heading->id);
      }
    } catch (...) {
      ClearSectionPage();
      Warn("settings section owner unavailable");
    }
  }
  if (OnThread())
    ClearPageText(controller);
  original(controller, context);
  if (!OnThread() || !pagesActive)
    return;
  try {
    if (auto* page = PageFor(context)) {
      Root label(ReadField(controller, PageMeta().title));
      SetPageText(controller, label.get(), page->label);
      if (Target(sectionPage.controller) == controller && Target(sectionPage.context) == context
          && !sectionPage.refreshing && !sectionPage.collapsed.empty()) {
        // Let native navigation establish the page and Back target, then apply
        // the initial folded presentation in the same call, before a frame draws.
        SectionRefreshScope scope;
        try {
          ShowSections(controller, context, *page);
        } catch (...) {
          // If folding is unavailable, keep the controls accessible and the
          // heading arrows consistent with the expanded fallback.
          sectionPage.collapsed.clear();
          try {
            ShowSections(controller, context, *page);
          } catch (...) {
          }
          throw;
        }
      }
    }
  } catch (...) {
    Warn();
  }
}
void PageDestroyedHook(auto original, Il2CppObject* controller)
{
  if (OnThread()) {
    ClearPageText(controller);
    if (Target(sectionPage.controller) == controller)
      ClearSectionPage();
  }
  original(controller);
}

struct HeadingMetadata {
  IL2CppClassHelper widget = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "TextOptionWidget");
  IL2CppClassHelper row = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameSettings", "TextOptionContext");
  const MethodInfo* refresh    = widget.GetMethodInfo("SetWidgetData", 0);
  const MethodInfo* clear      = widget.GetMethodInfo("ClearWidgetData", 0);
  const MethodInfo* add        = Meta().context.GetMethodInfo("AddText", 4);
  const MethodInfo* getContext = widget.GetMethodInfo("get_Context", 0);
  FieldInfo*        label      = Field(widget.get_cls(), "_label");
  FieldInfo*        queryField = Field(row.get_cls(), "<QueryOptionState>k__BackingField");
};
HeadingMetadata& HeadingMeta()
{
  static HeadingMetadata metadata;
  return metadata;
}
NativeCallback<Il2CppString*> headingGetter;
bool                          headingsActive = false;
Il2CppString*                 EmptyHeadingValue(Il2CppObject*, const MethodInfo*)
{ return il2cpp_string_new(""); }
const PageCatalog::Heading* HeadingFor(Il2CppObject* context)
{
  if (!context || context->klass != HeadingMeta().row.get_cls())
    return nullptr;
  auto* callback = reinterpret_cast<Il2CppDelegate*>(ReadField(context, HeadingMeta().queryField));
  if (!callback || callback->method != query.method() || callback->method_ptr != query.method()->methodPointer)
    return nullptr;
  for (const auto& page : pages)
    for (const auto& item : page.items)
      if (auto* heading = std::get_if<PageCatalog::Heading>(&item); heading && HasLabel(context, heading->id.c_str()))
        return heading;
  return nullptr;
}
void HeadingRefreshHook(auto original, Il2CppObject* widget)
{
  if (OnThread())
    ClearPageText(widget);
  original(widget);
  if (!OnThread() || !headingsActive || !pagesActive)
    return;
  try {
    Root context(Invoke(HeadingMeta().getContext, widget));
    if (const auto* heading = HeadingFor(context.get())) {
      Root label(ReadField(widget, HeadingMeta().label));
      // Rich text stays inside the existing local override and is cleared with
      // it. A darker bar and larger, bold label distinguish a heading from input.
      SetPageText(widget, label.get(), "<b><size=115%><color=#9ADBE7>" + heading->label + "</color></size></b>", true);
    }
  } catch (...) {
    Warn("settings heading unavailable");
  }
}
void HeadingClearHook(auto original, Il2CppObject* widget)
{
  if (OnThread())
    ClearPageText(widget);
  original(widget);
}
void AddHeadingRow(Il2CppObject* director, Il2CppObject* context, Il2CppObject* parent,
                   const PageCatalog::Heading& heading)
{
  if (heading.collapsible) {
    Root  id(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(heading.id.c_str())));
    Root  state(MakeDelegate(il2cpp_class_from_type(PageMeta().add->parameters[3]), director, query.method()));
    void* args[] = {parent, id.get(), id.get(), state.get()};
    Root  row(Invoke(PageMeta().add, context, args));
    if (!CollapsibleHeadingFor(row.get()))
      throw std::runtime_error("settings section identity");
    return;
  }
  if (!headingsActive)
    throw std::runtime_error("settings heading adapter missing");
  const auto* add = HeadingMeta().add;
  Root        children(Call(parent, "get_Children"));
  const int   before = Count(children.get());
  if (before == 128)
    throw std::runtime_error("settings heading capacity");
  Root  label(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(heading.id.c_str())));
  Root  get(MakeDelegate(il2cpp_class_from_type(add->parameters[2]), director, headingGetter.method()));
  Root  state(MakeDelegate(il2cpp_class_from_type(add->parameters[3]), director, query.method()));
  void* args[] = {parent, label.get(), get.get(), state.get()};
  Invoke(add, context, args);
  if (Count(children.get()) != before + 1)
    throw std::runtime_error("settings heading insertion");
  Root row(Item(children.get(), before));
  if (!HeadingFor(row.get()) || !HasLabel(row.get(), heading.id.c_str()))
    throw std::runtime_error("settings heading identity");
}

void AddChoiceRows(Il2CppObject* director, Il2CppObject* context, Il2CppObject* parent, ChoiceSetting& setting)
{
  if (!selectionActive)
    return;
  auto&       m   = SelectionMeta();
  const auto* add = m.context.GetMethodInfo("AddSelection", 6);
  Root        children(Call(parent, "get_Children"));
  const int   before = Count(children.get());
  const auto  count  = setting.labels().size();
  if (before + count > 128)
    throw std::runtime_error("selection category capacity");
  Root values(reinterpret_cast<Il2CppObject*>(
      il2cpp_array_new(il2cpp_class_from_name(il2cpp_get_corlib(), "System", "String"), count)));
  for (std::size_t i = 0; i < count; ++i) {
    Root  value(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(setting.item_id(static_cast<int>(i)).c_str())));
    auto* array = reinterpret_cast<Il2CppArraySize*>(values.get());
    il2cpp_gc_wbarrier_set_field(values.get(), reinterpret_cast<void**>(&array->vector[i]), value.get());
  }
  Root  label(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(setting.state().id().c_str())));
  Root  category(reinterpret_cast<Il2CppObject*>(il2cpp_string_new("")));
  Root  get(MakeDelegate(il2cpp_class_from_type(add->parameters[3]), director, selectionGetter.method()));
  Root  set(MakeDelegate(il2cpp_class_from_type(add->parameters[4]), director, selectionSetter.method()));
  void* args[] = {parent, label.get(), values.get(), get.get(), set.get(), category.get()};
  Invoke(add, context, args);
  if (Count(children.get()) != before + static_cast<int>(count))
    throw std::runtime_error("selection row insertion");
  for (int i = 0; i < static_cast<int>(count); ++i) {
    Root row(Item(children.get(), before + i));
    if (row.get()->klass != m.row.get_cls())
      throw std::runtime_error("selection row class");
    Root index(Call(row.get(), "get_Index"));
    if (!index.get() || !Type(il2cpp_class_get_type(index.get()->klass), IL2CPP_TYPE_I4)
        || *static_cast<int*>(il2cpp_object_unbox(index.get())) != i)
      throw std::runtime_error("selection row index");
    Root  text(Call(row.get(), "get_LabelContext"));
    Root  id(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(setting.item_id(i).c_str())));
    void* labelArgs[] = {id.get()};
    Call(text.get(), "set_Identifier", 1, labelArgs);
    Root  state(MakeDelegate(il2cpp_class_from_type(m.querySetter->parameters[0]), director, query.method()));
    void* stateArgs[] = {state.get()};
    Invoke(m.querySetter, row.get(), stateArgs);
  }
}

void AddSliderRow(Il2CppObject* director, Il2CppObject* context, Il2CppObject* parent, SliderSetting& setting)
{
  if (!sliderActive)
    return;
  auto&       m   = SliderMeta();
  const auto* add = m.context.GetMethodInfo("AddSlider", 9);
  Root        children(Call(parent, "get_Children"));
  const int   before = Count(children.get());
  if (before == 128)
    throw std::runtime_error("slider category capacity");
  Root  label(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(setting.state().id().c_str())));
  Root  get(MakeDelegate(il2cpp_class_from_type(add->parameters[2]), director, sliderGetter.method()));
  Root  set(MakeDelegate(il2cpp_class_from_type(add->parameters[3]), director, sliderSetter.method()));
  Root  state(MakeDelegate(il2cpp_class_from_type(add->parameters[4]), director, query.method()));
  bool  whole   = false;
  float minimum = setting.minimum(), maximum = setting.maximum();
  void* args[] = {parent, label.get(), get.get(), set.get(), state.get(), nullptr, &whole, &minimum, &maximum};
  Invoke(add, context, args);
  if (Count(children.get()) != before + 1)
    throw std::runtime_error("slider row insertion");
  Root row(Item(children.get(), before));
  if (row.get()->klass != m.row.get_cls() || !HasLabel(row.get(), setting.state().id().c_str()))
    throw std::runtime_error("slider row identity");
  int   percentage  = 1; // SliderOptionLabelType.Percentage; value stays normalized 0..1.
  void* labelArgs[] = {&percentage};
  Call(row.get(), "set_LabelType", 1, labelArgs);
}

void AddPages(Il2CppObject* director, Il2CppObject* context)
{
  if (!pagesActive || pages.empty())
    return;
  Root root(Call(context, "get_RootOption"));
  Root children(Call(root.get(), "get_Children"));
  for (int i = 0, count = Count(children.get()); i < count; ++i)
    if (HasLabel(Item(children.get(), i), pages.front().id.c_str()))
      return;
  std::map<std::string, Il2CppObject*> parents;
  Il2CppObject*                        addedRoot = nullptr;
  std::optional<Root>                  addedRootGuard;
  try {
    for (const auto& page : pages) {
      auto* parent = page.parent.empty() ? root.get() : parents.at(page.parent);
      Root  id(reinterpret_cast<Il2CppObject*>(il2cpp_string_new(page.id.c_str())));
      Root  state(MakeDelegate(il2cpp_class_from_type(PageMeta().add->parameters[3]), director, query.method()));
      void* args[] = {parent, id.get(), id.get(), state.get()};
      Root  category(Invoke(PageMeta().add, context, args));
      if (!category.get())
        throw std::runtime_error("settings category construction");
      if (!addedRoot) {
        addedRoot = category.get();
        addedRootGuard.emplace(addedRoot);
      }
      if (!HasLabel(category.get(), page.id.c_str()))
        throw std::runtime_error("settings category identity");
      parents.emplace(page.id, category.get()); // Native root owns all added contexts.
      for (const auto& item : page.items)
        std::visit(
            [&](const auto& value) {
              using T = std::decay_t<decltype(value)>;
              if constexpr (std::is_same_v<T, PageCatalog::Heading>)
                AddHeadingRow(director, context, category.get(), value);
              else if constexpr (std::is_same_v<T, BooleanSetting*>)
                AddBooleanRow(director, context, category.get(), *value);
              else if constexpr (std::is_same_v<T, ChoiceSetting*>)
                AddChoiceRows(director, context, category.get(), *value);
              else
                AddSliderRow(director, context, category.get(), *value);
            },
            item);
    }
    // Unsupported leaf adapters can leave empty groups; remove them bottom-up.
    for (auto it = pages.rbegin(); it != pages.rend(); ++it) {
      auto* category = parents.at(it->id);
      Root  items(Call(category, "get_Children"));
      if (Count(items.get()) != 0)
        continue;
      auto* parent = it->parent.empty() ? root.get() : parents.at(it->parent);
      void* args[] = {category};
      Call(parent, "RemoveChild", 1, args);
    }
  } catch (...) {
    if (addedRoot) {
      void* args[] = {addedRoot};
      Call(root.get(), "RemoveChild", 1, args);
    }
    throw;
  }
}

void AddGeneralHook(auto original, Il2CppObject* director, Il2CppObject* context)
{
  original(director, context);
  if (!OnThread())
    return;
  try {
    AddRow(director, context);
  } catch (...) {
    Warn();
  }
  try {
    AddPages(director, context);
  } catch (...) {
    Warn();
  }
}
void RefreshHook(auto original, Il2CppObject* widget)
{
  if (!OnThread()) {
    original(widget);
    return;
  }
  bool owned = false;
  try {
    Root context(Invoke(WidgetMeta(widget).getContext, widget));
    owned      = Owned(context.get());
    auto* view = Find(widget);
    if (view && (view->rendering || view->binding || view->clearing))
      return;
    if (view && Target(view->context) != context.get()) {
      Clear(*view);
      view = nullptr;
    }
    if (!owned) {
      if (view)
        Clear(*view);
    } else {
      if (!view)
        view = &Track(widget, context.get());
      // Observe calls a feature-owned reader. It may synchronously release this
      // widget and bind another, so protect the slot before calling Bind too.
      struct Binding {
        View& view;
        View* previous;
        explicit Binding(View& view)
            : view(view)
            , previous(bindingView)
        {
          view.binding = true;
          bindingView  = &view;
        }
        ~Binding()
        {
          view.binding = false;
          bindingView  = previous;
        }
      } binding(*view);
      if (!view->preserveNextRefresh)
        view->state->Bind();
      if (Target(view->widget) != widget || Target(view->context) != context.get()) {
        view->state->Unbind();
        return;
      }
      view->preserveNextRefresh = false;
      Render(*view, original, widget);
      return;
    }
  } catch (const std::exception& error) {
    Warn(error.what());
    if (owned) {
      HideUnsupported(widget);
      return;
    }
  } catch (...) {
    if (owned) {
      HideUnsupported(widget);
      return;
    }
    Warn();
  }
  original(widget);
}
void SelectionTransitionHook(auto original, Il2CppObject* control, int state, bool instant)
{
  original(control, state, instant);
  if (!OnThread() || !selectionActive)
    return;
  static auto* toggleClass = il2cpp_class_from_type(SelectionMeta().toggleField->type);
  if (!control || control->klass != toggleClass)
    return;
  for (auto& view : Views()) {
    if (Target(view.selectionControl) != control || !view.state)
      continue;
    view.pressed = state == 2; // Selectable.SelectionState.Pressed, not Selected/focus.
    if (view.rendering || view.binding || view.requesting || view.clearing)
      return;
    try {
      Root widget(Target(view.widget));
      Root context(widget.get() ? Invoke(WidgetMeta(widget.get()).getContext, widget.get()) : nullptr);
      if (!context.get() || Target(view.context) != context.get() || !Owned(context.get()))
        return;
      struct Scope {
        View&                        view;
        NativeViewState::RenderScope suppress;
        Scope(View& view)
            : view(view)
            , suppress(*view.state)
        { view.rendering = true; }
        ~Scope()
        { view.rendering = false; }
      } scope(view);
      // On/Off animation may have first published its sprite since the last
      // bind. Learn it on input, never through an Update hook or timer.
      if (!Target(pressedChoiceSprite))
        for (auto& sibling : Views())
          if (auto* background = Target(sibling.choiceBackground))
            RememberChoiceSprite(background);
      TryStyleChoice(view);
    } catch (...) {
      Warn("settings selection presentation unavailable");
    }
    return;
  }
}
void RefreshViews()
{
  if (!OnThread())
    return;
  for (auto& view : Views()) {
    if (view.requesting || view.rendering || view.binding || view.clearing)
      continue;
    Root widget(Target(view.widget));
    if (!widget.get())
      continue;
    try {
      Root context(Invoke(WidgetMeta(widget.get()).getContext, widget.get()));
      if (Target(view.context) != context.get() || !Owned(context.get()))
        continue;
      Invoke(WidgetMeta(widget.get()).refresh, widget.get());
    } catch (...) {
      HideUnsupported(widget.get());
    }
  }
}
void ChangeValue(auto original, Il2CppObject* widget, auto desired)
{
  if (!OnThread()) {
    original(widget, desired);
    return;
  }
  bool owned = false;
  try {
    Root context(Invoke(WidgetMeta(widget).getContext, widget));
    owned = Owned(context.get());
    if (owned) {
      auto* view = Find(widget);
      if (!view || view->rendering || view->binding || view->requesting || view->clearing
          || Target(view->context) != context.get())
        return;
      struct RequestScope {
        View& view;
        explicit RequestScope(View* view)
            : view(*view)
        { view->requesting = true; }
        ~RequestScope()
        { view.requesting = false; }
      } requestScope(view);
      auto result = view->state->Request(desired);
      if (Target(view->widget) != widget || Target(view->context) != context.get())
        return;
      if (result == Outcome::Suppressed || result == Outcome::Busy)
        return;
      // Refresh through the hook once, preserving the write result. A fresh Bind
      // here would erase an unverified outcome merely because a later read works.
      view->preserveNextRefresh = true;
      Invoke(WidgetMeta(widget).refresh, widget);
      return;
    }
  } catch (...) {
    if (owned) {
      HideUnsupported(widget);
      return;
    }
    Warn();
  }
  original(widget, desired);
}
void ChangedHook(auto original, Il2CppObject* widget, bool desired)
{ ChangeValue(original, widget, desired); }
void SliderChangedHook(auto original, Il2CppObject* widget, float desired)
{ ChangeValue(original, widget, desired); }
void ReleaseHook(auto original, Il2CppObject* widget)
{
  if (OnThread()) {
    try {
      if (auto* view = Find(widget))
        Clear(*view);
    } catch (...) {
      Warn();
    }
  }
  original(widget);
}
void Invalidate()
{
  ClearSectionPage();
  InvalidateFleetCommanderConfirmationSession();
  ForbiddenTechConfirmationSetting().InvalidateSession();
  for (const auto& page : pages)
    for (auto* setting : page.Controls<SliderSetting>())
      setting->state().InvalidateSession();
  for (const auto& page : pages)
    for (auto* choice : page.Controls<ChoiceSetting>())
      choice->state().InvalidateSession();
  for (const auto& page : pages)
    for (auto* setting : page.Controls<BooleanSetting>())
      if (setting != &FleetCommanderConfirmationSetting())
        setting->InvalidateSession();
  for (auto& view : Views()) {
    if (!view.state)
      continue;
    view.state->Invalidate();
    // Re-rendering during a native account transition can read the old account.
    // Hide the indicators immediately; next explicit bind may establish readiness.
    if (auto* widget = Target(view.widget)) {
      try {
        Render(view, [](Il2CppObject*) {}, widget);
      } catch (...) {
        HideUnsupported(widget);
      }
    }
  }
}
void SessionBoundary(auto original, Il2CppObject* owner)
{
  if (OnThread()) {
    try {
      Invalidate();
    } catch (...) {
      Warn();
    }
  }
  original(owner);
}
void ReloadHook(auto original, Il2CppObject* owner)
{ SessionBoundary(original, owner); }
void SessionHook(auto original, Il2CppObject* owner)
{ SessionBoundary(original, owner); }
void LoadHook(auto original, Il2CppObject* owner)
{ SessionBoundary(original, owner); }
bool Extent(const MethodInfo* method)
{
  if (!method || !method->methodPointer)
    return false;
  DWORD64    base    = 0;
  const auto address = reinterpret_cast<DWORD64>(method->methodPointer);
  auto*      entry   = RtlLookupFunctionEntry(address, &base, nullptr);
  // Bundled x64 SPUD reserves 24 bytes; the 64-byte minimum and exact entry reject
  // shared tiny accessors/thunks. Only Windows x64 is enabled by this adapter.
  return entry && base + entry->BeginAddress == address && entry->EndAddress - entry->BeginAddress >= 64;
}

#ifdef _MODDBG
View* writeProbeOuter = nullptr;
bool  ReentryProbeEnabled()
{
  const auto* enabled = std::getenv("STFC_MOD_SETTINGS_NAV_REENTRY_TEST");
  return enabled && std::strcmp(enabled, "1") == 0;
}
void ExerciseReadReentry()
{
  static bool exercised = false;
  if (exercised || !bindingView || !ReentryProbeEnabled())
    return;
  exercised      = true;
  auto* previous = bindingView;
  Root  widget(Target(previous->widget));
  // Exercise the actual release bookkeeping and refresh path inside a reader.
  // Keep the native context bound so this is independent of game navigation.
  Clear(*previous);
  Invoke(WidgetMeta(widget.get()).refresh, widget.get());
  auto* rebound = Find(widget.get());
  spdlog::info("[ModSettings] Read reentry fixture: {}",
               rebound && rebound != previous && previous->binding ? "PASS" : "FAIL");
}
void ExerciseNestedWrite()
{
  static bool exercised = false;
  if (exercised || !ReentryProbeEnabled())
    return;
  View* outer  = nullptr;
  View* nested = nullptr;
  for (auto& view : Views()) {
    if (!Target(view.widget) || !view.state)
      continue;
    if (view.requesting && view.state->id() == "community_mod.test.enabled")
      outer = &view;
    if (view.state->id() == "community_mod.test.nested_write")
      nested = &view;
  }
  if (!outer || !nested)
    return;
  exercised = true;
  struct Scope {
    explicit Scope(View* outer)
    { writeProbeOuter = outer; }
    ~Scope()
    { writeProbeOuter = nullptr; }
  } scope(outer);
  Root  widget(Target(nested->widget));
  bool  desired = !nested->state->value().value_or(false);
  void* args[]  = {&desired};
  Invoke(Meta().changed, widget.get(), args);
}
void RebindOuterWrite()
{
  if (!writeProbeOuter)
    return;
  Root widget(Target(writeProbeOuter->widget));
  Clear(*writeProbeOuter);
  Invoke(WidgetMeta(widget.get()).refresh, widget.get());
  auto* rebound = Find(widget.get());
  spdlog::info("[ModSettings] Nested write reentry fixture: {}",
               rebound && rebound != writeProbeOuter && writeProbeOuter->requesting ? "PASS" : "FAIL");
}
#endif

void InstallPages()
{
  RegisterModPages();
#ifdef _MODDBG
  // Temporary opt-in navigation fixture; no real mod feature placement is chosen.
  // It mirrors the existing FC owner so rebuilds never introduce a second value.
  if (const auto* probe = std::getenv("STFC_MOD_SETTINGS_NAV_TEST"); probe && std::strcmp(probe, "1") == 0) {
    auto& catalog = ModPages();
    catalog.AddPage("community_mod.test", "Infrastructure Test", "community_mod.settings");
    catalog.AddPage("community_mod.test.nested", "Nested Group", "community_mod.test");
    catalog.AddBoolean("community_mod.test.nested", FleetCommanderConfirmationSetting());
    static bool           value = false;
    static BooleanSetting fixture({"community_mod.test.enabled", "[MOD] Infrastructure test toggle",
                                   [] {
                                     ExerciseReadReentry();
                                     return ReadResult::Known(value, 1);
                                   },
                                   [](bool desired, std::uint64_t generation) {
                                     if (generation != 1)
                                       return ApplyResult::Rejected;
                                     ExerciseNestedWrite();
                                     value = desired;
                                     return ApplyResult::Applied;
                                   }});
    catalog.AddBoolean("community_mod.test.nested", fixture);
    if (ReentryProbeEnabled()) {
      static bool           nestedValue = false;
      static BooleanSetting nestedFixture({"community_mod.test.nested_write", "[MOD] Nested write test toggle",
                                           [] { return ReadResult::Known(nestedValue, 1); },
                                           [](bool desired, std::uint64_t generation) {
                                             if (generation != 1)
                                               return ApplyResult::Rejected;
                                             RebindOuterWrite();
                                             nestedValue = desired;
                                             return ApplyResult::Applied;
                                           }});
      catalog.AddBoolean("community_mod.test.nested", nestedFixture);
    }
  }
#endif
  pages                = ModPages().Build();
  std::size_t rowCount = 2; // FC and FT live on the native confirmation page.
  for (const auto& page : pages)
    rowCount += page.ControlRows();
  Views().resize(std::max(Views().size(), rowCount));
  if (pages.empty())
    return;
  auto&            m = PageMeta();
  const std::array hooks{m.bind, m.release, m.selected, m.destroyed};
  for (std::size_t i = 0; i < hooks.size(); ++i) {
    if (!Instance(hooks[i], i == 2 ? 1 : 0, IL2CPP_TYPE_VOID) || !Extent(hooks[i]))
      throw std::runtime_error("settings page hook metadata/extent");
    for (std::size_t j = 0; j < i; ++j)
      if (hooks[i]->methodPointer == hooks[j]->methodPointer)
        throw std::runtime_error("settings page shared hook");
    const auto& core = Meta();
    for (auto* owned :
         {core.addGeneral, core.refresh, core.changed, core.release, core.reload, core.session, core.load})
      if (hooks[i]->methodPointer == owned->methodPointer)
        throw std::runtime_error("settings page overlaps existing hook");
  }
  if (!Instance(m.add, 4, IL2CPP_TYPE_CLASS) || !Reference(m.add->parameters[0])
      || !Type(m.add->parameters[1], IL2CPP_TYPE_STRING) || !Type(m.add->parameters[2], IL2CPP_TYPE_STRING)
      || !Reference(m.add->parameters[3]) || !Reference(m.selected->parameters[0]))
    throw std::runtime_error("settings category signature");
  if (std::any_of(pages.begin(), pages.end(),
                  [](const auto& page) { return !page.template Controls<ChoiceSetting>().empty(); })) {
    auto&       selection = SelectionMeta();
    const auto* get       = selection.director.GetMethodInfo("GetQualityOptionSelectedIndex", 0);
    const auto* set       = selection.director.GetMethodInfo("OnQualityOptionSelected", 1);
    const auto* add       = selection.context.GetMethodInfo("AddSelection", 6);
    if (!Instance(get, 0, IL2CPP_TYPE_I4) || !Instance(set, 1, IL2CPP_TYPE_VOID)
        || !Type(set->parameters[0], IL2CPP_TYPE_I4) || !Instance(add, 6, IL2CPP_TYPE_VOID)
        || !Reference(add->parameters[0]) || !Type(add->parameters[1], IL2CPP_TYPE_STRING)
        || !Type(add->parameters[2], IL2CPP_TYPE_SZARRAY) || !Reference(add->parameters[3])
        || !Reference(add->parameters[4]) || !Type(add->parameters[5], IL2CPP_TYPE_STRING)
        || !Instance(selection.querySetter, 1, IL2CPP_TYPE_VOID) || !Reference(selection.querySetter->parameters[0])
        || !selection.getContext || !Reference(selection.getContext->return_type)
        || !Instance(selection.getContext, 0, selection.getContext->return_type->type)
        || !selectionGetter.Initialize(get, GetSelected) || !selectionSetter.Initialize(set, SetSelected))
      throw std::runtime_error("selection callback schema");
    static auto selectable = il2cpp_get_class_helper("UnityEngine.UI", "UnityEngine.UI", "Selectable");
    const auto* transition = selectable.GetMethodInfo("DoStateTransition", 2);
    if (!Instance(transition, 2, IL2CPP_TYPE_VOID) || !Type(transition->parameters[0], IL2CPP_TYPE_VALUETYPE)
        || !il2cpp_class_is_enum(il2cpp_class_from_type(transition->parameters[0]))
        || !Type(il2cpp_class_enum_basetype(il2cpp_class_from_type(transition->parameters[0])), IL2CPP_TYPE_I4)
        || !Type(transition->parameters[1], IL2CPP_TYPE_BOOLEAN))
      throw std::runtime_error("selection transition signature");
    const std::array targets{selection.refresh, selection.changed, selection.release, transition};
    for (std::size_t i = 0; i < targets.size(); ++i) {
      if (!Instance(targets[i], i == 3 ? 2 : i == 1 ? 1 : 0, IL2CPP_TYPE_VOID) || !Extent(targets[i]))
        throw std::runtime_error("selection hook metadata/extent");
      if (i == 1 && !Type(targets[i]->parameters[0], IL2CPP_TYPE_BOOLEAN))
        throw std::runtime_error("selection changed signature");
      for (std::size_t j = 0; j < i; ++j)
        if (targets[i]->methodPointer == targets[j]->methodPointer)
          throw std::runtime_error("selection shared hook");
      const auto& core = Meta();
      for (auto* existing : {core.refresh, core.changed, core.release, core.addGeneral, core.reload, core.session,
                             core.load, m.bind, m.release, m.selected, m.destroyed})
        if (targets[i]->methodPointer == existing->methodPointer)
          throw std::runtime_error("selection hook overlap");
    }
    for (const auto& page : pages)
      for (auto* choice : page.Controls<ChoiceSetting>())
        if (!choice->state().SetChangeObserver(RefreshViews))
          throw std::runtime_error("selection observer ownership");
    SPUD_STATIC_DETOUR(selection.refresh->methodPointer, RefreshHook);
    SPUD_STATIC_DETOUR(selection.changed->methodPointer, ChangedHook);
    SPUD_STATIC_DETOUR(selection.release->methodPointer, ReleaseHook);
    SPUD_STATIC_DETOUR(transition->methodPointer, SelectionTransitionHook);
    selectionActive = true;
  }
  if (std::any_of(pages.begin(), pages.end(),
                  [](const auto& page) { return !page.template Controls<SliderSetting>().empty(); })) {
    auto&       slider    = SliderMeta();
    const auto* get       = slider.director.GetMethodInfo("GetCurrentShadowsIndex", 0);
    const auto* set       = slider.director.GetMethodInfo("OnShadowsSettingChanged", 1);
    const auto* add       = slider.context.GetMethodInfo("AddSlider", 9);
    const auto* labelType = slider.row.GetMethodInfo("set_LabelType", 1);
    if (!Instance(get, 0, IL2CPP_TYPE_R4) || !Instance(set, 1, IL2CPP_TYPE_VOID)
        || !Type(set->parameters[0], IL2CPP_TYPE_R4) || !Instance(add, 9, IL2CPP_TYPE_VOID)
        || !Reference(add->parameters[0]) || !Type(add->parameters[1], IL2CPP_TYPE_STRING)
        || !Reference(add->parameters[2]) || !Reference(add->parameters[3]) || !Reference(add->parameters[4])
        || !Type(add->parameters[5], IL2CPP_TYPE_SZARRAY) || !Type(add->parameters[6], IL2CPP_TYPE_BOOLEAN)
        || !Type(add->parameters[7], IL2CPP_TYPE_R4) || !Type(add->parameters[8], IL2CPP_TYPE_R4)
        || !Instance(labelType, 1, IL2CPP_TYPE_VOID) || !Type(labelType->parameters[0], IL2CPP_TYPE_VALUETYPE)
        || !il2cpp_class_is_enum(il2cpp_class_from_type(labelType->parameters[0]))
        || !Type(il2cpp_class_enum_basetype(il2cpp_class_from_type(labelType->parameters[0])), IL2CPP_TYPE_I4)
        || !slider.getContext || !Reference(slider.getContext->return_type)
        || !Instance(slider.getContext, 0, slider.getContext->return_type->type)
        || !sliderGetter.Initialize(get, GetNumber) || !sliderSetter.Initialize(set, SetNumber))
      throw std::runtime_error("slider callback schema");
    const std::array targets{slider.refresh, slider.changed, slider.release};
    for (std::size_t i = 0; i < targets.size(); ++i) {
      if (!Instance(targets[i], i == 1 ? 1 : 0, IL2CPP_TYPE_VOID) || !Extent(targets[i]))
        throw std::runtime_error("slider hook metadata/extent");
      if (i == 1 && !Type(targets[i]->parameters[0], IL2CPP_TYPE_R4))
        throw std::runtime_error("slider changed signature");
      for (std::size_t j = 0; j < i; ++j)
        if (targets[i]->methodPointer == targets[j]->methodPointer)
          throw std::runtime_error("slider shared hook");
      const auto& core = Meta();
      for (auto* existing : {core.refresh, core.changed, core.release, core.addGeneral, core.reload, core.session,
                             core.load, m.bind, m.release, m.selected, m.destroyed})
        if (targets[i]->methodPointer == existing->methodPointer)
          throw std::runtime_error("slider hook overlap");
      if (selectionActive)
        for (auto* existing : {SelectionMeta().refresh, SelectionMeta().changed, SelectionMeta().release})
          if (targets[i]->methodPointer == existing->methodPointer)
            throw std::runtime_error("slider selection overlap");
    }
    for (const auto& page : pages)
      for (auto* setting : page.Controls<SliderSetting>())
        if (!setting->state().SetChangeObserver(RefreshViews))
          throw std::runtime_error("slider observer ownership");
    SPUD_STATIC_DETOUR(slider.refresh->methodPointer, RefreshHook);
    SPUD_STATIC_DETOUR(slider.changed->methodPointer, SliderChangedHook);
    SPUD_STATIC_DETOUR(slider.release->methodPointer, ReleaseHook);
    sliderActive = true;
  }
  if (std::any_of(pages.begin(), pages.end(), [](const auto& page) {
        return std::any_of(page.items.begin(), page.items.end(), [](const auto& item) {
          const auto* heading = std::get_if<PageCatalog::Heading>(&item);
          return heading && !heading->collapsible;
        });
      })) {
    auto&       heading = HeadingMeta();
    const auto* get     = Meta().director.GetMethodInfo("GetClientVersion", 0);
    if (!Instance(get, 0, IL2CPP_TYPE_STRING) || !Instance(heading.add, 4, IL2CPP_TYPE_VOID)
        || !Reference(heading.add->parameters[0]) || !Type(heading.add->parameters[1], IL2CPP_TYPE_STRING)
        || !Reference(heading.add->parameters[2]) || !Reference(heading.add->parameters[3]) || !heading.getContext
        || !Reference(heading.getContext->return_type)
        || !Instance(heading.getContext, 0, heading.getContext->return_type->type)
        || !headingGetter.Initialize(get, EmptyHeadingValue))
      throw std::runtime_error("heading callback schema");
    const std::array targets{heading.refresh, heading.clear};
    for (auto* target : targets) {
      if (!Instance(target, 0, IL2CPP_TYPE_VOID) || !Extent(target)
          || targets[0]->methodPointer == targets[1]->methodPointer)
        throw std::runtime_error("heading hook metadata/extent");
      const auto& core = Meta();
      for (auto* existing : {core.refresh, core.changed, core.release, core.addGeneral, core.reload, core.session,
                             core.load, m.bind, m.release, m.selected, m.destroyed})
        if (target->methodPointer == existing->methodPointer)
          throw std::runtime_error("heading hook overlap");
      if (selectionActive)
        for (auto* existing : {SelectionMeta().refresh, SelectionMeta().changed, SelectionMeta().release})
          if (target->methodPointer == existing->methodPointer)
            throw std::runtime_error("heading selection overlap");
      if (sliderActive)
        for (auto* existing : {SliderMeta().refresh, SliderMeta().changed, SliderMeta().release})
          if (target->methodPointer == existing->methodPointer)
            throw std::runtime_error("heading slider overlap");
    }
    SPUD_STATIC_DETOUR(heading.refresh->methodPointer, HeadingRefreshHook);
    SPUD_STATIC_DETOUR(heading.clear->methodPointer, HeadingClearHook);
    headingsActive = true;
  }
  for (const auto& page : pages)
    for (auto* setting : page.Controls<BooleanSetting>()) {
      if (setting->id() == FleetCommanderConfirmationSetting().id() && setting != &FleetCommanderConfirmationSetting())
        throw std::runtime_error("settings owner collision");
      if (!setting->SetChangeObserver(RefreshViews))
        throw std::runtime_error("settings observer ownership");
    }
  SPUD_STATIC_DETOUR(m.bind->methodPointer, CategoryBindHook);
  SPUD_STATIC_DETOUR(m.release->methodPointer, CategoryReleaseHook);
  SPUD_STATIC_DETOUR(m.selected->methodPointer, PageSelectedHook);
  SPUD_STATIC_DETOUR(m.destroyed->methodPointer, PageDestroyedHook);
  pagesActive = true;
  spdlog::info("[ModSettings] Native navigation installed: {} registered pages", pages.size());
}
} // namespace

void InstallModConfirmationSettings()
{
  if (active || installing)
    return;
  installing = true;
  try {
    auto&            m = Meta();
    const std::array hooks{m.addGeneral, m.refresh, m.changed, m.release, m.reload, m.session, m.load};
    for (std::size_t i = 0; i < hooks.size(); ++i) {
      if (!Instance(hooks[i], i == 0 || i == 2 ? 1 : 0, IL2CPP_TYPE_VOID) || !Extent(hooks[i]))
        throw std::runtime_error("settings hook metadata/extent");
      for (std::size_t j = 0; j < i; ++j)
        if (hooks[i]->methodPointer == hooks[j]->methodPointer)
          throw std::runtime_error("settings shared hook");
    }
    const auto* getSchema   = m.director.GetMethodInfo("IsBorgCubeCuttingBeamConfirmationOn", 0);
    const auto* setSchema   = m.director.GetMethodInfo("ToggleBorgCubeCuttingBeamConfirmation", 1);
    const auto* querySchema = m.director.GetMethodInfo("QueryShouldShowGenericPcSetting", 0);
    if (!Instance(getSchema, 0, IL2CPP_TYPE_BOOLEAN) || !Instance(setSchema, 1, IL2CPP_TYPE_VOID)
        || !Type(setSchema->parameters[0], IL2CPP_TYPE_BOOLEAN) || !querySchema
        || !il2cpp_class_is_enum(il2cpp_class_from_type(querySchema->return_type))
        || !Type(il2cpp_class_enum_basetype(il2cpp_class_from_type(querySchema->return_type)), IL2CPP_TYPE_I4)
        || !Instance(querySchema, 0, IL2CPP_TYPE_VALUETYPE) || !Instance(m.addToggle, 4, IL2CPP_TYPE_VOID)
        || !Type(m.addToggle->parameters[1], IL2CPP_TYPE_STRING) || !Reference(m.addToggle->parameters[0])
        || !Reference(m.addToggle->parameters[2]) || !Reference(m.addToggle->parameters[3])
        || !Type(m.changed->parameters[0], IL2CPP_TYPE_BOOLEAN) || !Reference(m.addGeneral->parameters[0])
        || !m.getContext || !Reference(m.getContext->return_type) || !Instance(m.querySetter, 1, IL2CPP_TYPE_VOID)
        || !Reference(m.querySetter->parameters[0]) || !getter.Initialize(getSchema, GetEnabled)
        || !setter.Initialize(setSchema, SetEnabled) || !query.Initialize(querySchema, QueryState))
      throw std::runtime_error("settings callback schema");
    uiThread = std::this_thread::get_id();
    if (!FleetCommanderConfirmationSetting().SetChangeObserver(RefreshViews))
      throw std::runtime_error("settings observer ownership");
    if (!ForbiddenTechConfirmationSetting().SetChangeObserver(RefreshViews))
      throw std::runtime_error("settings observer ownership");
    SPUD_STATIC_DETOUR(m.refresh->methodPointer, RefreshHook);
    SPUD_STATIC_DETOUR(m.changed->methodPointer, ChangedHook);
    SPUD_STATIC_DETOUR(m.release->methodPointer, ReleaseHook);
    SPUD_STATIC_DETOUR(m.reload->methodPointer, ReloadHook);
    SPUD_STATIC_DETOUR(m.session->methodPointer, SessionHook);
    SPUD_STATIC_DETOUR(m.load->methodPointer, LoadHook);
    SPUD_STATIC_DETOUR(m.addGeneral->methodPointer, AddGeneralHook);
    active = true;
    try {
      InstallPages();
    } catch (...) {
      pagesActive = false;
      spdlog::warn("[ModSettings] Navigation unavailable; native confirmation control remains available");
    }
    spdlog::info("[ModSettings] Native FC confirmation adapter installed (Windows x64)");
  } catch (...) {
    Warn();
  }
}
#else
void InstallModConfirmationSettings() {}
#endif
