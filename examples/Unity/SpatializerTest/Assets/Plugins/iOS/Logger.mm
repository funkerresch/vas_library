//
//  GameCenterHelper.m
//  Unity-iPhone
//
//  Created by Vladyslav Sviatetskyi on 16/03/2017.
//
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

extern "C"
{
    void _log(const char* log)
    {
        NSString* string = [NSString stringWithUTF8String:log];
        NSLog( @"%@", string );
    }
}
