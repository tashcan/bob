#include "config.h"
#include "prime_types.h"

#include <spud/detour.h>

#include "prime/AllianceStarbaseObjectViewerWidget.h"
#include "prime/AnimatedRewardsScreenViewController.h"
#include "prime/ArmadaObjectViewerWidget.h"
#include "prime/BookmarksManager.h"
#include "prime/CelestialObjectViewerWidget.h"
#include "prime/ChatManager.h"
#include "prime/ChatMessageListLocalViewController.h"
#include "prime/DeploymentManager.h"
#include "prime/EmbassyObjectViewer.h"
#include "prime/EventSystem.h"
#include "prime/FleetBarViewController.h"
#include "prime/FleetLocalViewController.h"
#include "prime/FleetsManager.h"
#include "prime/FullScreenChatViewController.h"
#include "prime/HousingObjectViewerWidget.h"
#include "prime/Hub.h"
#include "prime/KeyCode.h"
#include "prime/MiningObjectViewerWidget.h"
#include "prime/MissionsObjectViewerWidget.h"
#include "prime/NavigationInteractionUIViewController.h"
#include "prime/NavigationSectionManager.h"
#include "prime/PreScanTargetWidget.h"
#include "prime/ScanEngageButtonsWidget.h"
#include "prime/ScanTargetViewController.h"
#include "prime/SceneManager.h"
#include "prime/ScreenManager.h"
#include "prime/StarNodeObjectViewerWidget.h"

#include "utils.h"

#include <iostream>

extern HWND unityWindow;

static bool reset_focus_next_frame = false;
static int  show_info_pending      = 0;

bool     force_space_action_next_frame = false;

void     ChangeNavigationSection(SectionID sectionID);
void     ExecuteSpaceAction(FleetBarViewController* fleet_bar, bool (*GetKeyDownInt)(KeyCode));
HullType GetHullTypeFromBattleTarget(BattleTargetData* context);
void     GotoSection(SectionID sectionID, void* screen_data = nullptr);
bool     DidHideViewers();

