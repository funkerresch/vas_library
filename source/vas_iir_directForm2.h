//
//  vas_iir_directForm2.h
//  AudioPlugin_VAS_Binaural
//
//  Created by Harvey Keitel on 25.02.21.
//
//
//                                          FLOOR MATERIAL COEFFICIENTS
//
// Carpet (Office) : -1.91121051689704 0.914140685265147 0.838793814064210 -1.59884321376178 0.762883081966184
//
// Concrete: -1.82643639390958 0.847882124746266 0.975753295649048 -1.78001817490876 0.825516449664865
//
// Wood Blocks: -1.11798701440198 0.131988928466843 1.00537107888312 -1.15940905804911 0.164393766696331
//
//                                          WALL MATERIAL COEFFICIENTS
//
// Ziegelstein: -1.93551356895674 0.945684912526935 0.975107581537218 -1.88648386304401 0.921330635829737
//
// Wood (Panneling): -1.13182956041605 0.149379533567893 0.900219266855330 -0.993059417426799 0.109012420679139
//
// Glas: -1.12806332158209 0.141367047902679 0.978072438754895 -1.09798447472100 0.132313458565826
//
// Wood (With air space): -1.14152517185384 0.155556224109876 0.860965182351408 -0.941133498161133 0.0893499730675749
//
//                                          CEILING MATERIAL COEFFICIENTS
//
// Fibreboard: -1.81421442650781 0.825697950385756 0.840981738424465 -1.51709285559445 0.687304777790377

#ifndef vas_iir_directForm2_h
#define vas_iir_directForm2_h

#include <stdio.h>
#include "vas_util.h"

#include "vas_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vas_iir_directForm2
{
    int order;
    float sampleRate;
    float a1;
    float a2;
    float a3;
    float b0;
    float b1;
    float b2;
    float b3;
    
    VAS_INPUTBUFFER vz1;
    VAS_INPUTBUFFER vz2;
    VAS_INPUTBUFFER vz3;
    
    VAS_INPUTBUFFER x1;
    VAS_INPUTBUFFER x2;
    VAS_INPUTBUFFER y1;
    VAS_INPUTBUFFER y2;

    double coeffs[8][4];
    
    float coeffs1[4][8];
    
} vas_iir_directForm2;

void vas_iir_directForm2_process(vas_iir_directForm2 *x, VAS_INPUTBUFFER *data, int n);

void vas_iir_directForm2_1stOrder_setCoefficients(vas_iir_directForm2 *x, float b0, float a1, float b1 );

void vas_iir_directForm2_2ndOrder_setCoefficients(vas_iir_directForm2 *x, float b0, float a1, float b1, float a2, float b2 );

void vas_iir_directForm2_2ndOrder_process_neon(vas_iir_directForm2 *x, VAS_INPUTBUFFER *in, int n);

void vas_iir_directForm2_3rdOrder_setCoefficients(vas_iir_directForm2 *x, float b0, float a1, float b1, float a2, float b2,  float a3, float b3 );

vas_iir_directForm2 *vas_iir_directForm2_new(int order) ;

void vas_iir_directForm2_free(vas_iir_directForm2 *x);

#ifdef __cplusplus
}
#endif

#endif /* vas_iir_directForm2_h */
