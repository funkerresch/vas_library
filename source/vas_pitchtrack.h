

#ifndef vas_pitchtrack_h
#define vas_pitchtrack_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <math.h>
#include "vas_mem.h"
#ifdef PUREDATA
#include "m_pd.h"
#define REALFFT mayer_realfft
#define REALIFFT mayer_realifft
#endif
    
#define DEFFRAMESIZE 1024       // default analysis framesize
#define DEFOVERLAP 1            // default overlap
#define DEFBIAS 0.2             // default bias
#define DEFMINRMS 0.003         // default minimum RMS
#define SEEK 0.85
    
typedef struct vas_pitchtrack
{
    // buffers
    float *inputbuf;
    float *inputbuf2;
    float *processbuf;
    
    // state variables
    int timeindex;
    int framesize;
    int overlap;
    int periodindex;
    
    float periodlength;
    float fidelity;
    float biasfactor;
    float minrms;
    
} vas_pitchtrack;
    
vas_pitchtrack *vas_pitchtrack_new(int frame, int overlap, float bias);
void vas_pitchtrack_iosamples(vas_pitchtrack *x, float *in, float *out, int size);
void vas_pitchtrack_setframesize(vas_pitchtrack *x, int frame);
void vas_pitchtrack_setoverlap(vas_pitchtrack *x, int overlap);
void vas_pitchtrack_setbias(vas_pitchtrack *x, float bias);
void vas_pitchtrack_setminRMS(vas_pitchtrack *x, float rms);

float vas_pitchtrack_getperiod(vas_pitchtrack *x);
float vas_pitchtrack_getfidelity(vas_pitchtrack *x);

void vas_pitchtrack_analyzeframe(vas_pitchtrack *x);
void vas_pitchtrack_autocorrelation(vas_pitchtrack *x);
void vas_pitchtrack_normalize(vas_pitchtrack *x);
void vas_pitchtrack_pickpeak(vas_pitchtrack *x);
void vas_pitchtrack_periodandfidelity(vas_pitchtrack *x);

float vas_pitchtrack_interpolate3max(float *buf, int peakindex);
float interpolate3phase(float *buf, int peakindex);
    
#ifdef __cplusplus
}
#endif

#endif /* vas_pitchtrack_h */
