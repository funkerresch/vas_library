//
//  vas_fir_read.h
//  vas_binaural~
//
//  Created by Harvey Keitel on 18.02.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#ifndef vas_fir_read_h
#define vas_fir_read_h

#include <stdio.h>
#include "vas_fir.h"
#include "vas_fir_list.h"
#include "mysofa.h"

#ifdef __cplusplus
extern "C" {
#endif

void vas_fir_read_impulseFromFile(vas_fir *x, char *fullpath, int segmentSize, int offset, int end);
void vas_fir_read_singleImpulseFromFloatArray(vas_fir *x, char *name, float *left, float *right, float length, int segmentSize, int offset, int end);

#ifdef __cplusplus
}
#endif

#endif /* vas_fir_read_h */
