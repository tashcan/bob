cmake_minimum_required(VERSION 3.11)
project(STFC-Community-Patch)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

# Windows' math include does not define constants by default.
# Set this definition so it does.
# Also set NOMINMAX so the min and max functions are not overwritten with macros.
IF(MSVC)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
ENDIF()

add_definitions(-DJM_XORSTR_DISABLE_AVX_INTRINSICS)

include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v2.5.0
)
FetchContent_MakeAvailable(tomlplusplus)

include(FetchContent)

# FetchContent_Declare(
#       spud
#       GIT_REPOSITORY "https://github.com/tashcan/spud.git"
#       GIT_TAG origin/main
# )

# FetchContent_MakeAvailable(spud)

SET(TARGET_H
    Header.h
    Plugin.h
    messagelog.h
    win32application.h
    timer.h
)

# Local source files here
SET(TARGET_SRC
    src/main.cc
    src/version.cc
    src/utils.cc
    src/il2cpp/il2cpp_helper.cc
    src/patches/config.cc
    src/patches/patches.cc
)

SET(ABSL_PROPAGATE_CXX_STD TRUE)

SET(PATCHES
src/patches/parts/buff_fixes.cc
src/patches/parts/chat.cc
src/patches/parts/disable_banners.cc
src/patches/parts/fix_pan.cc
src/patches/parts/fix_unity_web_request.cc
src/patches/parts/free_resize.cc
src/patches/parts/hotkeys.cc
src/patches/parts/improve_responsiveness.cc
src/patches/parts/ui_scale.cc
src/patches/parts/misc.cc
src/patches/parts/testing.cc
src/patches/parts/zoom.cc
)

SET(PRIME
    "src/prime/NavigationZoom.h"
    "src/prime/TKTouch.h"
    "src/prime/NavigationPan.h"
    "src/prime/Vector2.h"
    "src/prime/HttpRequest.h"
    "src/prime/HttpResponse.h"
    "src/prime/HttpJob.h"
    "src/prime/UnityWebRequest.h"
    "src/prime/FleetBarViewController.h"
    "src/prime/KeyCode.h"
    "src/prime/ChatManager.h"
    "src/prime/MonoSingleton.h"
    "src/prime/Hub.h"
    "src/prime/ScanEngageButtonsWidget.h"
    "src/prime/NavigationInteractionUIViewController.h"
    "src/prime/FleetLocalViewController.h"
    "src/prime/TransitionManager.h"
    "src/prime/BlurController.h"
    "src/prime/ScreenManager.h"
    "src/prime/CanvasScaler.h"
    "src/prime/Camera.h"
    "src/prime/MiningObjectViewerWidget.h"
    "src/prime/ChatMessageListLocalViewController.h"
    "src/prime/TMP_InputField.h"
    "src/prime/FullScreenChatViewController.h"
    "src/prime/FleetPlayerData.h"
    "src/prime/StateContainer.h"
    "src/prime/StarNodeObjectViewerWidget.h"
    "src/prime/FleetsManager.h"
    "src/prime/NavigationSectionManager.h"
    "src/prime/NavigationManager.h"
    "src/prime/SceneArgs.h"
    "src/prime/NavigationDirector.h"
    "src/prime/SectionManager.h"
    "src/prime/EventSystem.h"
    "src/prime/GameObject.h"
    "src/prime/SceneManager.h"
    "src/prime/ArmadaObjectViewerWidget.h"
    "src/prime/Widget.h"
    "src/prime/BattleTargetData.h"
    "src/prime/IBuffData.h"
    "src/prime/ClientModifierType.h"
    "src/prime/InventoryForPopup.h"
    "src/prime/CallbackContainer.h"
)

add_library(stfc-communty-patch SHARED ${TARGET_SRC} ${PATCHES} ${PRIME} src/version.def "src/il2cpp/il2cpp_helper.h" "src/prime/TabBarViewController.h" "src/prime/IList.h" "src/prime/GenericButtonContext.h" "src/prime/ChatPreviewController.h" "src/prime/NavigationInteractionUIContext.h" "src/prime/ViewController.h" "src/prime/PreScanTargetWidget.h" "src/prime/VisibilityController.h" "src/prime/ObjectViewerBaseWidget.h" "src/prime/AspectRatioConstraintHandler.h" "src/prime/RewardsButtonWidget.h" "src/prime/IEnumerator.h")
set_property(TARGET stfc-communty-patch PROPERTY CXX_STANDARD 20)
set_property(TARGET stfc-communty-patch PROPERTY CXX_STANDARD_REQUIRED ON)

target_sources(stfc-communty-patch PRIVATE src/version.rc)

include_directories(stfc-communty-patch third_party src src/patches third_party/xorstr/include)

target_link_libraries(stfc-communty-patch PRIVATE spud tomlplusplus_tomlplusplus absl::base absl::synchronization absl::strings spdlog)

add_subdirectory(third_party)
