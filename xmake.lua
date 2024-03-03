set_languages("c++23")

add_requires("eastl")
add_requires("spdlog")
add_requires("imgui")
add_requires("protobuf-cpp")
add_requires("toml++")
add_requires("nlohmann_json")

if is_plat("windows") then
    includes("win-proxy-dll")
end

if is_plat("macosx") then
    includes("macos-loader")
end

add_rules("mode.releasedbg")

package("spud")
    add_deps("cmake")
    add_urls("https://github.com/tashcan/spud/archive/refs/tags/$(version).tar.gz",
             "https://github.com/tashcan/spud.git")
    add_versions("v0.1.1", "4298ec14727080166a959051d647a2acfcdceb0170bd1d269c1c76c8e51c1dca")
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DSPUD_BUILD_TESTS=OFF")
        table.insert(configs, "-DSPUD_NO_LTO=ON")
        import("package.tools.cmake").install(package, configs)
    end)
    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            void test() {
                auto memory = spud::alloc_executable_memory(128);
                (void)memory;
            }
        ]]}, {configs = {languages = "c++17"}, includes = "spud/utils.h"}))
    end)
package_end()

package("libil2cpp")
    -- add_urls("https://github.com/tashcan/spud/archive/refs/tags/$(version).tar.gz",
    --          "https://github.com/tashcan/spud.git")
    -- add_versions("v0.1.0", "3d8e9945a30e82eacd1ee81124cdf3f03d31e1f235d77e02682c418c3bae1c2e")
    on_fetch(function (package, opt)
        return {includedirs = path.join(os.scriptdir(), "third_party/libil2cpp")}
    end)
package_end()

add_requires("spud")
add_requires("libil2cpp")

-- includes("launcher")
includes("mods")

