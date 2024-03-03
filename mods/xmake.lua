target("mods")
    set_kind("static")
    add_files("src/**.cc")
    add_includedirs("src", {public = true})
    add_packages("spud", "nlohmann_json", "protobuf-cpp", "libil2cpp", "eastl", "toml++", "spdlog")
    add_rules("protobuf.cpp")
    add_files("src/prime/proto/*.proto")
    set_exceptions("cxx")
    if is_plat("windows") then
        add_cxflags("/bigobj")
    end
    set_policy("build.optimization.lto", true)