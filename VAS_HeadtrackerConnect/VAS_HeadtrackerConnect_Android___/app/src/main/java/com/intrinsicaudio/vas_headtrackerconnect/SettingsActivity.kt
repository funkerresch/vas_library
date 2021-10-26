package com.intrinsicaudio.vas_headtrackerconnect


import android.Manifest
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.ContentValues.TAG
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.ParcelUuid
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import androidx.preference.PreferenceFragmentCompat
import android.bluetooth.BluetoothGattCharacteristic.*

import java.util.*


class SettingsActivity : AppCompatActivity() {

    var BLP_SERVICE_UUID = UUID.fromString("713D0000-503E-4C75-BA94-3148F18D941E")
    var BLP_CHAR_UUID = UUID.fromString("713D0002-503E-4C75-BA94-3148F18D941E")
    var GATT_DESC = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
    private val REQUEST_ENABLE_BT = 1
    private val ACCESS_COARSE_LOCATION_REQUEST = 2
    private val ACCESS_FINE_LOCATION_REQUEST = 3
    private val mScanning = false

    override fun onCreate(savedInstanceState: Bundle?) {

        val adapter = BluetoothAdapter.getDefaultAdapter()
        val scanner = adapter.bluetoothLeScanner

        val filter = ScanFilter.Builder()
               // .setDeviceName("rwaht84")
                .setServiceUuid(ParcelUuid(BLP_SERVICE_UUID))
                .build()

        val filters = mutableListOf(filter)

        val scanCallback = object : ScanCallback() {
            override fun onScanFailed(errorCode: Int) {
                Log.i("MYTAG", "error")
            }

            override fun onScanResult(
                    callbackType: Int,
                    result: ScanResult
            ) {
                Log.i("MYTAG", "scan result!")
                val myDevice = result!!.device
                if(myDevice.name != null) {
                    Log.i("MYTAG", myDevice.name)
                    val bluetoothGatt = myDevice.connectGatt(applicationContext, true, mGattCallback, BluetoothDevice.TRANSPORT_LE)
                }
            }

            override fun onBatchScanResults(results: MutableList<ScanResult>) {
                Log.i("MYTAG", "Batch scan results: ${results.size}")
            }



            private fun BluetoothGatt.printGattTable() {
                if (services.isEmpty()) {
                    Log.i("printGattTable", "No service and characteristic available, call discoverServices() first?")
                    return
                }
                services.forEach { service ->
                    val characteristicsTable = service.characteristics.joinToString(
                            separator = "\n|--",
                            prefix = "|--"
                    ) { it.uuid.toString() }
                    Log.i("printGattTable", "\nService ${service.uuid}\nCharacteristics:\n$characteristicsTable"
                    )
                }
            }



            // Implements callback methods for GATT events that the app cares about.  For example,
            // connection change and services discovered.
            private val mGattCallback: BluetoothGattCallback = object : BluetoothGattCallback() {
                override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                    Log.i("Debug", "Debug 017")
                    if (status == BluetoothGatt.GATT_SUCCESS) {
                        if (newState == BluetoothProfile.STATE_CONNECTED) {
                            // We successfully connected, proceed with service discovery
                            Log.i("Debug", "Debug 013")
                            gatt.discoverServices()
                        } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                            // We successfully disconnected on our own request
                            Log.i("Debug", "Debug 012")
                            gatt.close()
                        } else {
                            // We're CONNECTING or DISCONNECTING, ignore for now
                            Log.i("Debug", "Debug 011")
                        }
                    } else {
                        // An error happened...figure out what happened!
                        Log.i("Debug", "Debug 010")
                        gatt.close()
                    }
                }

                override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
                    with(gatt) {
                        Log.w("BluetoothGattCallback", "Discovered ${services.size} services for ${device.address}")
                        printGattTable() // See implementation just above this section
                        if (status == BluetoothGatt.GATT_SUCCESS) {
                            var Service = gatt.getService(BLP_SERVICE_UUID);
                            if(Service != null) {
                                var Characteristic = Service.getCharacteristic(BLP_CHAR_UUID);
                                if(Characteristic == null)
                                    Log.i("Char", "CHAR IS INVALID")

                                var des: BluetoothGattDescriptor = BluetoothGattDescriptor(BLP_CHAR_UUID, PERMISSION_READ)
                               /* for (descriptor in Characteristic.getDescriptors()) {
                                    Log.e(TAG, "BluetoothGattDescriptor: " + descriptor.uuid.toString())
                                    if(descriptor != null){
                                        des = descriptor
                                        break
                                    }
                                }*/

                                gatt.setCharacteristicNotification(Characteristic, true);
                                //val descriptor = Characteristic.getDescriptor(GATT_DESC)
                               // gatt.readCharacteristic(Characteristic)
                                if(des != null) {
                                    des.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                                    gatt.writeDescriptor(des)
                                   // gatt.readCharacteristic(Characteristic)
                                    //thread.start();
                                }
                            }
                        }
                    }
                }

