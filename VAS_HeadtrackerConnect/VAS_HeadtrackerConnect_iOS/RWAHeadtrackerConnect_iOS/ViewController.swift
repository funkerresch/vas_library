//
//  ViewController.swift
//  RWAHeadtrackerConnect_iOS
//
//  Created by Harvey Keitel on 05.06.19.
//  Copyright © 2019 Beryllium Design. All rights reserved.
//

import UIKit

var TRACKERID = "rwaht81"


class ViewController: UIViewController {
    
    var btconnect:VasHeadtrackerConnect!
    override func viewDidLoad() {
        super.viewDidLoad()
        self.btconnect = VasHeadtrackerConnect(headtrackerId: TRACKERID, portNumber: Int(51051))
        // Do any additional setup after loading the view.
    }
}

