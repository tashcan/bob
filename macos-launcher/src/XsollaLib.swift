import Foundation

struct XsollaUpdater {
  var gameName: String

  public init(_ gameName: String) {
    self.gameName = gameName
  }


  func gamePath() -> String {
    return ""
  }

  func installedVersion() -> Int {
  do {
      var gamePath = URL(fileURLWithPath: self.gamePath())
      gamePath.appendPathComponent(".version")
      let versionFile = try String(contentsOf: gamePath)
      let versionSegments = versionFile.split(separator: "=")
      return Int(versionSegments[1]) ?? 0
      } catch {
      return 0
      }
  }

  func checkForUpdateAvailable() -> Bool {
    return false;

  }
}

func run_update() {
    //
}