                override fun onCharacteristicRead(gatt: BluetoothGatt?, characteristic: BluetoothGattCharacteristic, status: Int) {
                    val data = characteristic.value
                    if (data != null && data.size > 0) {
                        val stringBuilder = StringBuilder(data.size)
                        for (byteChar in data) {
                            stringBuilder.append(String.format("%02X ", byteChar))
                        }
                        val strReceived = stringBuilder.toString()
                        Log.i("data", strReceived)
                    }
                }
            }
        }

       /* val scanCallback: ScanCallback = object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult) {
                val device: BluetoothDevice = result.getDevice()
                // ...do whatever you want with this found device
            }

            override fun onBatchScanResults(results: List<ScanResult?>?) {
                Log.d(TAG, "on batch results")
                if(results.isNullOrEmpty())
                    Log.d(TAG, "no results")

                for (result in results!!) {
                    val myDevice = result!!.device
                    Log.i("Debug", "onBatchScanResults: " + myDevice.name)
                    if (myDevice.name == "rwaht84") {
                        Log.d(TAG, "Found rwaht84")
                        val bluetoothGatt = myDevice.connectGatt(applicationContext, true, mGattCallback, BluetoothDevice.TRANSPORT_LE)
                    }
                }
            }



            override fun onScanFailed(errorCode: Int) {
                Log.d(TAG, "on scan failed")
            }
        }*/



        val scanSettings = ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_POWER)
                .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                .setMatchMode(ScanSettings.MATCH_MODE_AGGRESSIVE)
                .setNumOfMatches(ScanSettings.MATCH_NUM_MAX_ADVERTISEMENT)
                .setReportDelay(5000)

                .build()

        CheckPermissions()

        if (scanner != null) {
          //  scanner.startScan(filters, scanSettings, scanCallback)
            scanner.startScan(scanCallback)

            Log.d(TAG, "scan started")
        } else {
            Log.e(TAG, "could not get scanner object")
        }



       /* val bluetoothAdapter = BluetoothAdapter.getDefaultAdapter()
        val scanner = bluetoothAdapter.bluetoothLeScanner

        CheckPermissions()
        hasPermissions()

        val filter = ScanFilter.Builder()
                .setDeviceName("rwaht84")
                .build()

        val filters = mutableListOf(filter)


        if (!bluetoothAdapter.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT)
        }

        val scanSettings = ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                .setMatchMode(ScanSettings.MATCH_MODE_AGGRESSIVE)
                .setNumOfMatches(ScanSettings.MATCH_NUM_ONE_ADVERTISEMENT)
                .setReportDelay(10)
                .build()

        if (scanner != null)
        {
            scanner.startScan(filters, scanSettings, scanCallback);
            Log.d("Debug", "scan started");
        }  else {
            Log.e("Debug", "could not get scanner object");
        }*/

        super.onCreate(savedInstanceState)
        setContentView(R.layout.settings_activity)
        if (savedInstanceState == null) {
            supportFragmentManager
                    .beginTransaction()
                    .replace(R.id.settings, SettingsFragment())
                    .commit()
        }
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
    }

    /*private val scanCallback: ScanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val device: BluetoothDevice = result.getDevice()
            Log.i("Debug", "fScanCallback")
            // ...do whatever you want with this found device
            Log.i("Debug", "found something 1")
            val gatt = device.connectGatt(applicationContext, true, mGattCallback, BluetoothDevice.TRANSPORT_LE)
            Log.d("Debug", "Trying to create a new connection.")
        }

        fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            Log.i("Debug", "Debug 017")
            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    // We successfully connected, proceed with service discovery
                    Log.i("Debug", "Debug 013")
                    gatt.discoverServices()
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    // We successfully disconnected on our own request
                    Log.i("Debug", "Debug 012")
                    gatt.close()
                } else {
                    // We're CONNECTING or DISCONNECTING, ignore for now
                    Log.i("Debug", "Debug 011")
                }
            } else {
                // An error happened...figure out what happened!
                Log.i("Debug", "Debug 010")
                gatt.close()
            }
        }

        // Implements callback methods for GATT events that the app cares about.  For example,
        // connection change and services discovered.
        private val mGattCallback: BluetoothGattCallback = object : BluetoothGattCallback() {
            override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                var intentAction: String
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.i("Debug", "Connected to GATT server.")
                    // Attempts to discover services after successful connection.
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.i("Debug", "Debug 009")
                }
            }
        }

        override fun onBatchScanResults(results: List<ScanResult?>?) {
            for (result in results!!) {
                val myDevice = result!!.device
                Log.i("Debug", "onBatchScanResults: " + myDevice.name)
                if (myDevice.name == "rwaht84") {
                    val bluetoothGatt = myDevice.connectGatt(applicationContext, true, mGattCallback, BluetoothDevice.TRANSPORT_LE)
                }
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.d("Debug", "found something 3")
        }
    }
    */


    private fun CheckPermissions(): Boolean {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (applicationContext.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                Log.i("Debug", "COARSE NOT GRANTED")
                requestPermissions(arrayOf(Manifest.permission.ACCESS_COARSE_LOCATION), 1)
                return false
            }

            if (applicationContext.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
                Log.i("Debug", "FINE NOT YET GRANTED")
                requestPermissions(arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), 2)
                return false
            }

            if (applicationContext.checkSelfPermission(Manifest.permission.BLUETOOTH) != PackageManager.PERMISSION_GRANTED) {
                Log.i("Debug", "BLUETOOTH NOT YET GRANTED")
                requestPermissions(arrayOf(Manifest.permission.BLUETOOTH), 3)
                return false
            }

            if (applicationContext.checkSelfPermission(Manifest.permission.BLUETOOTH_ADMIN) != PackageManager.PERMISSION_GRANTED) {
                Log.i("Debug", "BLUETOOTH ADMIN NOT YET GRANTED")
                requestPermissions(arrayOf(Manifest.permission.BLUETOOTH_ADMIN), 4)
                return false
            }


            Log.i("Debug", "Debug 002")
        }
        return true
    }

    private fun hasPermissions(): Boolean {
        Log.i("Debug", "Debug 020")
        if (applicationContext.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            Log.i("Debug", "Debug 021")
            requestPermissions(arrayOf(Manifest.permission.ACCESS_COARSE_LOCATION), ACCESS_COARSE_LOCATION_REQUEST)
            return false
        }

        if (applicationContext.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            Log.i("Debug", "Debug 021")
            requestPermissions(arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), ACCESS_FINE_LOCATION_REQUEST)
            return false
        }
        return false
    }



    class SettingsFragment : PreferenceFragmentCompat() {
        override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
            setPreferencesFromResource(R.xml.root_preferences, rootKey)
        }
    }
}