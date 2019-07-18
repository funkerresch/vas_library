//
//  btheadtracker.swift
//  btheadtracker
//
//  Created by Thomas Resch on 27.05.19.
//  Copyright © 2019 Harvey Keitel. All rights reserved.
//

import CoreBluetooth
import os.log

var DEBUG_LEGACY = true;
var SEND_TO_LOCALHOST = true;

enum HeadtrackerType {
    case UNKNOWN
    case RWAHEADTRACKER_BNO055 // this is build with a Red Bear Duo Board and a Bosch BNO055
    case RWAHEADTRACKER_BNO080 // this is build with a Feather ESP32 Board and a Bosch BNO080
}

@objc public class VasHeadtrackerConnect: NSObject, CBCentralManagerDelegate, CBPeripheralDelegate, OSCServerDelegate {
    var azimuth = 0;
    var elevation = 0;
    var azimuthOffset = 0
    var elevationOffset = 0
    var azimuthOrg = 0
    var elevationOrg = 0
    var headtrackerType: HeadtrackerType
    var headTrackerConnected:Bool
    var azimuthInverted:Bool
    var headtrackerId:String
    var centralManager:CBCentralManager!
    var peripheral:CBPeripheral?
    var dataBuffer:NSMutableData!
    var autoReconnect:Bool;
    var client = OSCClient(address: "localhost", port: 51080)
    
    @objc public init(headtrackerId: String, portNumber: Int)
    {        
        self.dataBuffer = NSMutableData()
        self.headtrackerId = headtrackerId;
        self.headTrackerConnected = false
        self.headtrackerType = HeadtrackerType.RWAHEADTRACKER_BNO080
        self.autoReconnect = true
        self.azimuthInverted = false
        
        if #available(iOS 10.0, *) {
            os_log("%@", type: .debug, "Init RWAHeadtrackerConnect for tracker ID: \(headtrackerId)")
        } else {
            if DEBUG_LEGACY {
                print("Init RWAHeadtrackerConnect for tracker ID: \(headtrackerId)") }
        }
        
