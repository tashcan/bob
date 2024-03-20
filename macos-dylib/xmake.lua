target("stfc-community-patch")
do
    set_kind("shared")
    add_files("src/*.cc")
    add_deps("mods")
    set_exceptions("cxx")
    set_policy("build.optimization.lto", true)
end
