import Foundation

public protocol ManagedURL {
  var contentURL: URL { get }
  func keepAlive()
}

extension ManagedURL {
  public func keepAlive() {}
}

extension URL: ManagedURL {
  public var contentURL: URL { return self }
}
