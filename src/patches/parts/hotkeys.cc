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

#include "key.h"
#include "mapkey.h"
#include "utils.h"

#include <iostream>

extern HWND unityWindow;

static bool reset_focus_next_frame = false;
static int  show_info_pending      = 0;

bool force_space_action_next_frame = false;

void     ChangeNavigationSection(SectionID sectionID);
void     ExecuteSpaceAction(FleetBarViewController* fleet_bar);
void     ExecuteRecall(FleetBarViewController* fleet_bar);
HullType GetHullTypeFromBattleTarget(BattleTargetData* context);
void     GotoSection(SectionID sectionID, void* screen_data = nullptr);
bool     CanHideViewers();
bool     DidHideViewers();

void ScreenManager_Update_Hook(auto original, ScreenManager* _this)
{
  if (Config::Get().use_scopely_hotkeys && Config::Get().hotkeys_enabled) {
    return original(_this);
  }

  Key::ResetCache();

  static auto il2cpp_string_new =
      (il2cpp_string_new_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_string_new"));
  static auto GetDeltaTime = il2cpp_resolve_icall<float()>("UnityEngine.Time::get_deltaTime()");

  const auto is_in_chat = Hub::IsInChat();

  int32_t ship_select_request = -1;
  if (MapKey::IsDown(GameFunction::SelectShip1)) {
    ship_select_request = 0;
  } else if (MapKey::IsDown(GameFunction::SelectShip2)) {
    ship_select_request = 1;
  } else if (MapKey::IsDown(GameFunction::SelectShip3)) {
    ship_select_request = 2;
  } else if (MapKey::IsDown(GameFunction::SelectShip4)) {
    ship_select_request = 3;
  } else if (MapKey::IsDown(GameFunction::SelectShip5)) {
    ship_select_request = 4;
  } else if (MapKey::IsDown(GameFunction::SelectShip6)) {
    ship_select_request = 5;
  } else if (MapKey::IsDown(GameFunction::SelectShip7)) {
    ship_select_request = 6;
  } else if (MapKey::IsDown(GameFunction::SelectShip8)) {
    ship_select_request = 7;
  }

  if (ship_select_request != -1 && !Key::IsInputFocused()) {

    if (Key::HasShift()) {
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
        if (!CanHideViewers() && fleet_bar->IsIndexSelected(ship_select_request)) {
          auto fleet = fleet_bar->_fleetPanelController->fleet;
          if (NavigationSectionManager::Instance() && NavigationSectionManager::Instance()->SNavigationManager) {
            NavigationSectionManager::Instance()->SNavigationManager->HideInteraction();
          }
          FleetsManager::Instance()->RequestViewFleet(fleet, true);
        } else {
          fleet_bar->RequestSelect(ship_select_request);
        }
        return;
      }
    }
  }

  if (Key::Pressed(KeyCode::Escape) && (Key::IsInputFocused() || Hub::IsInChat())) {
    // This fixes issues with detecting when an input is selected
    // As the game usually doesn't clear this when using Escape, only when
    // pressing the back button with the mouse...
    return Key::ClearInputFocus();
  }

  if (!is_in_chat) {
    if (!Key::IsInputFocused()) {
      if ((MapKey::IsDown(GameFunction::ShowChat) || MapKey::IsDown(GameFunction::ShowChatSide1)
           || MapKey::IsDown(GameFunction::ShowChatSide2))) {
        if (auto chat_manager = ChatManager::Instance(); chat_manager) {
          if (chat_manager->IsSideChatOpen) {
            if (auto view_controller = ObjectFinder<FullScreenChatViewController>::Get(); view_controller) {
              if (auto message_list = view_controller->_messageList; message_list) {
                if (auto message_field = message_list->_inputField; message_field) {
                  message_field->ActivateInputField();
                }
              }
            }
          } else if (MapKey::IsDown(GameFunction::ShowChatSide1) || MapKey::IsDown(GameFunction::ShowChatSide2)) {
            chat_manager->OpenChannel(ChatChannelCategory::Alliance, ChatViewMode::Side);
          } else {
            chat_manager->OpenChannel(ChatChannelCategory::Alliance, ChatViewMode::Fullscreen);
          }
        }
      } else if (MapKey::IsDown(GameFunction::ShowQTrials)) {
        return GotoSection(SectionID::ChallengeSelection);
      } else if (MapKey::IsDown(GameFunction::ShowBookmarks)) {
        auto bookmark_manager = BookmarksManager::Instance();
        if (bookmark_manager) {
          return bookmark_manager->ViewBookmarks();
        }
        return GotoSection(SectionID::Bookmarks_Main);
      } else if (MapKey::IsDown(GameFunction::ShowRefinery)) {
        return GotoSection(SectionID::Shop_Refining_List);
      } else if (MapKey::IsDown(GameFunction::ShowFactions)) {
        return GotoSection(SectionID::Shop_MainFactions);
      } else if (MapKey::IsDown(GameFunction::ShoWStationExterior)) {
        return GotoSection(SectionID::Starbase_Exterior);
      } else if (MapKey::IsDown(GameFunction::ShowGalaxy)) {
        return ChangeNavigationSection(SectionID::Navigation_Galaxy);
      } else if (MapKey::IsDown(GameFunction::ShowStationInterior)) {
        return GotoSection(SectionID::Starbase_Interior);
      } else if (MapKey::IsDown(GameFunction::ShowSystem)) {
        return ChangeNavigationSection(SectionID::Navigation_System);
      } else if (MapKey::IsDown(GameFunction::ShowInventory)) {
        return GotoSection(SectionID::InventoryList);
      } else if (MapKey::IsDown(GameFunction::ShowMissions)) {
        return GotoSection(SectionID::Missions_AcceptedList);
      } else if (MapKey::IsDown(GameFunction::ShowOfficers)) {
        return GotoSection(SectionID::OfficerInventory);
      } else if (MapKey::IsDown(GameFunction::ShowCommander)) {
        // TODO: Does not work properly, defaults to first FleetCommander (spock, rather than selected fleet
        // commander)
        return GotoSection(SectionID::FleetCommander_Management);
      } else if (MapKey::IsDown(GameFunction::ShowAwayTeam)) {
        return GotoSection(SectionID::Missions_AwayTeamsList);
      } else if (MapKey::IsDown(GameFunction::ShowEvents)) {
        return GotoSection(SectionID::Tournament_Group_Selection);
      } else if (MapKey::IsDown(GameFunction::ShowExoComp)) {
        return GotoSection(SectionID::Consumables);
      } else if (MapKey::IsDown(GameFunction::ShowDaily)) {
        return GotoSection(SectionID::Missions_DailyGoals);
      } else if (MapKey::IsPressed(GameFunction::UiScaleUp)) {
        auto& config    = Config::Get();
        auto  old_scale = config.ui_scale;
        config.ui_scale -= config.ui_scale_adjust;
        spdlog::info("UI has ben scaled up, was {}, now {}", old_scale, config.ui_scale);
      } else if (MapKey::IsPressed(GameFunction::UiScaleDown)) {
        auto& config    = Config::Get();
        auto  old_scale = config.ui_scale;
        config.ui_scale += config.ui_scale_adjust;
        spdlog::info("UI has been scaled down, was {}, now {}", old_scale, config.ui_scale);
      } else if (MapKey::IsDown(GameFunction::ShowShips)) {
        auto fleet_bar        = ObjectFinder<FleetBarViewController>::Get();
        auto fleet_controller = fleet_bar->_fleetPanelController;
        auto fleet            = fleet_bar->_fleetPanelController->fleet;
        if (fleet) {
          fleet_controller->RequestAction(fleet, ActionType::Manage, 0, ActionBehaviour::Default);
        }
      }
    }
  } else {
    if (auto chat_manager = ChatManager::Instance(); chat_manager) {
      if (MapKey::IsDown(GameFunction::SelectChatGlobal)) {
        return chat_manager->OpenChannel(ChatChannelCategory::Global);
      } else if (MapKey::IsDown(GameFunction::SelectChatAlliance)) {
        return chat_manager->OpenChannel(ChatChannelCategory::Alliance);
      } else if (MapKey::IsDown(GameFunction::SelectChatPrivate)) {
        return chat_manager->OpenChannel(ChatChannelCategory::Private);
      }
    }

    if (MapKey::IsDown(GameFunction::ActionView)) {
      if (auto view_controller = ObjectFinder<FullScreenChatViewController>::Get(); view_controller) {
        if (view_controller->_messageList && view_controller->_messageList->_inputField) {
          return view_controller->_messageList->_inputField->ActivateInputField();
        }
      }
    }
  }

  if (!Key::IsInputFocused()) {
    // Lets try to remove the pre-scan because we hit escape and it's visible
    if (Key::Pressed(KeyCode::Escape) && DidHideViewers()) {
      return;
    }

    // Dismiss the golden rewards screen when escape or space is pressed.
    if (MapKey::IsDown(GameFunction::ActionPrimary) || Key::Pressed(KeyCode::Escape)) {
      if (auto reward_controller = ObjectFinder<AnimatedRewardsScreenViewController>::Get(); reward_controller) {
        if (reward_controller->IsActive()) {
          return reward_controller->GoBackToLastSection();
        }
      }
    }

    if (MapKey::IsDown(GameFunction::ActionPrimary) || MapKey::IsDown(GameFunction::ActionSecondary)
        || MapKey::IsDown(GameFunction::ActionRecall) || force_space_action_next_frame) {
      if (Hub::IsInSystemOrGalaxyOrStarbase() && !Hub::IsInChat() && !Key::IsInputFocused()) {
        auto fleet_bar = ObjectFinder<FleetBarViewController>::Get();
        if (fleet_bar) {
          bool was_forced = force_space_action_next_frame;
          ExecuteSpaceAction(fleet_bar);
          if (was_forced) {
            force_space_action_next_frame = false;
          }
        }
      }
    }

    if (MapKey::IsDown(GameFunction::ActionView)) {
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

    // Did we not find a rewards widget in the previous frame?
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
  }
}

// NOTE: If you change this loop functionality, also change DoHideViewersOfType template
template <typename T> inline bool CanHideViewersOfType()
{
  auto widgets = ObjectFinder<T>::GetAll();
  for (auto i = 0; i < widgets->max_length; ++i) {
    auto       widget  = il2cpp_get_array_element<T>(widgets, i);
    const auto visible = widget
                         && (widget->_visibilityController->_state == VisibilityState::Visible
                             || widget->_visibilityController->_state == VisibilityState::Show);
    if (visible) {
      return true;
    }
  }

  return false;
}

bool CanHideViewers()
{
  return CanHideViewersOfType<AllianceStarbaseObjectViewerWidget>() || CanHideViewersOfType<ArmadaObjectViewerWidget>()
         || CanHideViewersOfType<CelestialObjectViewerWidget>() || CanHideViewersOfType<EmbassyObjectViewer>()
         || CanHideViewersOfType<HousingObjectViewerWidget>() || CanHideViewersOfType<MiningObjectViewerWidget>()
         || CanHideViewersOfType<MissionsObjectViewerWidget>() || CanHideViewersOfType<PreScanTargetWidget>()
         || CanHideViewersOfType<HousingObjectViewerWidget>();
}

// NOTE: If you change this loop functionality, also change CanideViewersOfType template
template <typename T> inline bool DidHideViewersOfType()
{
  auto widgets = ObjectFinder<T>::GetAll();
  auto didHide = false;

  for (auto i = 0; i < widgets->max_length; ++i) {
    auto       widget  = il2cpp_get_array_element<T>(widgets, i);
    const auto visible = widget
                         && (widget->_visibilityController->_state == VisibilityState::Visible
                             || widget->_visibilityController->_state == VisibilityState::Show);
    if (visible) {
      widget->HideAllViewers();
      didHide = true;
    }
  }

  return didHide;
}

bool DidHideViewers()
{
  return DidHideViewersOfType<AllianceStarbaseObjectViewerWidget>() || DidHideViewersOfType<ArmadaObjectViewerWidget>()
         || DidHideViewersOfType<CelestialObjectViewerWidget>() || DidHideViewersOfType<EmbassyObjectViewer>()
         || DidHideViewersOfType<HousingObjectViewerWidget>() || DidHideViewersOfType<MiningObjectViewerWidget>()
         || DidHideViewersOfType<MissionsObjectViewerWidget>() || DidHideViewersOfType<PreScanTargetWidget>()
         || DidHideViewersOfType<HousingObjectViewerWidget>();
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

void ExecuteRecall(FleetBarViewController* fleet_bar)
{
  auto fleet_controller = fleet_bar->_fleetPanelController;
  auto fleet            = fleet_bar->_fleetPanelController->fleet;
  auto fleet_state      = fleet->CurrentState;

  auto fleet_id     = fleet->Id;
  auto prev_state   = fleet->PreviousState;
  auto recall_reqs  = fleet->RecallRequirements;
  auto recall_ismet = recall_reqs->IsMet;

  spdlog::info("Recall fleet {} previous {} current {}, ismet {}.", (int)fleet_id, (int)prev_state, (int)fleet_state,
               recall_ismet);

  if (fleet_state == FleetState::IdleInSpace || fleet_state == FleetState::Impulsing
      || fleet_state == FleetState::Mining) {
    if (NavigationSectionManager::Instance() && NavigationSectionManager::Instance()->SNavigationManager) {
      NavigationSectionManager::Instance()->SNavigationManager->HideInteraction();
    }
    fleet_controller->RequestAction(fleet, ActionType::Recall, 0, ActionBehaviour::Default);
  }
}

void ExecuteSpaceAction(FleetBarViewController* fleet_bar)
{
  auto has_primary   = MapKey::IsDown(GameFunction::ActionPrimary) || force_space_action_next_frame;
  auto has_secondary = MapKey::IsDown(GameFunction::ActionSecondary);
  auto has_recall    = MapKey::IsDown(GameFunction::ActionRecall);

  auto fleet_controller = fleet_bar->_fleetPanelController;
  auto fleet            = fleet_controller->fleet;

  if (has_primary && fleet->CurrentState == FleetState::WarpCharging) {
    fleet_controller->CancelWarpClicked();
  } else {
    auto did_pre_scan         = false;
    auto all_pre_scan_widgets = ObjectFinder<PreScanTargetWidget>::GetAll();
    for (auto i = 0; i < all_pre_scan_widgets->max_length && !did_pre_scan; ++i) {
      auto pre_scan_widget = il2cpp_get_array_element<PreScanTargetWidget>(all_pre_scan_widgets, i);
      if (pre_scan_widget
          && (pre_scan_widget->_visibilityController->_state == VisibilityState::Visible
              || pre_scan_widget->_visibilityController->_state == VisibilityState::Show)) {

        if (auto mine_object_viewer_widget = ObjectFinder<MiningObjectViewerWidget>::Get();
            mine_object_viewer_widget
            && (mine_object_viewer_widget->_visibilityController->_state == VisibilityState::Visible
                || mine_object_viewer_widget->_visibilityController->_state == VisibilityState::Show)) {
          if (has_secondary) {
            return pre_scan_widget->_scanEngageButtonsWidget->OnScanButtonClicked();
          } else if (has_primary) {
            return mine_object_viewer_widget->MineClicked();
          }
        } else {
          if (has_secondary) {
            return pre_scan_widget->_scanEngageButtonsWidget->OnScanButtonClicked();
          } else if (has_primary) {
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

              return;
            }
          }
        }
      }
    }

    if (auto mine_object_viewer_widget = ObjectFinder<MiningObjectViewerWidget>::Get();
        mine_object_viewer_widget
        && (mine_object_viewer_widget->_visibilityController->_state == VisibilityState::Visible
            || mine_object_viewer_widget->_visibilityController->_state == VisibilityState::Show)) {
      if (has_secondary) {
        if (mine_object_viewer_widget->_scanEngageButtonsWidget->Context) {
          return mine_object_viewer_widget->_scanEngageButtonsWidget->OnScanButtonClicked();
        }
      } else if (has_primary) {
        return mine_object_viewer_widget->MineClicked();
      }
    } else if (auto star_node_object_viewer_widget = ObjectFinder<StarNodeObjectViewerWidget>::Get();
               star_node_object_viewer_widget && star_node_object_viewer_widget->Context) {
      if (has_secondary) {
        star_node_object_viewer_widget->OnViewButtonActivation();
      } else if (has_primary) {
        star_node_object_viewer_widget->InitiateWarp();
      }
    } else if (auto navigation_ui_controller = ObjectFinder<NavigationInteractionUIViewController>::Get();
               navigation_ui_controller) {
      if (has_recall) {
        spdlog::info("ExecuteRecall() #1");
        return ExecuteRecall(fleet_bar);
      } else if (has_primary) {
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
    } else if (has_recall) {
      spdlog::info("ExecuteRecall() #2");
      return ExecuteRecall(fleet_bar);
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

void InitializeActions_Hook(auto original, void* _this)
{
  if (Config::Get().use_scopely_hotkeys) {
    return original(_this);
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
  auto fleet_type = target_fleet_deployed->FleetType;
  if (fleet_type == DeployedFleetType::Player) {
    return Config::Get().show_player_cargo;
  } else if (fleet_type == DeployedFleetType::Marauder) {
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
  auto ptr_can_user_shortcuts = shortcuts_manager_helper.GetMethodXor("InitializeActions");
  if (!ptr_can_user_shortcuts) {
    return;
  }
  SPUD_STATIC_DETOUR(ptr_can_user_shortcuts, InitializeActions_Hook);

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
