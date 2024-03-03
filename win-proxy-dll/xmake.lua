target("stfc-community-patch")
    set_kind("shared")
    add_files("src/*.cc")
    add_files("src/*.rc")
    add_deps("mods")
    set_exceptions("cxx")
    if is_plat("windows") then
        add_cxflags("/bigobj")
    end
    set_policy("build.optimization.lto", true)
    add_links("User32.lib", "Ole32.lib", "OleAut32.lib")