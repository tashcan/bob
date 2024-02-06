cmake_minimum_required(VERSION 3.11)
project(STFC-Community-Patch)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /W4 /permissive- /bigobj")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") 

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
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tomlplusplus)

FetchContent_Declare(
      spud
      GIT_REPOSITORY "https://github.com/tashcan/spud.git"
      GIT_TAG origin/main
      GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(spud)

FetchContent_Declare(
      protobuf
      GIT_REPOSITORY "https://github.com/protocolbuffers/protobuf.git"
      GIT_TAG v3.24.3
      GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(protobuf)

FetchContent_Declare(
    eabase
    GIT_REPOSITORY https://github.com/electronicarts/EABase.git
    GIT_TAG origin/master
    GIT_PROGRESS TRUE
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(eabase)

FetchContent_Declare(
    eastl
    GIT_REPOSITORY https://github.com/electronicarts/EASTL.git
    GIT_TAG origin/master
    GIT_PROGRESS TRUE
    GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(eastl)

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

add_library(stfc-community-patch SHARED ${TARGET_SRC} ${PATCHES} ${PRIME} 
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
 "src/prime/LanguageManager.h")

set_property(TARGET stfc-community-patch PROPERTY CXX_STANDARD 20)
set_property(TARGET stfc-community-patch PROPERTY CXX_STANDARD_REQUIRED ON)

target_sources(stfc-community-patch PRIVATE src/version.rc)

include_directories(stfc-community-patch third_party src src/patches third_party/xorstr/include ${Protobuf_INCLUDE_DIRS})

add_dependencies(stfc-community-patch EASTL)
target_link_libraries(stfc-community-patch PRIVATE spud nlohmann_json::nlohmann_json protobuf::libprotobuf tomlplusplus_tomlplusplus absl::base absl::synchronization absl::strings spdlog EASTL)

add_subdirectory(third_party)
