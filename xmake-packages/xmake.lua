package("spud")
do
    add_deps("cmake")
    add_urls("https://github.com/tashcan/spud/archive/refs/tags/$(version).tar.gz",
        "https://github.com/tashcan/spud.git")
    add_versions("v0.1.1", "4298ec14727080166a959051d647a2acfcdceb0170bd1d269c1c76c8e51c1dca")
    add_versions("v0.2.0.alpha.1", "30df1499b5e4a51ae7ddd9fbda410dcbe6fed2725d6097dfbb2990e9a9ba2ece")
    add_versions("v0.2.0.alpha.2", "414db66510c3410ea1e90207fff1be15c72b362983c04b14c63edc5d7067b4f1")
    add_versions("v0.2.0.alpha.3", "a0e0d810a6dfd3919267581766663fa28d1980b8d4a6f111fa770d0d3e6ce211")
    add_versions("v0.2.0.alpha.4", "f758c0a2403256837d84df7b103612d7ad4360917310f3dd6a04f77bbfd22bfb")
    on_install(function(package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DSPUD_BUILD_TESTS=OFF")
        table.insert(configs, "-DSPUD_NO_LTO=ON")
        table.insert(configs, "-DCMAKE_OSX_ARCHITECTURES=x86_64")
        table.insert(configs, "-DSPUD_DETOUR_TRACING=ON")
        import("package.tools.cmake").install(package, configs)
    end)
    package_end()
end

package("protobuf-cpp2")
do
    set_homepage("https://developers.google.com/protocol-buffers/")
    set_description("Google's data interchange format for cpp")

    function get_url(version)
        if version:get() == "3" then
            return format("v%s/protobuf-cpp-%s.zip", version, version)
        end
        return format("v%s/protobuf-%s.zip", version, version)
    end

    add_urls("https://github.com/protocolbuffers/protobuf/releases/download/$(version)", { version = get_url })
    add_versions("3.8.0", "91ea92a8c37825bd502d96af9054064694899c5c7ecea21b8d11b1b5e7e993b5")
    add_versions("3.12.0", "da826a3c48a9cae879928202d6fe06afb15aaee129e9035d6510cc776ddfa925")
    add_versions("3.12.3", "74da289e0d0c24b2cb097f30fdc09fa30754175fd5ebb34fae4032c6d95d4ce3")
    add_versions("3.13.0", "f7b99f47822b0363175a6751ab59ccaa4ee980bf1198f11a4c3cef162698dde3")
    add_versions("3.14.0", "87d6e96166cf5cafc16f2bcfa91c0b54f48bab38538285bee1b9331d992569fa")
    add_versions("3.15.5", "cdd7d3925240af541a95a4361ab100b703bee3a9df0d7e9e05c069cf2c76a039")
    add_versions("3.15.8", "093e0dca5277b377c36a48a3633325dca3d92d68ac17d5700a1f7e1c3eca2793")
    add_versions("3.17.3", "fe65f4bfbd6cbb8c23de052f218cbe4ebfeb72c630847e0cca63eb27616c952a")
    add_versions("3.19.4", "a11a262a395f999f9dca83e195cc15b6c23b6d5e74133f8e3250ad0950485da1")
    add_versions("24.2", "a5c60cd8fbab18f1a795c5a3046c6ce6af3070a7dbe76de8fb147253478bd674")
    add_versions("26.0", "288d3b95e17d7890921d7bf3ea02361faf638bb1370afaf5cfbc4da01d19496c")

    add_patches("3.17.3", path.join(os.scriptdir(), "patches", "3.17.3", "field_access_listener.patch"),
        "ac9bdf49611b01e563fe74b2aaf1398214129454c3e18f1198245549eb281e85")
    add_patches("3.19.4", path.join(os.scriptdir(), "patches", "3.19.4", "vs_runtime.patch"),
        "8e73e585d29f3b9dca3c279df0b11b3ee7651728c07f51381a69e5899b93c367")
    add_patches("26.0", path.join(os.scriptdir(), "patches", "protobuf-cpp", "26.0", "osx_architectures.patch"))

    add_deps("cmake")

    if is_plat("windows") then
        add_links("libprotobuf")
    else
        add_links("protobuf")
    end

    if is_plat("linux") then
        add_syslinks("pthread")
    end

    on_load(function(package)
        package:addenv("PATH", "bin")
        package:add("deps", "abseil2")
    end)

    on_install("windows", "linux", "macosx", function(package)
        local version = package:version()
        if version:major() == "3" then
            os.cd("cmake")
        end
        io.replace("CMakeLists.txt", "set(protobuf_DEBUG_POSTFIX \"d\"", "set(protobuf_DEBUG_POSTFIX \"\"",
            { plain = true })
        local configs = { "-Dprotobuf_BUILD_TESTS=OFF", "-Dprotobuf_BUILD_PROTOC_BINARIES=ON" }
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        if package:is_plat("windows") then
            table.insert(configs,
                "-Dprotobuf_MSVC_STATIC_RUNTIME=" .. (package:config("vs_runtime"):startswith("MT") and "ON" or "OFF"))
            if package:config("shared") then
                package:add("defines", "PROTOBUF_USE_DLLS")
            end
        end
        table.insert(configs, "-Dprotobuf_WITH_ZLIB=OFF")
        table.insert(configs, "-Dprotobuf_ABSL_PROVIDER=package")
        table.insert(configs, "-DCMAKE_CXX_STANDARD=17")
        table.insert(configs, "-DCMAKE_OSX_ARCHITECTURES=x86_64")
        import("package.tools.cmake").install(package, configs, { buildir = "build", packagedeps = { "abseil2" } })
        os.trycp("build/Release/protoc.exe", package:installdir("bin"))
    end)

    on_test(function(package)
        if package:is_cross() then
            return
        end
        io.writefile("test.proto", [[
            syntax = "proto3";
            package test;
            message TestCase {
                string name = 4;
            }
            message Test {
                repeated TestCase case = 1;
            }
        ]])
        os.vrun("protoc test.proto --cpp_out=.")
        assert(package:check_cxxsnippets({ test = io.readfile("test.pb.cc") },
            { configs = { includedirs = { ".", package:installdir("include") }, languages = "c++11" } }))
    end)
    package_end()
end


package("abseil2")
do
    set_homepage("https://abseil.io")
    set_description("C++ Common Libraries")
    set_license("Apache-2.0")

    add_urls("https://github.com/abseil/abseil-cpp/archive/$(version).tar.gz",
        "https://github.com/abseil/abseil-cpp.git")
    add_versions("20200225.1", "0db0d26f43ba6806a8a3338da3e646bb581f0ca5359b3a201d8fb8e4752fd5f8")
    add_versions("20210324.1", "441db7c09a0565376ecacf0085b2d4c2bbedde6115d7773551bc116212c2a8d6")
    add_versions("20210324.2", "59b862f50e710277f8ede96f083a5bb8d7c9595376146838b9580be90374ee1f")
    add_versions("20211102.0", "dcf71b9cba8dc0ca9940c4b316a0c796be8fab42b070bb6b7cab62b48f0e66c4")
    add_versions("20220623.0", "4208129b49006089ba1d6710845a45e31c59b0ab6bff9e5788a87f55c5abd602")
    add_versions("20230125.2", "9a2b5752d7bfade0bdeee2701de17c9480620f8b237e1964c1b9967c75374906")
    add_versions("20230802.1", "987ce98f02eefbaf930d6e38ab16aa05737234d7afbab2d5c4ea7adbe50c28ed")
    add_versions("20240116.1", "3c743204df78366ad2eaf236d6631d83f6bc928d1705dd0000b872e53b73dc6a")

    add_deps("cmake")

    if is_plat("macosx") then
        add_frameworks("CoreFoundation")
    end

    on_load(function(package)
        if package:is_plat("windows") and package:config("shared") then
            package:add("defines", "ABSL_CONSUME_DLL")
        end
    end)

    on_install("macosx", "linux", "windows", "mingw", "cross", function(package)
        if package:version() and package:version():eq("20230802.1") and package:is_plat("mingw") then
            io.replace(path.join("absl", "synchronization", "internal", "pthread_waiter.h"), "#ifndef _WIN32",
                "#if !defined(_WIN32) && !defined(__MINGW32__)", { plain = true })
            io.replace(path.join("absl", "synchronization", "internal", "win32_waiter.h"),
                "#if defined(_WIN32) && _WIN32_WINNT >= _WIN32_WINNT_VISTA",
                "#if defined(_WIN32) && !defined(__MINGW32__) && _WIN32_WINNT >= _WIN32_WINNT_VISTA", { plain = true })
        end
        local configs = { "-DCMAKE_CXX_STANDARD=17" }
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DCMAKE_OSX_ARCHITECTURES=x86_64")
        import("package.tools.cmake").install(package, configs, { buildir = os.tmpfile() .. ".dir" })

        -- get links and ensure link order
        import("core.base.graph")
        local dag = graph.new(true)
        local pkgconfigdir = package:installdir("lib", "pkgconfig")
        for _, pcfile in ipairs(os.files(path.join(pkgconfigdir, "*.pc"))) do
            local link = path.basename(pcfile)
            local content = io.readfile(pcfile)
            for _, line in ipairs(content:split("\n")) do
                if line:startswith("Requires: ") then
                    local requires = line:sub(10):split(",")
                    for _, dep in ipairs(requires) do
                        dep = dep:split("=")[1]:trim()
                        dag:add_edge(link, dep)
                    end
                end
            end
        end
        local links = dag:topological_sort()
        package:add("links", links)

        local cycle = dag:find_cycle()
        if cycle then
            wprint("cycle links found", cycle)
        end
    end)

    on_test(function(package)
        if package:is_cross() then
            return
        end
        assert(package:check_cxxsnippets({
            test = [[
           #include "absl/strings/numbers.h"
           #include "absl/strings/str_join.h"
           #include <iostream>
           #include <string>
           #include <vector>
           void test() {
             std::vector<std::string> v = {"foo", "bar", "baz"};
             std::string s = absl::StrJoin(v, "-");
             int result = 0;
             auto a = absl::SimpleAtoi("123", &result);
             std::cout << "Joined string: " << s << "\\n";
           }
       ]]
        }, { configs = { languages = "cxx17" } }))
    end)
    package_end()
end

package("simdutf2")
do
    set_homepage("https://simdutf.github.io/simdutf/")
    set_description(
        "Unicode routines (UTF8, UTF16, UTF32): billions of characters per second using SSE2, AVX2, NEON, AVX-512. Part of Node.js.")
    set_license("Apache-2.0")

    add_urls("https://github.com/simdutf/simdutf/archive/refs/tags/$(version).tar.gz",
        "https://github.com/simdutf/simdutf.git")

    add_versions("v3.2.17", "c24e3eec1e08522a09b33e603352e574f26d367a7701bf069a65881f64acd519")
    add_versions("v4.0.9", "599e6558fc8d06f8346e5f210564f8b18751c93d83bce1a40a0e6a326c57b61e")
    add_versions("v5.0.0", "088d750466bf3487117cce7f828eb94a0a3474d7e76b45d4902c99a2387212b7")

    add_configs("iconv",
        {
            description = "Whether to use iconv as part of the CMake build if available.",
            default = false,
            type =
            "boolean"
        })

    add_deps("cmake")

    on_load(function(package)
        if package:config("iconv") then
            package:add("deps", "libiconv")
        end
    end)

    on_install(function(package)
        local configs = { "-DSIMDUTF_TESTS=OFF", "-DSIMDUTF_BENCHMARKS=OFF", "-DSIMDUTF_TOOLS=OFF", }
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DSIMDUTF_ICONV=" .. (package:config("iconv") and "ON" or "OFF"))
        table.insert(configs, "-DCMAKE_OSX_ARCHITECTURES=x86_64")
        io.replace("CMakeLists.txt", "add_subdirectory(singleheader)", "", { plain = true })
        import("package.tools.cmake").install(package, configs)
    end)

    on_test(function(package)
        if package:is_cross() then
            return
        end
        assert(package:check_cxxsnippets({
            test = [[
            #include <simdutf.h>
            void test() {
                bool validutf8 = simdutf::validate_utf8("1234", 4);
            }
        ]]
        }, { configs = { languages = "c++11" } }))
    end)
    package_end()
end

package("eastl2")
do
    set_homepage("https://github.com/electronicarts/EASTL")
    set_description("EASTL stands for Electronic Arts Standard Template Library.")
    set_license("BSD-3-Clause")

    set_urls("https://github.com/electronicarts/EASTL/archive/refs/tags/$(version).tar.gz",
        "https://github.com/electronicarts/EASTL.git")
    add_versions("3.17.03", "50a072066e30fda364d482df6733572d8ca440a33825d81254b59a6ca9f4375a")
    add_versions("3.17.06", "9ebeef26cdf091877ee348450d2711cd0bb60ae435309126c0adf8fec9a01ea5")
    add_versions("3.18.00", "a3c5b970684be02e81fb16fbf92ed2584e055898704fde87c72d0331afdea12b")
    add_versions("3.21.12", "2a4d77e5eda23ec52fea8b22abbf2ea8002f38396d2a3beddda3ff2e17f7db2e")

    add_deps("cmake")
    add_deps("eabase")

    on_install("windows", "linux", "macosx", function(package)
        io.replace("CMakeLists.txt", "add_subdirectory(test/packages/EABase)", "", { plain = true })
        io.replace("CMakeLists.txt", "target_link_libraries(EASTL EABase)", "", { plain = true })
        local configs = { "-DEASTL_BUILD_TESTS=OFF", "-DEASTL_BUILD_BENCHMARK=OFF" }
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        if not package:is_plat("windows") then
            table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        end
        table.insert(configs, "-DCMAKE_OSX_ARCHITECTURES=x86_64")
        import("package.tools.cmake").install(package, configs, { packagedeps = "eabase" })
        os.cp("include/EASTL", package:installdir("include"))
    end)


    on_test(function(package)
        if package:is_cross() then
            return
        end
        assert(package:check_cxxsnippets({
            test = [[
            void test() {
                eastl::vector<int> testInt{};
            }
        ]]
        }, { configs = { languages = "c++17" }, includes = "EASTL/vector.h" }))
    end)
    package_end()
end
