cmake_minimum_required(VERSION 3.11)
project(STFC-Community-Patch)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")

# Windows' math include does not define constants by default.
# Set this definition so it does.
# Also set NOMINMAX so the min and max functions are not overwritten with macros.
IF(MSVC)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
ENDIF()

add_definitions(-DJM_XORSTR_DISABLE_AVX_INTRINSICS)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v2.5.0
)
FetchContent_MakeAvailable(tomlplusplus)

FetchContent_Declare(
      spud
      GIT_REPOSITORY "https://github.com/tashcan/spud.git"
      GIT_TAG origin/main
)
FetchContent_MakeAvailable(spud)

FetchContent_Declare(
      protobuf
      GIT_REPOSITORY "https://github.com/protocolbuffers/protobuf.git"
      GIT_TAG v3.24.3
)
FetchContent_MakeAvailable(protobuf)

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
    src/patches/parts/sync.cc
    src/patches/parts/testing.cc
    src/patches/parts/zoom.cc
)

file(GLOB_RECURSE PRIME src/prime/*.cc src/prime/*.h)

add_library(stfc-communty-patch SHARED ${TARGET_SRC} ${PATCHES} ${PRIME} 
    src/version.def
    "src/il2cpp/il2cpp_helper.h"
    "src/prime/TabBarViewController.h"
    "src/prime/IList.h"
    "src/prime/GenericButtonContext.h"
    "src/prime/ChatPreviewController.h"
    "src/prime/NavigationInteractionUIContext.h"
    "src/prime/ViewController.h"
    "src/prime/PreScanTargetWidget.h"
    "src/prime/VisibilityController.h"
    "src/prime/ObjectViewerBaseWidget.h"
    "src/prime/AspectRatioConstraintHandler.h"
    "src/prime/RewardsButtonWidget.h"
    "src/prime/IEnumerator.h"
)

set_property(TARGET stfc-communty-patch PROPERTY CXX_STANDARD 20)
set_property(TARGET stfc-communty-patch PROPERTY CXX_STANDARD_REQUIRED ON)

target_sources(stfc-communty-patch PRIVATE src/version.rc)

include_directories(stfc-communty-patch third_party src src/patches third_party/xorstr/include ${Protobuf_INCLUDE_DIRS})

target_link_libraries(stfc-communty-patch PRIVATE spud nlohmann_json::nlohmann_json protobuf::libprotobuf tomlplusplus_tomlplusplus absl::base absl::synchronization absl::strings spdlog)

add_subdirectory(third_party)
