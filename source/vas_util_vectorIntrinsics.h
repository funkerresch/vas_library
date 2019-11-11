//
//  vas_util_vectorIntrinsics.h
//  vas_binauralrir_unity
//
//  Created by Harvey Keitel on 29.10.19.
//

#ifndef vas_util_vectorIntrinsics_h
#define vas_util_vectorIntrinsics_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <immintrin.h>
#include <stdbool.h>


#ifdef __cplusplus
}
#endif

#ifdef __GNUC__

void __cpuid(int* cpuinfo, int info);

unsigned long long _xgetbv(unsigned int index);

#endif

bool vas_util_vectorIntrinsics_supportsAVX(void);

#endif /* vas_util_vectorIntrinsics_h */
