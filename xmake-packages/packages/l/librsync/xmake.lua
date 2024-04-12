package("librsync")
add_deps("cmake")
add_urls("https://github.com/librsync/librsync/archive/refs/tags/$(version).tar.gz",
    "https://github.com/librsync/librsync.git")
add_versions("v2.3.4", "a0dedf9fff66d8e29e7c25d23c1f42beda2089fb4eac1b36e6acd8a29edfbd1f")
add_patches("v2.3.4", path.join(os.scriptdir(), "patches", "v2.3.4", "rsync_cmake.patch"))
on_install(function(package)
    local configs = {}
    table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
    table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
    import("package.tools.cmake").install(package, configs)
end)
