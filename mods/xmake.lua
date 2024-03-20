target("mods")
do
    add_ldflags("-v")
    set_kind("static")
    add_files("src/**.cc")
    add_headerfiles("src/**.h")
    add_includedirs("src", { public = true })
    add_packages("spud", "nlohmann_json", "protobuf-cpp2", "libil2cpp", "eastl2", "toml++", "spdlog", "cpp-httplib",
        "simdutf2")
    add_rules("protobuf.cpp")
    add_files("src/prime/proto/*.proto")
    set_exceptions("cxx")
    add_defines("NOMINMAX")
    if is_plat("windows") then
        add_cxflags("/bigobj")
        add_linkdirs("src/il2cpp")
    end
    if is_plat("macosx") then
        add_cxflags("-fms-extensions")
    end
    set_policy("build.optimization.lto", true)
    -- Workaround for silly cmake, protobuf thing where it doesn't actually provide this to the linker...
    add_links("utf8_validity")
end
