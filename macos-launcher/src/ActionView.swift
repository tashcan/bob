import Foundation
import SwiftUI

struct ActionView: View {

  @StateObject var gameUpdater = GameUpdaterViewModel()
  @State private var gameVersion: Int = 0
  @State private var gameUpdateAvailable: Bool = false

  var body: some View {
    Grid {
      GridRow {
        Button {
        } label: {
          commonButton()
            .foregroundColor(.lcarViolet)
        }.buttonStyle(PlainButtonStyle())
        Button {

          withAnimation {
            if gameUpdateAvailable {
              Task {
                do {
                  try await gameUpdater.updateGame()
                } catch {
                  print("Error updating game: \(error)")
                }
                gameUpdateAvailable = await gameUpdater.checkForGameUpdate()
              }
            }
          }
        } label: {
          if gameUpdateAvailable {
            commonButton(text: "Update Game!")
              .foregroundColor(.lcarTan)
          } else {
            commonButton()
              .foregroundColor(.lcarTan)
          }
        }.buttonStyle(PlainButtonStyle())
        Button {
          withAnimation {
            launchSTFC()
          }
        } label: {
          commonButton(text: "Engage!")
            .foregroundColor(.lcarOrange)
        }.buttonStyle(PlainButtonStyle())
      }
    }
    .frame(width: 300, height: 150)
    .offset(x: 85, y: 20)
    .task {
      gameVersion = await gameUpdater.fetchLatestGameVersion()
      gameUpdateAvailable = await gameUpdater.checkForGameUpdate()
    }
  }

  private func commonButton(text: String = "") -> some View {
    RoundedRectangle(cornerRadius: 20)
      .frame(width: 125, height: 50)
      .overlay(alignment: .bottomTrailing) {
        HStack {
          Spacer()
          Text(text.count > 0 ? text : "\(randomDigits(4))-\(randomDigits(3))")
            .font(.custom("HelveticaNeue-CondensedBold", size: 17))
            .foregroundColor(.black)
        }
        .scaleEffect(x: 0.7, anchor: .trailing)
        .padding(.bottom, 5)
        .padding(.trailing, 20)
      }
  }

  private func randomDigits(_ count: Int) -> String {
    (1...count)
      .map { _ in "\(Int.random(in: 0...9))" }
      .joined()
  }

  private func launchSTFC() {
    let process = Process()
    let helper = Bundle.main.path(forAuxiliaryExecutable: "stfc-community-patch-loader")
    process.executableURL = URL(fileURLWithPath: helper!)
    try! process.run()
    process.waitUntilExit()
  }

}
