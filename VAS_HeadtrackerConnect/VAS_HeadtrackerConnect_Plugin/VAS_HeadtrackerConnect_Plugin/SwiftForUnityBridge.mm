//
//  SwiftForUnityBridge.m
//  VAS_HeadtrackerConnect_Plugin
//
//  Created by Harvey Keitel on 12.01.21.
//  Copyright © 2021 Beryllium Design. All rights reserved.
//


#pragma mark - C interface
#import <CoreBluetooth/CoreBluetooth.h>
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include "VAS_HeadtrackerConnect_Plugin/VAS_HeadtrackerConnect_Plugin-Swift.h"
#else
#include "VAS_HeadtrackerConnect_Plugin-Swift.h"
#endif


extern "C" {
    void _setHeadtrackerId(char * rwaid) {
         [[SwiftForUnity btconnect] setHeadtrackerIDWithName:rwaid];
    }

    int _getAzimuth() {
         int azi = [[SwiftForUnity btconnect] azimuth];
         return azi;
    }

    int _getElevation() {
        int ele = [[SwiftForUnity btconnect] elevation];
        return ele;
    }

    void _tare() {
        [[SwiftForUnity btconnect] setAzimuthOffset];
    }

    char* cStringCopy(const char* string){
         if (string == NULL){
              return NULL;
         }
         char* res = (char*)malloc(strlen(string)+1);
         strcpy(res, string);
         return res;
    }
}


