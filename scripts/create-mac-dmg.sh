#!/bin/bash

set -xe

xmake clean
xmake f -y -a x86_64 -m release
xmake

rm build/macosx/x86_64/release/libmods.a
rm build/macosx/x86_64/release/macOSLauncher
mv build/macosx/x86_64/release/stfc-community-patch-loader build/macosx/x86_64/release/macOSLauncher.app/Contents
mv build/macosx/x86_64/release/libstfc-community-patch.dylib build/macosx/x86_64/release/macOSLauncher.app/Contents
cp assets/launcher.icns build/macosx/x86_64/release/macOSLauncher.app/Contents/Resources
rm -r build/macosx/x86_64/release/STFC\ Community\ Patch.app || true
mv build/macosx/x86_64/release/macOSLauncher.app build/macosx/x86_64/release/STFC\ Community\ Patch.app

codesign --force --verify --verbose --deep --sign "-" build/macosx/x86_64/release/STFC\ Community\ Patch.app

rm STFC-Community-Patch-Installer.dmg || true
create-dmg \
  --volname "STFC Community Patch Installer" \
  --background "assets/mac_installer_background.png" \
  --window-pos 200 120 \
  --window-size 800 400 \
  --icon-size 100 \
  --icon "STFC Community Patch.app" 200 190 \
  --hide-extension "STFC Community Patch.app" \
  --app-drop-link 600 185 \
  "STFC-Community-Patch-Installer.dmg" \
  "build/macosx/x86_64/release/"
