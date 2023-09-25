#include <spud/detour.h>

#include <il2cpp/il2cpp_helper.h>

#include <prime/EntityGroup.h>
#include <prime/HttpResponse.h>
#include <prime/ServiceResponse.h>

#include <prime/proto/Digit.PrimeServer.Models.pb.h>

#include "config.h"

#include <nlohmann/json.hpp>

#include <string>

void HandleEntityGroup(EntityGroup* entity_group);

void MissionsDataContainer_ParseBinaryObject(auto original, void* _this, EntityGroup* group, bool isPlayerData)
{
  HandleEntityGroup(group);
  return original(_this, group, isPlayerData);
}

void GameServerModelRegistry_ProcessResultInternal(auto original, void* _this, HttpResponse* http_response,
                                                   ServiceResponse* service_response, void* callback,
                                                   void* callback_error)
{
  auto entity_groups = service_response->EntityGroups;
  for (int i = 0; i < entity_groups->Count; ++i) {
    auto entity_group = entity_groups->get_Item(i);
    HandleEntityGroup(entity_group);
  }

  return original(_this, http_response, service_response, callback, callback_error);
}
void GameServerModelRegistry_HandleBinaryObjects(auto original, void* _this, ServiceResponse* service_response)
{
  auto entity_groups = service_response->EntityGroups;
  for (int i = 0; i < entity_groups->Count; ++i) {
    auto entity_group = entity_groups->get_Item(i);
    HandleEntityGroup(entity_group);
  }

  return original(_this, service_response);
}

#include <TlHelp32.h>
#include <Windows.h>
#include <shellapi.h>
#include <wininet.h>

#pragma comment(lib, "Wininet.lib")

