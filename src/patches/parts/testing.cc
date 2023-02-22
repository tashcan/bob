#include "prime_types.h"

#include <spud/detour.h>

#include "utils.h"

#include "prime/BlurController.h"
#include "prime/CallbackContainer.h"
#include "prime/ChatManager.h"
#include "prime/ChatMessageListLocalViewController.h"
#include "prime/ClientModifierType.h"
#include "prime/FleetBarViewController.h"
#include "prime/FleetLocalViewController.h"
#include "prime/FullScreenChatViewController.h"
#include "prime/Hub.h"
#include "prime/InventoryForPopup.h"
#include "prime/KeyCode.h"
#include "prime/MiningObjectViewerWidget.h"
#include "prime/NavigationInteractionUIViewController.h"
#include "prime/ScanEngageButtonsWidget.h"
#include "prime/ScreenManager.h"

#include <il2cpp/il2cpp_helper.h>

#include <chrono>
#include <iostream>

static int i = 0;

bool                                     BlurController_IsTweeningBlur(BlurController* _this);
decltype(BlurController_IsTweeningBlur)* oIsTweeningBlur = nullptr;
bool                                     BlurController_IsTweeningBlur(BlurController* _this3)
{
  return false;
}

int64_t                       BlurControllerCtor(__int64 a1, float a2, float a3, __int64 a4, __int64 a5, __int64 a6);
decltype(BlurControllerCtor)* oBlurControllerCtor = nullptr;
int64_t                       BlurControllerCtor(__int64 a1, float a2, float a3, __int64 a4, __int64 a5, __int64 a6)
{
  return oBlurControllerCtor(a1, a2, a3, a4, a5, a6);
}

__int64                    raise_exception(__int64 a1, __int64 a2, __int64 a3, __int64 a4);
decltype(raise_exception)* oraise_exception = nullptr;
__int64                    raise_exception(__int64 a1, __int64 a2, __int64 a3, __int64 a4)
{
  return oraise_exception(a1, a2, a3, a4);
}

void                                   OnDidChangeSelectedTab_Hook(FullScreenChatViewController* _this);
decltype(OnDidChangeSelectedTab_Hook)* oOnDidChangeSelectedTab = nullptr;
void                                   OnDidChangeSelectedTab_Hook(FullScreenChatViewController* _this)
{
  oOnDidChangeSelectedTab(_this);

  if (_this->_messageList->_inputField) {
    _this->_messageList->_inputField->ActivateInputField();
  }
}

void                             ShopListDirector_Hook(void* _this, void* a1);
decltype(ShopListDirector_Hook)* oShopListDirector = nullptr;
void                             ShopListDirector_Hook(void* _this, void* a1)
{
  return oShopListDirector(_this, a1);
}

void                                 FactionsListDirector_Hook(void* _this, void* a1, void* a2);
decltype(FactionsListDirector_Hook)* oFactionsListDirector = nullptr;
void                                 FactionsListDirector_Hook(void* _this, void* a1, void* a2)
{
  return oFactionsListDirector(_this, a1, a2);
}

// OnBundlesReceived

//
// void
// ChatMessageListLocalViewController_LateUpdate_Hook(ChatMessageListLocalViewController*
// _this); decltype(ChatMessageListLocalViewController_LateUpdate_Hook)*
// oChatMessageListLocalViewController_LateUpdate = nullptr; void
// ChatMessageListLocalViewController_LateUpdate_Hook(ChatMessageListLocalViewController*
// _this)
//{
//	oChatMessageListLocalViewController_LateUpdate(_this);
//	static auto cat = _this->CanvasContext->_category;
//	auto n = _this->CanvasContext->_category;
//	n = n;
//	if (cat != _this->CanvasContext->_category && _this->_inputField) {
//		_this->_inputField->ActivateInputField();
//		cat = _this->CanvasContext->_category;
//	}
//}

LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter_Hook(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
  return nullptr;
  return SetUnhandledExceptionFilter(lpTopLevelExceptionFilter);
}

void RequestDispatcherBase_SetDefaultHeader(void* _this, Il2CppString* key, Il2CppString* value);
decltype(RequestDispatcherBase_SetDefaultHeader)* oRequestDispatcherBase_SetDefaultHeader = nullptr;
void RequestDispatcherBase_SetDefaultHeader(void* _this, Il2CppString* key, Il2CppString* value)
{
  static auto il2cpp_string_new =
      (il2cpp_string_new_t)(GetProcAddress(GetModuleHandle("GameAssembly.dll"), "il2cpp_string_new"));
  oRequestDispatcherBase_SetDefaultHeader(_this, key, value);
}

void                             Chat_handleNewMessage(void* _this, Il2CppString* message);
decltype(Chat_handleNewMessage)* oChatMessage = nullptr;

void Chat_handleNewMessage(void* _this, Il2CppString* message)
{
  wprintf(L"%s\n", message->chars);
  return oChatMessage(_this, message);
  // original(_this, message);
}

void InstallTestPatches()
{
  auto chat_service =
      il2cpp_get_class_helper("Digit.Client.PrimeLib.Runtime", "Digit.PrimePlatform.Services", "ChatService");
  auto ptr = chat_service.GetMethod("HandleMessageReceived");
  // oChatMessage = SPUD_STATIC_DETOUR(ptr, Chat_handleNewMessage);
  return;
}