import Foundation
import librsync

func rsyncApply(source: URL, patch: URL, output: URL) throws {
  let err = rsync_apply(source.path, patch.path, output.path)
  if err != 0 {
    throw NSError(domain: NSPOSIXErrorDomain, code: Int(err), userInfo: nil)
  }
}
