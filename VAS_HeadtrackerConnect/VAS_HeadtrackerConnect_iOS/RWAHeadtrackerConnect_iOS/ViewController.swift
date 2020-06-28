//
//  ViewController.swift
//  RWAHeadtrackerConnect_iOS
//
//  Created by Harvey Keitel on 05.06.19.
//  Copyright © 2019 Beryllium Design. All rights reserved.
//

import UIKit

var TRACKERID = "rwaht81"

extension String {
    var isInt: Bool {
        return Int(self) != nil
    }
}

class ViewController: UIViewController, UITextFieldDelegate {
    
    var btconnect:VasHeadtrackerConnect!
    
    @IBOutlet weak var headTrackerId: UITextField!
    @IBOutlet weak var portNumber: UITextField!
    @IBOutlet weak var azimuthInverted: UIButton!
    @IBOutlet weak var tare: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.headTrackerId.delegate = self
        self.headTrackerId.text = "rwaht85"
//        self.portNumber.text = "51080"
//        self.azimuthInverted.isSelected = false
//        self.tare.isSelected = false;
        self.btconnect = VasHeadtrackerConnect(headtrackerId: TRACKERID, portNumber: Int(51051))
        // Do any additional setup after loading the view.
    }
    
    @IBAction func updateHeadtrackerId(_ sender: Any) {
        btconnect.setHeadTrackerId(headTrackerId: self.headTrackerId.text!)
    }
    
    @IBAction func updateAzimuthInverted(_ sender: Any) {
        if(self.azimuthInverted.isSelected == true) {
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
        let port = self.portNumber.text
        if(port!.isInt) {
            btconnect.setOscPortNumber(portNumber: Int(port!)!)
        }
    }
}

