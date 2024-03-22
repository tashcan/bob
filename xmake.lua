set_project("stfc-community-patch")

set_languages("c++23")

add_requires("eastl")
add_requires("spdlog")
add_requires("protobuf-cpp")
add_requires("toml++")
add_requires("nlohmann_json")

if is_plat("windows") then
    includes("win-proxy-dll")
end

if is_plat("macosx") then
    includes("macos-dylib")
    includes("macos-loader")
end

add_rules("mode.debug")
add_rules("mode.releasedbg")


package("libil2cpp")
on_fetch(function(package, opt)
    return { includedirs = path.join(os.scriptdir(), "third_party/libil2cpp") }
end)
package_end()

add_requires("spud v0.2.0.alpha.4")
add_requires("libil2cpp")
add_requires("cpp-httplib")
add_requires("simdutf")

-- includes("launcher")
includes("mods")


add_repositories("stfc-community-patch-repo xmake-packages")