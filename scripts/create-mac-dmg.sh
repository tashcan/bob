#!/bin/bash

set -xe

CONFIG=${1:-release}

xmake clean
xmake f -y -a x86_64 -m $CONFIG
xmake

rm build/macosx/x86_64/$CONFIG/libmods.a
rm build/macosx/x86_64/$CONFIG/macOSLauncher
mv build/macosx/x86_64/$CONFIG/stfc-community-patch-loader build/macosx/x86_64/$CONFIG/macOSLauncher.app/Contents
mv build/macosx/x86_64/$CONFIG/libstfc-community-patch.dylib build/macosx/x86_64/$CONFIG/macOSLauncher.app/Contents
cp assets/launcher.icns build/macosx/x86_64/$CONFIG/macOSLauncher.app/Contents/Resources
rm -r build/macosx/x86_64/$CONFIG/STFC\ Community\ Patch.app || true
mv build/macosx/x86_64/$CONFIG/macOSLauncher.app build/macosx/x86_64/$CONFIG/STFC\ Community\ Patch.app

rm -rf build/macosx/x86_64/$CONFIG/*.dSYM || true
codesign --force --verify --verbose --deep --sign "-" build/macosx/x86_64/$CONFIG/STFC\ Community\ Patch.app

rm stfc-community-patch-installer.dmg || true
create-dmg \
  --volname "STFC Community Patch Installer" \
  --background "assets/mac_installer_background.png" \
  --window-pos 200 120 \
  --window-size 800 400 \
  --icon-size 100 \
  --icon "STFC Community Patch.app" 200 190 \
  --hide-extension "STFC Community Patch.app" \
  --app-drop-link 600 185 \
  "stfc-community-patch-installer.dmg" \
  "build/macosx/x86_64/$CONFIG/"
