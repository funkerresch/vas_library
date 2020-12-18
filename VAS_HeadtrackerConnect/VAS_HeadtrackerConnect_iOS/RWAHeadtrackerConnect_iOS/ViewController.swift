//
//  ViewController.swift
//  RWAHeadtrackerConnect_iOS
//
//  Created by Thomas Resch on 05.06.19.
//  Copyright © 2019 Beryllium Design. All rights reserved.
//

import UIKit

extension String {
    var isInt: Bool {
        return Int(self) != nil
    }
}

class ViewController: UIViewController, UITextFieldDelegate {
    
    var btconnect:VasHeadtrackerConnect!
    
    @IBOutlet weak var headTrackerId: UITextField!
    @IBOutlet weak var portNumber: UITextField!
    @IBOutlet weak var azimuthInverted: UISwitch!
    @IBOutlet weak var tare: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.headTrackerId.delegate = self
        self.headTrackerId.text = "rwaht84"
        self.portNumber.text = "51080"
        self.azimuthInverted.isOn = false
        self.tare.isSelected = false;
        self.btconnect = VasHeadtrackerConnect(headtrackerId: self.headTrackerId.text!, portNumber: Int(51051))
    }
    
    func textFieldShouldReturn(_ textField: UITextField) -> Bool
    {
        textField.resignFirstResponder();
        return true;
    }
    
    @IBAction func updateHeadtrackerId(_ sender: Any) {
        btconnect.setHeadTrackerId(headTrackerId: self.headTrackerId.text!)
        print(self.headTrackerId.text!);
    }
    
    @IBAction func updateAzimuthInverted(_ sender: Any) {
        if(self.azimuthInverted.isOn == true) {
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

