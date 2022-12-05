/**
 * @file vas_fir_read.h
 * @author Thomas Resch <br>
 * Audio Communication Group, TU-Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for calculating convolution based virtual acoustics (mainly dynamic binaural synthesis) <br>
 * <br>
 * @brief Utilties for reading IRs from arrays, .txt. and .sofa  files.<br>
 * <br>
 */

#ifndef vas_fir_read_h
#define vas_fir_read_h

#include <stdio.h>
#include "vas_fir.h"
#include "vas_fir_list.h"
#ifdef VAS_USE_LIBMYSOFA
#include "mysofa.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void vas_fir_read_impulseFromFile(vas_fir *x, char *fullpath, int segmentSize, int offset, int end);
void vas_fir_read_singleImpulseFromFloatArray(vas_fir *x, char *name, float *left, float *right, float length, int segmentSize, int offset, int end, bool filterSharingActive);

#ifdef __cplusplus
}
#endif

#endif /* vas_fir_read_h */
