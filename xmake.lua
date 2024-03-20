set_languages("c++23")

set_arch("x86_64")

includes("xmake-packages")

add_requires("eastl2")
add_requires("spdlog")
add_requires("protobuf-cpp2 26.0")
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
add_requires("simdutf2 v5.0.0")

-- includes("launcher")
includes("mods")
