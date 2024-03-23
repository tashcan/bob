add_rules("mode.debug", "mode.release")

target("macOSLauncher")
do
    add_rules("xcode.application")
    add_files("src/*.swift", "src/*.xcassets")
    add_files("src/Info.plist")
    add_values("xcode.bundle_identifier", "com.tashcan.startrekpatch")
    add_values("xcode.bundle_display_name", "Star Trek Fleet Command Community Patch")
end
