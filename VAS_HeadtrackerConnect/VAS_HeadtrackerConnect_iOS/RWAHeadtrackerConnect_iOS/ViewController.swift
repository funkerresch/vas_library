//
//  ViewController.swift
//  RWAHeadtrackerConnect_iOS
//
//  Created by Harvey Keitel on 05.06.19.
//  Copyright © 2019 Beryllium Design. All rights reserved.
//

import UIKit

var TRACKERID = "rwaht00"
var btconnect = VasHeadtrackerConnect(headtrackerId: TRACKERID)

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        btconnect.connect()
        // Do any additional setup after loading the view.
    }
}

