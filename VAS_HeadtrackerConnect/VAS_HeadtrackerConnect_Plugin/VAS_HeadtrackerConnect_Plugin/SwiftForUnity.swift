import Foundation
import CoreBluetooth


@objc public class SwiftForUnity: NSObject {
      @objc public static let shared = SwiftForUnity()
      @objc public static let btconnect = VasHeadtrackerConnect(headtrackerId: "rwaht00", portNumber: 51080)
}
    