        super.init();
        connect()
    }
    
    public func setAzimuthOffset()
    {
        azimuthOffset = azimuthOrg
    }
    
    public func setOscPortNumber(portNumber: Int)
    {
        client.port = portNumber        
    }
    
    public func setHeadTrackerId(headTrackerId: String)
    {
        disconnect();
        self.headtrackerId = headTrackerId;
        connect();
    }
    
    public func sendTimestamp()
    {
        var timestamp = NSDate().timeIntervalSince1970 * 1000
        timestamp -= 1559393916351
        let message = OSCMessage(OSCAddressPattern("/time"), Int(timestamp))
        print(Int(timestamp));
        client.send(message);
        
    }
    
    public func stopScanning() {
        if(centralManager != nil) {
            centralManager.stopScan()
        }
    }
    
    public func startScanning() {
        if centralManager.isScanning {
            return;
        }
        
        switch(self.headtrackerType) {
        case .RWAHEADTRACKER_BNO055:
            centralManager.scanForPeripherals(withServices: [CBUUID.init(string: Device.TransferService)], options: [CBCentralManagerScanOptionAllowDuplicatesKey:true])
        case .RWAHEADTRACKER_BNO080:
            centralManager.scanForPeripherals(withServices: nil, options: [CBCentralManagerScanOptionAllowDuplicatesKey:true])
        default:
            if #available(iOS 10.0, *) {
                os_log("%@", type: .debug, "Unknow Headtracker Type")
            } else {
                if DEBUG_LEGACY {
                    print("Unknow Headtracker Type") }
            }
            return;
        }
        
        if #available(iOS 10.0, *) {
            os_log("%@", type: .debug, "Scanning started")
        } else {
            if DEBUG_LEGACY {
                print("Scanning started") }
        }
    }
    
    @objc public func connect() {
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    func disconnect() {
        guard let peripheral = self.peripheral else {
            return
        }
        
        if peripheral.state != .connected {
            self.peripheral = nil
            return
        }
        
        guard let services = peripheral.services else {
            centralManager.cancelPeripheralConnection(peripheral)
            return
        }
        
        for service in services {
            if let characteristics = service.characteristics {
                for characteristic in characteristics {
                    if characteristic.uuid == CBUUID.init(string: Device.TransferCharacteristic) {
                        // We can return after calling CBPeripheral.setNotifyValue because CBPeripheralDelegate's
                        // didUpdateNotificationStateForCharacteristic method will be called automatically
                        peripheral.setNotifyValue(false, for: characteristic)
                        return
                    }
                }
            }
        }
        centralManager.cancelPeripheralConnection(peripheral)
    }
    
    public func centralManagerDidUpdateState(_ central: CBCentralManager) {
        print("Central Manager State Updated: \(central.state)")
        
        if central.state != .poweredOn {
            self.peripheral = nil
            return
        }
        
        startScanning()
        
        guard let peripheral = self.peripheral else {
            return
        }
        
        // see if that peripheral is connected
        guard peripheral.state == .connected else {
            return
        }
        
        // make sure the peripheral has services
        guard let peripheralServices = peripheral.services else {
            return
        }
        
        // we have services, but we need to check for the Transfer Service
        // (honestly, this may be overkill for our project but it demonstrates how to make this process more bulletproof...)
        // Also: Pardon the pyramid.
        let serviceUUID = CBUUID(string: Device.TransferService)
        if let serviceIndex = peripheralServices.firstIndex(where: {$0.uuid == serviceUUID}) {
            // we have the service, but now we check to see if we have a characteristic that we've subscribed to...
            let transferService = peripheralServices[serviceIndex]
            let characteristicUUID = CBUUID(string: Device.TransferCharacteristic)
            if let characteristics = transferService.characteristics {
                if let characteristicIndex = characteristics.firstIndex(where: {$0.uuid == characteristicUUID}) {
                    // Because this is a characteristic that we subscribe to in the standard workflow,
                    // we need to check if we are currently subscribed, and if not, then call the
                    // setNotifyValue like we did before.
                    let characteristic = characteristics[characteristicIndex]
                    if !characteristic.isNotifying {
                        peripheral.setNotifyValue(true, for: characteristic)
                    }
                } else {
                    // if we have not discovered the characteristic yet, then call discoverCharacteristics, and the delegate method will get called as in the standard workflow...
                    peripheral.discoverCharacteristics([characteristicUUID], for: transferService)
                }
            }
        } else {
            // we have a CBPeripheral object, but we have not discovered the services yet,
            // so we call discoverServices and the delegate method will handle the rest...
            peripheral.discoverServices([serviceUUID])
        }
    }
    
    public func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        //print("Discovered \(String(describing: peripheral.name)) at \(RSSI)")
                switch(headtrackerType)
        {
        case .RWAHEADTRACKER_BNO055:
            var advdata = advertisementData
            let rwaTrackerId:String? = String(describing: advdata.removeValue(forKey: "kCBAdvDataLocalName")!)
            
            print( "\(rwaTrackerId!)  \(headtrackerId)" )
            
            if (rwaTrackerId! == headtrackerId)
            {
                if self.peripheral != peripheral {

                    self.peripheral = peripheral
                    print("Connecting to peripheral: \(peripheral)")
                    centralManager?.connect(peripheral, options: nil)
                }
            }
        
        case .RWAHEADTRACKER_BNO080:
            if(peripheral.name == headtrackerId)
            {
                if self.peripheral != peripheral {
                    
                    // save a reference to the peripheral object so Core Bluetooth doesn't get rid of it
                    self.peripheral = peripheral
                    print("Connecting to peripheral: \(peripheral)")
                    centralManager?.connect(peripheral, options: nil)
                }
            }
        default:
            return
        }
    }
    
    public func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        centralManager.stopScan()
        headTrackerConnected = true
        dataBuffer.length = 0
        peripheral.delegate = self
        
        if #available(iOS 10.0, *) {
            os_log("%@", type: .debug, "Connected to RWAHeadtracker")
        } else {
            if DEBUG_LEGACY {
                print("Connected to RWAHeadtracker") }
        }
        
        peripheral.discoverServices(nil)
    }
    
    public func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        print("Failed to connect to \(peripheral) (\(String(describing: error?.localizedDescription)))")
        self.disconnect()
    }
    
    public func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {

        self.peripheral = nil
        headTrackerConnected = false
        
        if #available(iOS 10.0, *) {
            os_log("%@", type: .debug, "Disconnected from RWAHeadtracker")
        } else {
            if DEBUG_LEGACY {
                print("Disconnected from RWAHeadtracker") }
        }
        
        if autoReconnect {
            startScanning()
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        
        if error != nil {
            print("Error discovering services: \(String(describing: error?.localizedDescription))")
            disconnect()
            return
        }
        
        if let services = peripheral.services {
            
            for service in services {
                print("Discovered service \(service)")
                if (service.uuid == CBUUID(string: Device.TransferService)) {
                    let transferCharacteristicUUID = CBUUID.init(string: Device.TransferCharacteristic)
                    peripheral.discoverCharacteristics([transferCharacteristicUUID], for: service)
                }
            }
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if error != nil {
            print("Error discovering characteristics: \(String(describing: error?.localizedDescription))")
            return
        }
        
        if let characteristics = service.characteristics {
            for characteristic in characteristics {
                
                if characteristic.uuid == CBUUID(string: Device.TransferCharacteristic) {
                    // subscribe to dynamic changes
                    print("Found RWA Headtracker")
                    peripheral.setNotifyValue(true, for: characteristic)
                }
            }
        }
    }
    
    public func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?)
    {
        if error != nil {
            print("Error updating value for characteristic: \(characteristic) - \(String(describing: error?.localizedDescription))")
            return
        }
        
        guard let value = characteristic.value else {
            print("Characteristic Value is nil on this go-round")
            return
        }
        
        guard let nextChunk = String(data: value, encoding: String.Encoding.utf8) else {
            print("Next chunk of data is nil.")
            return
        }
        
        let words = nextChunk.components(separatedBy: " ")
        if characteristic.uuid == CBUUID(string: Device.TRACKERSERVICETX)
        {
            if words.count < 3 {
                return;
            }
            
            let azimuthTmp = words[0]
            let elevationTmp = words[1]
            
            azimuthOrg = Int(NSString(string: azimuthTmp).intValue)
            azimuth = azimuthOrg-azimuthOffset
            if(azimuth < 0) {
                azimuth += 360 }
            
            if(azimuthInverted)
            {
                azimuth = 360 - azimuth
            }
            
            elevationOrg = Int(NSString(string: elevationTmp).intValue)
            elevation = elevationOrg-elevationOffset
            
            if(SEND_TO_LOCALHOST == true)
            {
                var message = OSCMessage(OSCAddressPattern("/azimuth"), azimuth)
                client.send(message);
                message = OSCMessage(OSCAddressPattern("/elevation"), elevation)
                client.send(message);
                print(azimuth);
                //sendTimestamp();

            }
        }
        else {
            print(characteristic.uuid);
        }
        
        // If we get the EOM tag, we fill the text view
        if (nextChunk == Device.EOM) {
            if let message = String(data: dataBuffer as Data, encoding: String.Encoding.utf8) {
                // textView.text = message
                print("Final message: \(message)")
                
                // truncate our buffer now that we received the EOM signal!
                dataBuffer.length = 0
            }
        }
    }
    
    /*
     Invoked when the peripheral receives a request to start or stop providing notifications
     for a specified characteristic’s value.
     
     This method is invoked when your app calls the setNotifyValue:forCharacteristic: method.
     If successful, the error parameter is nil.
     If unsuccessful, the error parameter returns the cause of the failure.
     */
    public func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        // if there was an error then print it and bail out
        if error != nil {
            print("Error changing notification state: \(String(describing: error?.localizedDescription))")
            return
        }
        
        if characteristic.isNotifying {
            // notification started
            print("Notification STARTED on characteristic: \(characteristic)")
        } else {
            // notification stopped
            print("Notification STOPPED on characteristic: \(characteristic)")
            self.centralManager.cancelPeripheralConnection(peripheral)
        }
    }
}