static void PostSyncData(std::string post_data)
{
  if (Config::Get().sync_host.empty()) {
    return;
  }

  // initialize WinInet
  HINTERNET hInternet = ::InternetOpen(L"stfc community patch", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  if (hInternet != nullptr) {
    DWORD timeout = 1 * 1000;
    auto  result  = InternetSetOption(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOption(hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOption(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));

    HINTERNET hConnect = ::InternetConnectA(hInternet, Config::Get().sync_host.c_str(), Config::Get().sync_port, 0, 0,
                                            INTERNET_SERVICE_HTTP, 0, 0);
    if (hConnect != nullptr) {
      auto flags = INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD;
      if (Config::Get().sync_port == 443) {
        flags |= INTERNET_FLAG_SECURE;
      }

      HINTERNET hRequest = ::HttpOpenRequestA(hConnect, "POST", "/sync/", 0, 0, 0, flags, 0);
      if (hRequest != nullptr) {
        const auto was_sent = ::HttpSendRequest(hRequest, nullptr, 0, post_data.data(), post_data.size());
        ::InternetCloseHandle(hRequest);
      }
      ::InternetCloseHandle(hConnect);
    }
    ::InternetCloseHandle(hInternet);
  }
}

void HandleEntityGroup(EntityGroup* entity_group)
{
  using json = nlohmann::json;

  auto bytes = entity_group->Group;
  auto type  = entity_group->Type_;
  if (type == EntityGroup::Type::CompletedMissions) {
    auto response = Digit::PrimeServer::Models::CompletedMissionsResponse();
    if (response.ParseFromArray(bytes->bytes->m_Items, bytes->bytes->max_length)) {
      auto mission_array = json::array();
      for (const auto& mission : response.completedmissions()) {
        mission_array.push_back({{"type", "mission"}, {"mid", mission}});
      }
      PostSyncData(mission_array.dump());
    }
  } else if (type == EntityGroup::Type::PlayerInventories) {
    auto response = Digit::PrimeServer::Models::InventoryResponse();
    if (response.ParseFromArray(bytes->bytes->m_Items, bytes->bytes->max_length)) {

      auto inventory_object = json();
      for (const auto& inventory : response.inventories()) {
        for (const auto& item : inventory.second.items()) {
          auto type = item.type();
          if (type == Digit::PrimeServer::Models::INVENTORYITEMTYPE_INVENTORYRESOURCE)
            inventory_object[std::to_string(item.commonparams().refid())] = item.count();
        }
      }
    }
  } else if (type == EntityGroup::Type::ResearchTreesState) {
    auto response = Digit::PrimeServer::Models::ResearchTreesState();
    if (response.ParseFromArray(bytes->bytes->m_Items, bytes->bytes->max_length)) {
      auto research_array = json::array();
      for (const auto& research : response.researchprojectlevels()) {
        research_array.push_back({{"type", "research"}, {"rid", research.first}, {"level", research.second}});
      }
      PostSyncData(research_array.dump());
    }
  } else if (type == EntityGroup::Type::Officers) {
    auto response = Digit::PrimeServer::Models::OfficersResponse();
    if (response.ParseFromArray(bytes->bytes->m_Items, bytes->bytes->max_length)) {
      auto officers_array = json::array();
      for (const auto& officer : response.officers()) {
        officers_array.push_back(
            {{"type", "officer"}, {"oid", officer.id()}, {"rank", officer.rankindex()}, {"level", officer.level()}});
      }
      PostSyncData(officers_array.dump());
    }
  } else if (type == EntityGroup::Type::ForbiddenTechs) {
    auto response = Digit::PrimeServer::Models::ForbiddenTechsResponse();
    if (response.ParseFromArray(bytes->bytes->m_Items, bytes->bytes->max_length)) {
      auto ft_array = json::array();
      for (const auto& ft : response.forbiddentechs()) {
        ft_array.push_back({{"type", "ft"}, {"fid", ft.id()}, {"tier", ft.tier()}, {"level", ft.level()}});
      }
      PostSyncData(ft_array.dump());
    }
  } else if (type == EntityGroup::Type::ActiveOfficerTraits) {
    auto response = Digit::PrimeServer::Models::ActiveOfficerTraits();
    if (response.ParseFromArray(bytes->bytes->m_Items, bytes->bytes->max_length)) {
      auto trait_array = json::array();
      for (const auto& trait : response.activetraits()) {
        trait_array.push_back(
            {{"type", "trait"}, {"oid", response.officerid()}, {"tid", trait.first}, {"level", trait.second.level()}});
      }
      PostSyncData(trait_array.dump());
    }
  } else if (type == EntityGroup::Type::Json) {
    try {
      using json = nlohmann::json;

      auto text   = bytes->bytes->m_Items;
      auto result = json::parse(text);

      if (result.contains("resources")) {
        auto resource_array = json::array();
        for (const auto& resource : result["resources"].get<json::object_t>()) {
          resource_array.push_back(
              {{"type", "resource"}, {"rid", resource.first}, {"amount", resource.second["current_amount"]}});
        }
        PostSyncData(resource_array.dump());
      } else if (result.contains("starbase_modules")) {
        auto starbase_array = json::array();
        for (const auto& resource : result["starbase_modules"].get<json::object_t>()) {
          starbase_array.push_back(
              {{"type", "module"}, {"bid", resource.second["id"]}, {"level", resource.second["level"]}});
        }
        PostSyncData(starbase_array.dump());
      } else if (result.contains("ships")) {
        auto ship_array = json::array();
        for (const auto& resource : result["ships"].get<json::object_t>()) {
          ship_array.push_back({{"type", "ship"},
                                {"psid", resource.second["id"]},
                                {"level", resource.second["level"]},
                                {"tier", resource.second["tier"]},
                                {"hull_id", resource.second["hull_id"]},
                                {"components", resource.second["components"]}});
        }
        PostSyncData(ship_array.dump());
      }
    } catch (...) {
      //
    }
  }
}

void InstallSyncPatches()
{
  auto missions_data_container =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Models", "MissionsDataContainer");
  auto ptr = missions_data_container.GetMethod("ParseBinaryObject");
  SPUD_STATIC_DETOUR(ptr, MissionsDataContainer_ParseBinaryObject);

  auto inventory_data_container =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "InventoryDataContainer");
  ptr = inventory_data_container.GetMethod("ParseBinaryObject");
  SPUD_STATIC_DETOUR(ptr, MissionsDataContainer_ParseBinaryObject);

  auto research_data_container =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "ResearchDataContainer");
  ptr = research_data_container.GetMethod("ParseBinaryObject");
  SPUD_STATIC_DETOUR(ptr, MissionsDataContainer_ParseBinaryObject);

  auto research_service =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Services", "ResearchService");
  ptr = research_service.GetMethod("ParseBinaryObject");
  SPUD_STATIC_DETOUR(ptr, MissionsDataContainer_ParseBinaryObject);

  auto game_server_model_registry =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimeServer.Core", "GameServerModelRegistry");
  ptr = game_server_model_registry.GetMethod("ProcessResultInternal");
  SPUD_STATIC_DETOUR(ptr, GameServerModelRegistry_ProcessResultInternal);
  ptr = game_server_model_registry.GetMethod("HandleBinaryObjects");
  SPUD_STATIC_DETOUR(ptr, GameServerModelRegistry_HandleBinaryObjects);

  auto platform_model_registry =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimePlatform.Core", "PlatformModelRegistry");
  ptr = platform_model_registry.GetMethod("ProcessResultInternal");
  SPUD_STATIC_DETOUR(ptr, GameServerModelRegistry_ProcessResultInternal);
}