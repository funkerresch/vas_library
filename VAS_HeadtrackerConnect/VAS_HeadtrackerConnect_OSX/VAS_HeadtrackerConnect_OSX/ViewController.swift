//
//  ViewController.swift
//  VAS_HeadtrackerConnect_OSX
//
//  Created by Harvey Keitel on 13.06.19.
//  Copyright © 2019 Beryllium Design. All rights reserved.
//

import Cocoa


let TRACKERID = "rwaht81"

extension String {
    var isInt: Bool {
        return Int(self) != nil
    }
}

class ViewController: NSViewController, NSTextFieldDelegate {

    var btconnect:VasHeadtrackerConnect!
    
    @IBOutlet weak var headTrackerId: NSTextField!
    @IBOutlet weak var portNumber: NSTextField!
    @IBOutlet weak var azimuthInverted: NSButton!
    @IBOutlet weak var tare: NSButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.headTrackerId.delegate = self
        self.headTrackerId.stringValue = "rwaht85"
        self.portNumber.stringValue = "51080"
        self.azimuthInverted.state = NSControl.StateValue.off
        self.tare.state = NSControl.StateValue.off
        self.btconnect = VasHeadtrackerConnect(headtrackerId: self.headTrackerId.accessibilityValue()!, portNumber: Int(portNumber.accessibilityValue()!)!)
    }

    override var representedObject: Any? {
        didSet {
        // Update the view, if already loaded.
        }
    }
    @IBAction func updateHeadtrackerId(_ sender: Any) {
        btconnect.setHeadTrackerId(headTrackerId: self.headTrackerId.accessibilityValue()!)
    }
    
    @IBAction func updateAzimuthInverted(_ sender: Any) {
        if(self.azimuthInverted.state == NSControl.StateValue.on) {
            btconnect.azimuthInverted = true
        }
        else {
            btconnect.azimuthInverted = false
        }
        
    }
    
    @IBAction func updateTare(_ sender: Any) {
        btconnect.setAzimuthOffset()
    }
    
    @IBAction func updatePortNumber(_ sender: Any) {
        let port = self.portNumber.accessibilityValue()!
        if(port.isInt) {
            btconnect.setOscPortNumber(portNumber: Int(port)!)
        }
    }
}

