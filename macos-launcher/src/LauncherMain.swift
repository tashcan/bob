//
//  LauncherMain.swift
//  STFC Community Patch Launcher
//
//  Created by tashcan on 3/22/24.
//

import SwiftUI

extension Scene {
    func windowResizabilityContentSize() -> some Scene {
        if #available(macOS 13.0, *) {
            return windowResizability(.contentSize)
        } else {
            return self
        }
    }
}

@main
struct STFC_Community_Patch_LauncherApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
                .frame(width: 600, height: 400)
                .fixedSize()
        }
        .windowResizabilityContentSize()
    }
}
