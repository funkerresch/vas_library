//
//  vas_pitch.h
//  tr.binaural~
//
//  Created by Admin on 17.09.18.
//

#ifndef vas_pitch_h
#define vas_pitch_h

/* YIN configs */
#define YIN_DEFAULT_THRESHOLD 0.20
#define YIN_DEFAULT_OVERLAP 1536

#include <stdio.h>
#include "vas_util.h"

typedef struct vas_pitch
{
    float yin_buffer[1024];
    int yin_bufferSize;
    
} vas_pitch;

#endif /* vas_pitch_h */