void     ScreenManager_Update_Hook(auto original, ScreenManager* _this)
{
  if (Config::Get().use_scopely_hotkeys) {
    return original(_this);
  }

  static auto il2cpp_string_new =
      (il2cpp_string_new_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_string_new"));
  static auto GetKeyInt = il2cpp_resolve_icall<bool(KeyCode)>("UnityEngine.Input::GetKeyInt(UnityEngine.KeyCode)");
  static auto GetKeyDownInt =
      il2cpp_resolve_icall<bool(KeyCode)>("UnityEngine.Input::GetKeyDownInt(UnityEngine.KeyCode)");
  static auto GetDeltaTime = il2cpp_resolve_icall<float()>("UnityEngine.Time::get_deltaTime()");

  auto       section_manager  = Hub::get_SectionManager();
  const auto current_section  = section_manager->CurrentSection;

  const auto is_shift_pressed = GetKeyInt(KeyCode::LeftShift) || GetKeyInt(KeyCode::RightShift);

  const auto is_in_chat = current_section == SectionID::Chat_Private_Message
                          || current_section == SectionID::Chat_Alliance || current_section == SectionID::Chat_Main
                          || current_section == SectionID::Chat_Private_List;

  const auto is_in_system_galaxy =
      current_section == SectionID::Navigation_Galaxy || current_section == SectionID::Navigation_System
      || current_section == SectionID::Starbase_Interior || current_section == SectionID::Starbase_Exterior;

  int32_t ship_select_request = -1;

  if (GetKeyDownInt(KeyCode::Alpha1)) {
    ship_select_request = 0;
  } else if (GetKeyDownInt(KeyCode::Alpha2)) {
    ship_select_request = 1;
  } else if (GetKeyDownInt(KeyCode::Alpha3)) {
    ship_select_request = 2;
  } else if (GetKeyDownInt(KeyCode::Alpha4)) {
    ship_select_request = 3;
  } else if (GetKeyDownInt(KeyCode::Alpha5)) {
    ship_select_request = 4;
  } else if (GetKeyDownInt(KeyCode::Alpha6)) {
    ship_select_request = 5;
  } else if (GetKeyDownInt(KeyCode::Alpha7)) {
    ship_select_request = 6;
  } else if (GetKeyDownInt(KeyCode::Alpha8)) {
    ship_select_request = 7;
  }

  auto is_input_focused = []() {
    bool is_input_focused = false;
    auto eventSystem      = EventSystem::current();
    if (eventSystem) {
      auto n = eventSystem->currentSelectedGameObject;
      if (!n) {
        return false;
      }
      try {
        if (n) {
          auto n2 = n->GetComponentFastPath2<TMP_InputField>();
          if (n2) {
            return n2->isFocused;
          }
        }
      } catch (...) {
        return false;
      }
    }
    return false;
  };

  if (ship_select_request != -1 && !is_input_focused()) {

    if (is_shift_pressed) {
      FleetPlayerData* foundDisco = nullptr;
      for (int discoIdx = 0; discoIdx < 10; ++discoIdx) {
        auto fleetPlayerData = FleetsManager::Instance()->GetFleetPlayerData(discoIdx);
        if (fleetPlayerData && fleetPlayerData->Hull && fleetPlayerData->Hull->Id == 1307832955) {
          foundDisco = fleetPlayerData;
          break;
        }
      }

      if (foundDisco) {
        auto towedFleetId = FleetsManager::Instance()->GetFleetPlayerData(ship_select_request)->Id;
        auto plannedCourse =
            DeploymentManger::Instance()->PlanCourse(FleetsManager::Instance()->GetFleetPlayerData(ship_select_request),
                                                     foundDisco->Address, Vector3::zero(), nullptr, nullptr, nullptr);
        while (plannedCourse->MoveNext()) {
          ;
        }
        DeploymentManger::Instance()->SetTowRequest(towedFleetId, foundDisco->Id);
      }
    } else {
      auto fleet_bar = ObjectFinder<FleetBarViewController>::Get();
      if (fleet_bar) {
        if (fleet_bar->IsIndexSelected(ship_select_request)) {
          auto fleet = fleet_bar->_fleetPanelController->fleet;
          if (NavigationSectionManager::Instance() && NavigationSectionManager::Instance()->SNavigationManager) {
            NavigationSectionManager::Instance()->SNavigationManager->HideInteraction();
          }
          FleetsManager::Instance()->RequestViewFleet(fleet, true);
        } else {
          fleet_bar->RequestSelect(ship_select_request);
        }
      }
    }
  }

  if (GetKeyDownInt(KeyCode::Escape)) {
    // This fixes issues with detecting when an input is selected
    // As the game usually doesn't clear this when using Escape, only when
    // pressing the back button with the mouse...
    try {
      if (auto eventSystem = EventSystem::current(); eventSystem) {
        if (auto n = eventSystem->currentSelectedGameObject; n) {
          auto n2 = n->GetComponentFastPath2<TMP_InputField>();
          if (n2 && n2->isFocused) {
            eventSystem->SetSelectedGameObject(nullptr);
            return;
          }
        }
      }
    } catch (...) {
      //
    }
  }

  if (!is_in_chat) {
    if (!is_input_focused()) {
      if ((GetKeyDownInt(KeyCode::C) || GetKeyDownInt(KeyCode::BackQuote))) {
        if (auto chat_manager = ChatManager::Instance(); chat_manager) {
          if (chat_manager->IsSideChatOpen) {
            auto view_controller = ObjectFinder<FullScreenChatViewController>::Get();
            view_controller->_messageList->_inputField->ActivateInputField();
          } else if (GetKeyInt(KeyCode::LeftAlt) || GetKeyInt(KeyCode::RightAlt) || GetKeyDownInt(KeyCode::BackQuote)) {
            chat_manager->OpenChannel(ChatChannelCategory::Alliance, ChatViewMode::Side);
          } else {
            chat_manager->OpenChannel(ChatChannelCategory::Alliance, ChatViewMode::Fullscreen);
          }
        }
      } else if (GetKeyDownInt(KeyCode::Q) && is_shift_pressed) {
        return GotoSection(SectionID::ChallengeSelection);
      } else if (GetKeyDownInt(KeyCode::B)) {
        auto bookmark_manager = BookmarksManager::Instance();
        if (bookmark_manager) {
          bookmark_manager->ViewBookmarks();
          return;
        }
        return GotoSection(SectionID::Bookmarks_Main);
      } else if (GetKeyDownInt(KeyCode::F)) {
        if (is_shift_pressed) {
          return GotoSection(SectionID::Shop_Refining_List);
        } else {
          return GotoSection(SectionID::Shop_MainFactions);
        }
      } else if (GetKeyDownInt(KeyCode::G)) {
        if (is_shift_pressed) {
          return GotoSection(SectionID::Starbase_Exterior);
        } else {
          return ChangeNavigationSection(SectionID::Navigation_Galaxy);
        }
      } else if (GetKeyDownInt(KeyCode::H)) {
        if (is_shift_pressed) {
          return GotoSection(SectionID::Starbase_Interior);
        } else {
          return ChangeNavigationSection(SectionID::Navigation_System);
        }
      } else if (GetKeyDownInt(KeyCode::I)) {
        return GotoSection(SectionID::InventoryList);
      } else if (GetKeyDownInt(KeyCode::M)) {
        return GotoSection(SectionID::Missions_AcceptedList);
      } else if (GetKeyDownInt(KeyCode::O)) {
        if (is_shift_pressed) {
          return GotoSection(SectionID::OfficerInventory);
        } else {
          // TODO: Does not work properly, defaults to first FleetCommander (spock, rather than selected fleet commander)
          return GotoSection(SectionID::FleetCommander_Showcase);
        }
      } else if (GetKeyDownInt(KeyCode::T)) {
        if (is_shift_pressed) {
          return GotoSection(SectionID::Missions_AwayTeamsList);
        } else {
          return GotoSection(SectionID::Tournament_Group_Selection);
        }
      } else if (GetKeyDownInt(KeyCode::X)) {
        return GotoSection(SectionID::Consumables);
      } else if (GetKeyDownInt(KeyCode::Z)) {
        return GotoSection(SectionID::Missions_DailyGoals);
      }
    }
  } else {
    if (GetKeyInt(KeyCode::LeftControl)) {
      if (auto chat_manager = ChatManager::Instance(); chat_manager) {
        if (GetKeyDownInt(KeyCode::Alpha1)) {
          return chat_manager->OpenChannel(ChatChannelCategory::Global);
        } else if (GetKeyDownInt(KeyCode::Alpha2)) {
          return chat_manager->OpenChannel(ChatChannelCategory::Alliance);
        } else if (GetKeyDownInt(KeyCode::Alpha3)) {
          return chat_manager->OpenChannel(ChatChannelCategory::Private);
        }
      }
    } else if (GetKeyDownInt(KeyCode::V) && !is_input_focused()) {
      if (auto view_controller = ObjectFinder<FullScreenChatViewController>::Get(); view_controller) {
        if (view_controller->_messageList && view_controller->_messageList->_inputField) {
          return view_controller->_messageList->_inputField->ActivateInputField();
        }
      }
    }
  }

  // Dismiss the golden rewards screen when escape or space is pressed.
  if (GetKeyDownInt(KeyCode::Space) || GetKeyDownInt(KeyCode::Escape)) {
    if (auto reward_controller = ObjectFinder<AnimatedRewardsScreenViewController>::Get(); reward_controller) {
      if (reward_controller->IsActive()) {
        return reward_controller->GoBackToLastSection();
      }
    }
  }

  if (GetKeyDownInt(KeyCode::Space) || GetKeyDownInt(KeyCode::R) || force_space_action_next_frame) {
    if (is_in_system_galaxy && !is_in_chat && !is_input_focused()) {
      auto fleet_bar = ObjectFinder<FleetBarViewController>::Get();
      if (fleet_bar) {
        bool was_forced = force_space_action_next_frame;
        ExecuteSpaceAction(fleet_bar, GetKeyDownInt);
        if (was_forced) {
          force_space_action_next_frame = false;
        }
      }
    }
  }

  if (GetKeyDownInt(KeyCode::V)) {
    auto all_pre_scan_widgets = ObjectFinder<PreScanTargetWidget>::GetAll();
    for (auto i = 0; i < all_pre_scan_widgets->max_length; ++i) {
      auto pre_scan_widget = il2cpp_get_array_element<PreScanTargetWidget>(all_pre_scan_widgets, i);
      if (pre_scan_widget
          && (pre_scan_widget->_visibilityController->_state == VisibilityState::Visible
              || pre_scan_widget->_visibilityController->_state == VisibilityState::Show)) {
        auto rewardsWidget = pre_scan_widget->_rewardsButtonWidget;
        if (rewardsWidget->_rewardsController->_state != VisibilityState::Visible
            && rewardsWidget->_rewardsController->_state != VisibilityState::Show) {
          show_info_pending = 5;
        } else {
          rewardsWidget->_rewardsController->Hide();
        }
      }
    }
  }

  if (show_info_pending > 0) {
    auto all_pre_scan_widgets = ObjectFinder<PreScanTargetWidget>::GetAll();
    for (auto i = 0; i < all_pre_scan_widgets->max_length; ++i) {
      auto       pre_scan_widget  = il2cpp_get_array_element<PreScanTargetWidget>(all_pre_scan_widgets, i);
      const auto pre_scan_visible = pre_scan_widget
                                    && (pre_scan_widget->_visibilityController->_state == VisibilityState::Visible
                                        || pre_scan_widget->_visibilityController->_state == VisibilityState::Show);
      if (pre_scan_visible) {
        auto       rewardsWidget          = pre_scan_widget->_rewardsButtonWidget;
        const auto rewards_widget_visible = rewardsWidget->_rewardsController->_state == VisibilityState::Visible
                                            || rewardsWidget->_rewardsController->_state == VisibilityState::Show;
        if (!rewards_widget_visible) {
          rewardsWidget->_rewardsController->Show(true);
        }
      }
    }
    show_info_pending -= 1;
  }

  // Lets try to remove the pre-scan because we hit escape and it's visible
  if (GetKeyDownInt(KeyCode::Escape)) {
    if (DidHideViewers()) {
      return;
    }
  }

  // {
  //  auto all_pre_scan_widgets = ObjectFinder<PreScanTargetWidget>::GetAll();
  //  for (auto i = 0; i < all_pre_scan_widgets->max_length; ++i) {
  //    auto pre_scan_widget = il2cpp_get_array_element<PreScanTargetWidget>(all_pre_scan_widgets, i);
  //    if (pre_scan_widget
  //        && (pre_scan_widget->_visibilityController->_state == VisibilityState::Visible
  //            || pre_scan_widget->_visibilityController->_state == VisibilityState::Show)) {
  //      pre_scan_widget->_rewardsButtonWidget->RewardsClicked();
  //    }
  //  }
  //}

  // Config::Get().Load();

  original(_this);
}

template <typename T> inline bool DidHideViewersOfType() {
  auto widgets = ObjectFinder<T>::GetAll();
  for (auto i = 0; i < widgets->max_length; ++i) {
    auto       widget  = il2cpp_get_array_element<T>(widgets, i);
    const auto visible = widget
        && (widget->_visibilityController->_state == VisibilityState::Visible
        || widget->_visibilityController->_state == VisibilityState::Show);
    if (visible) {
      widget->HideAllViewers();
      return true;
    }
  }

  return false;
}

bool DidHideViewers() {
  return
    DidHideViewersOfType<AllianceStarbaseObjectViewerWidget>() ||
    DidHideViewersOfType<ArmadaObjectViewerWidget>() ||
    DidHideViewersOfType<CelestialObjectViewerWidget>() ||
    DidHideViewersOfType<EmbassyObjectViewer>() ||
    DidHideViewersOfType<HousingObjectViewerWidget>() ||
    DidHideViewersOfType<MiningObjectViewerWidget>() ||
    DidHideViewersOfType<MissionsObjectViewerWidget>() ||
    DidHideViewersOfType<PreScanTargetWidget>() ||
    DidHideViewersOfType<HousingObjectViewerWidget>();
}

void GotoSection(SectionID sectionID, void* section_data)
{
  Hub::get_SectionManager()->TriggerSectionChange(sectionID, section_data, false, false, true);
}

void ChangeNavigationSection(SectionID sectionID)
{
  const auto section_data = Hub::get_SectionManager()->_sectionStorage->GetState(sectionID);

  if (section_data) {
    GotoSection(sectionID, section_data);
  } else {
    NavigationSectionManager::ChangeNavigationSection(sectionID);
  }
}

void ExecuteSpaceAction(FleetBarViewController* fleet_bar, bool (*GetKeyDownInt)(KeyCode))
{
  auto fleet_local_controller = fleet_bar->_fleetPanelController;
  auto fleet                  = fleet_bar->_fleetPanelController->fleet;
  if (fleet->CurrentState == FleetState::WarpCharging) {
    fleet_local_controller->CancelWarpClicked();
  } else {
    auto did_pre_scan         = false;
    auto all_pre_scan_widgets = ObjectFinder<PreScanTargetWidget>::GetAll();
    for (auto i = 0; i < all_pre_scan_widgets->max_length && !did_pre_scan; ++i) {
      auto pre_scan_widget = il2cpp_get_array_element<PreScanTargetWidget>(all_pre_scan_widgets, i);
      if (pre_scan_widget
          && (pre_scan_widget->_visibilityController->_state == VisibilityState::Visible
              || pre_scan_widget->_visibilityController->_state == VisibilityState::Show)) {
        did_pre_scan = true;
        if (auto mine_object_viewer_widget = ObjectFinder<MiningObjectViewerWidget>::Get();
            mine_object_viewer_widget
            && (mine_object_viewer_widget->_visibilityController->_state == VisibilityState::Visible
                || mine_object_viewer_widget->_visibilityController->_state == VisibilityState::Show)) {
          if (GetKeyDownInt(KeyCode::R)) {
            pre_scan_widget->_scanEngageButtonsWidget->OnScanButtonClicked();
          } else {
            mine_object_viewer_widget->MineClicked();
          }
        } else {
          if (GetKeyDownInt(KeyCode::R)) {
            pre_scan_widget->_scanEngageButtonsWidget->OnScanButtonClicked();
          } else {
            auto armada_object_viewer_widget = ObjectFinder<ArmadaObjectViewerWidget>::Get();
            if (!armada_object_viewer_widget
                || (armada_object_viewer_widget->_visibilityController->_state != VisibilityState::Visible
                    && armada_object_viewer_widget->_visibilityController->_state != VisibilityState::Show)) {
              auto context = pre_scan_widget->_scanEngageButtonsWidget->Context;
              auto type    = GetHullTypeFromBattleTarget(context);

              // Try once more in X frames if we get ANY
              // in-case of failed to navgitate error?
              if (type != HullType::ArmadaTarget && (type != HullType::Any || force_space_action_next_frame)) {
                pre_scan_widget->_scanEngageButtonsWidget->OnEngageButtonClicked();
              } else if (type == HullType::Any) {
                force_space_action_next_frame = true;
              }
            }
          }
        }
      }
    }

    if (did_pre_scan) {
      return;
    }

    if (auto mine_object_viewer_widget = ObjectFinder<MiningObjectViewerWidget>::Get();
        mine_object_viewer_widget
        && (mine_object_viewer_widget->_visibilityController->_state == VisibilityState::Visible
            || mine_object_viewer_widget->_visibilityController->_state == VisibilityState::Show)) {
      if (GetKeyDownInt(KeyCode::R)) {
        if (mine_object_viewer_widget->_scanEngageButtonsWidget->Context) {
          mine_object_viewer_widget->_scanEngageButtonsWidget->OnScanButtonClicked();
        }
      } else {
        mine_object_viewer_widget->MineClicked();
      }
    } else if (auto star_node_object_viewer_widget = ObjectFinder<StarNodeObjectViewerWidget>::Get();
               star_node_object_viewer_widget && star_node_object_viewer_widget->Context) {
      if (GetKeyDownInt(KeyCode::R)) {
        star_node_object_viewer_widget->OnViewButtonActivation();
      } else {
        star_node_object_viewer_widget->InitiateWarp();
      }
    } else if (auto navigation_ui_controller = ObjectFinder<NavigationInteractionUIViewController>::Get();
               navigation_ui_controller) {
      if (GetKeyDownInt(KeyCode::R)) {
        auto fleet = fleet_bar->_fleetPanelController->fleet;
        if (NavigationSectionManager::Instance() && NavigationSectionManager::Instance()->SNavigationManager) {
          NavigationSectionManager::Instance()->SNavigationManager->HideInteraction();
        }
        fleet_local_controller->RequestAction(fleet, ActionType::Recall, 0, ActionBehaviour::Default);
      } else {
        if (auto armada_object_viewer_widget = ObjectFinder<ArmadaObjectViewerWidget>::Get();
            armada_object_viewer_widget
            && (armada_object_viewer_widget->_visibilityController->_state == VisibilityState::Visible
                || armada_object_viewer_widget->_visibilityController->_state == VisibilityState::Show)) {
          auto button = armada_object_viewer_widget->__get__joinContext();
          if (button && button->Interactable) {
            armada_object_viewer_widget->ValidateThenJoinArmada();
          }
        } else {
          navigation_ui_controller->OnSetCourseButtonClick();
        }
      }
    } else {
      if (GetKeyDownInt(KeyCode::R)) {
        auto fleet = fleet_bar->_fleetPanelController->fleet;
        if (NavigationSectionManager::Instance() && NavigationSectionManager::Instance()->SNavigationManager) {
          NavigationSectionManager::Instance()->SNavigationManager->HideInteraction();
        }

        fleet_local_controller->RequestAction(fleet, ActionType::Recall, 0, ActionBehaviour::Default);
      }
    }
  }
}

HullType GetHullTypeFromBattleTarget(BattleTargetData* context)
{
  if (!context) {
    return HullType::Any;
  }
  auto deployed_data = context->TargetFleetDeployedData;
  if (!deployed_data) {
    return HullType::Any;
  }
  auto hull_spec = deployed_data->Hull;
  if (!hull_spec) {
    return HullType::Any;
  }
  return hull_spec->Type;
}

void ChatMessageListLocalViewController_AboutToShow_Hook(ChatMessageListLocalViewController* _this);
decltype(ChatMessageListLocalViewController_AboutToShow_Hook)* oChatMessageListLocalViewController_AboutToShow =
    nullptr;
void ChatMessageListLocalViewController_AboutToShow_Hook(ChatMessageListLocalViewController* _this)
{
  oChatMessageListLocalViewController_AboutToShow(_this);
  if (_this->_inputField) {
    _this->_inputField->SendOnFocus();
  }
}

bool get_CanUseShortcuts_Hook(auto original, void* _this)
{
  if (Config::Get().use_scopely_hotkeys && Config::Get().hotkeys_enabled) {
    return original(_this);
  } else {
    return false;
  }
}

bool CheckShowCargo(RewardsButtonWidget* widget)
{
  if (!Config::Get().show_cargo_default) {
    return false;
  }

  if (!widget->Context) {
    return false;
  }

  const auto target_fleet_deployed = widget->Context->TargetFleetDeployedData;

  if (!target_fleet_deployed) {
    return Config::Get().show_station_cargo;
  }
  auto fleetType = target_fleet_deployed->FleetType;
  if (target_fleet_deployed->FleetType == DeployedFleetType::Player) {
    return Config::Get().show_player_cargo;
  } else if (target_fleet_deployed->FleetType == DeployedFleetType::Marauder) {
    if (auto hull = target_fleet_deployed->Hull; hull && hull->Type == HullType::ArmadaTarget) {
      return Config::Get().show_armada_cargo;
    } else {
      return Config::Get().show_hostile_cargo;
    }
  }

  return false;
}

void OnDidBindContext_Hook(auto original, RewardsButtonWidget* _this)
{
  auto pre_state = _this->_rewardsController->_state;
  pre_state      = pre_state;
  original(_this);

  auto post_state = _this->_rewardsController->_state;
  post_state      = post_state;
  if (CheckShowCargo(_this)) {
    _this->_rewardsController->Show(true);
    show_info_pending = 1;
  }
}

void ShowWithFleet_Hook(auto original, PreScanTargetWidget* _this, void* a1)
{
  original(_this, a1);
  auto rewards_button_widget = _this->_rewardsButtonWidget;
  if (CheckShowCargo(rewards_button_widget)) {
    rewards_button_widget->_rewardsController->Show(true);
    show_info_pending = 1;
  }
}

void InstallHotkeyHooks()
{
  auto shortcuts_manager_helper =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.GameInput", "ShortcutsManager");
  auto ptr_can_user_shortcuts = shortcuts_manager_helper.GetMethodXor("get_CanUseShortcuts");
  if (!ptr_can_user_shortcuts) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr_can_user_shortcuts, get_CanUseShortcuts_Hook);

  if (Config::Get().hotkeys_enabled == false) {
    return;
  }

  if (Config::Get().use_scopely_hotkeys == false) {
    auto screen_manager_helper = il2cpp_get_class_helper("Assembly-CSharp", "Digit.Client.UI", "ScreenManager");
    auto ptr_update            = screen_manager_helper.GetMethodXor("Update");
    if (!ptr_update) {
      return;
    }
    SPUD_STATIC_DETOUR(ptr_update, ScreenManager_Update_Hook);
  }

  static auto rewards_button_widget =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Combat", "RewardsButtonWidget");
  auto on_did_bind_context_ptr = rewards_button_widget.GetMethod("OnDidBindContext");

  on_did_bind_context_ptr = on_did_bind_context_ptr;

  SPUD_STATIC_DETOUR(on_did_bind_context_ptr, OnDidBindContext_Hook);

  static auto pre_scan_target_widget =
      il2cpp_get_class_helper("Assembly-CSharp", "Digit.Prime.Combat", "PreScanTargetWidget");
  auto show_with_fleet_ptr = pre_scan_target_widget.GetMethod("ShowWithFleet");
  show_with_fleet_ptr      = show_with_fleet_ptr;
  SPUD_STATIC_DETOUR(show_with_fleet_ptr, ShowWithFleet_Hook);
}