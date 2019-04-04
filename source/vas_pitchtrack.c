

#include "vas_pitchtrack.h"

vas_pitchtrack *vas_pitchtrack_new(int frame, int overlap, float bias)
{
    vas_pitchtrack *x = ( vas_pitchtrack *) vas_mem_alloc(sizeof(vas_pitchtrack));
    
    x->inputbuf = NULL;
    x->inputbuf2 = NULL;
    x->processbuf = NULL;
    
    vas_pitchtrack_setframesize(x, frame);
    vas_pitchtrack_setoverlap(x, overlap);
    if(bias)
        vas_pitchtrack_setbias(x, bias);
    else
        x->biasfactor = DEFBIAS;
    
    x->inputbuf = (t_float*)calloc(x->framesize, sizeof(t_float));
    x->inputbuf2 = (t_float*)calloc(x->framesize, sizeof(t_float));
    x->processbuf = (t_float*)calloc(x->framesize * 2, sizeof(t_float));
    
    x->timeindex = 0;
    x->periodindex = 0;
    x->periodlength = 0.;
    x->fidelity = 0.;
    x->minrms = DEFMINRMS;
    
    
    return x;
}

void vas_pitchtrack_iosamples(vas_pitchtrack *x, float *in, float *out, int size)
{
    int mask = x->framesize - 1;
    int outindex = 0;
    
    // call analysis function when it is time
    if(!(x->timeindex & (x->framesize / x->overlap - 1)))
        vas_pitchtrack_analyzeframe(x);
    
    while(size--)
    {
        x->inputbuf[x->timeindex] = *in++;
        out[outindex++] = x->processbuf[x->timeindex++];
        x->timeindex &= mask;
    }
}

void vas_pitchtrack_setframesize(vas_pitchtrack *x, int frame)
{
    if(!((frame==128)|(frame==256)|(frame==512)|(frame==1024)|(frame==2048)))
        frame = DEFFRAMESIZE;
    x->framesize = frame;
    
    if(x->inputbuf)
        x->inputbuf = (t_float*)realloc(x->inputbuf, x->framesize * sizeof(t_float));
    if(x->inputbuf2)
        x->inputbuf2 = (t_float*)realloc(x->inputbuf2, x->framesize * sizeof(t_float));
    if(x->processbuf)
        x->processbuf = (t_float*)realloc(x->processbuf, x->framesize * 2 * sizeof(t_float));
    
    x->timeindex = 0;
}

void vas_pitchtrack_setoverlap(vas_pitchtrack *x, int overlap)
{
    if(!((overlap==1)|(overlap==2)|(overlap==4)|(overlap==8)))
        overlap = DEFOVERLAP;
    x->overlap = overlap;
    
}

void vas_pitchtrack_setbias(vas_pitchtrack *x, float bias)
{
    if(bias > 1.)
        bias = 1.;
    if(bias < 0.)
        bias = 0.;
    x->biasfactor = bias;
}

void vas_pitchtrack_setminRMS(vas_pitchtrack *x, float rms)
{
    if(rms > 1.)
        rms = 1.;
    if(rms < 0.)
        rms = 0.;
    x->minrms = rms;
}

float vas_pitchtrack_getperiod(vas_pitchtrack *x)
{
    return x->periodlength;
}

float vas_pitchtrack_getfidelity(vas_pitchtrack *x)
{
    return x->fidelity;
}

void vas_pitchtrack_analyzeframe(vas_pitchtrack *x)
{
    int n, tindex = x->timeindex;
    int mask = x->framesize - 1;
    int peak;
    float norm = 1. / sqrt((float)(x->framesize * 2));
    
    // copy input to processing buffer
    for(n=0; n<x->framesize; n++)
        x->processbuf[n] = x->inputbuf[tindex++ & mask] * norm;
    
    // copy for normalization function
    for(n=0; n<x->framesize; n++)
        x->inputbuf2[n] = x->inputbuf[tindex++ & mask];
    
    // zeropadding
    for(n=x->framesize; n<(x->framesize<<1); n++)
        x->processbuf[n] = 0.;
    
    // call analysis procedures
    vas_pitchtrack_autocorrelation(x);
    vas_pitchtrack_normalize(x);
    vas_pitchtrack_pickpeak(x);
    vas_pitchtrack_periodandfidelity(x);
}

void vas_pitchtrack_autocorrelation(vas_pitchtrack *x)
{
    int n;
    int fftsize = x->framesize * 2;
    
    REALFFT(fftsize, x->processbuf);
    
    // compute power spectrum
    x->processbuf[0] *= x->processbuf[0]; // DC
    x->processbuf[x->framesize] *= x->processbuf[x->framesize]; // Nyquist
    
    for(n=1; n<x->framesize; n++)
    {
        x->processbuf[n] = x->processbuf[n] * x->processbuf[n]
        + x->processbuf[fftsize-n] * x->processbuf[fftsize-n]; // imag coefficients appear reversed
        x->processbuf[fftsize-n] = 0.;
        
    }
    
    REALIFFT(fftsize, x->processbuf);
}

void vas_pitchtrack_normalize(vas_pitchtrack *x)
{
    int n, mask = x->framesize - 1;
    int seek = x->framesize * SEEK;
    t_float signal1, signal2;
    
    // minimum RMS implemented as minimum autocorrelation at index 0
    // effectively this means possible white noise addition
    float rms = x->minrms / sqrt(1. / (float)x->framesize);
    float minrzero = rms * rms;
    float rzero = x->processbuf[0];
    if(rzero < minrzero)
        rzero = minrzero;
    double normintegral = rzero * 2.;
    
    // normalize biased autocorrelation function
    x->processbuf[0] = 1.;
    for(n=1; n<seek; n++)
    {
        signal1 = x->inputbuf2[n-1];
        signal2 = x->inputbuf2[x->framesize-n];
        normintegral -= (double)(signal1 * signal1 + signal2 * signal2);
        x->processbuf[n] /= (float)normintegral * 0.5;
    }
    
    // flush instable function tail
    for(n = seek; n<x->framesize; n++)
        x->processbuf[n] = 0.;
}

void vas_pitchtrack_pickpeak(vas_pitchtrack *x)
{
    int n, peakindex=0;
    int seek = x->framesize * SEEK;
    float maxvalue = 0.;
    float previous[2];
    float bias = x->biasfactor / (float)x->framesize;    // user-controlled bias
    float realpeak;
    
    // skip main lobe
    for(n=1; n<seek; n++)
    {
        if(x->processbuf[n] < 0.)
            break;
    }
    
    // find interpolated / biased maximum in specially normalized autocorrelation function
    // interpolation finds the 'real maximum'
    // biasing favours the first candidate
    for(; n<seek-1; n++)
    {
        if(x->processbuf[n] > x->processbuf[n-1])
        {
            if(x->processbuf[n] > x->processbuf[n+1]) // we have a local peak
            {
                realpeak = vas_pitchtrack_interpolate3max(x->processbuf, n);
                
                if((realpeak * (1. - n * bias)) > maxvalue)
                {
                    maxvalue = realpeak;
                    peakindex = n;
                }
            }
        }
    }
    x->periodindex = peakindex;
    
}
void vas_pitchtrack_periodandfidelity(vas_pitchtrack *x)
{
    if(x->periodindex)
    {
        x->periodlength = x->periodindex + interpolate3phase(x->processbuf, x->periodindex);
        x->fidelity = vas_pitchtrack_interpolate3max(x->processbuf, x->periodindex);
    }
}

float vas_pitchtrack_interpolate3max(float *buf, int peakindex)
{
    float realpeak;
    
    float a = buf[peakindex-1];
    float b = buf[peakindex];
    float c = buf[peakindex+1];
    
    realpeak = b + 0.5 * (0.5 * ((c - a) * (c - a)))
    / (2 * b - a - c);
    
    return(realpeak);
    
}
float interpolate3phase(float *buf, int peakindex)
{
    float fraction;
    
    float a = buf[peakindex-1];
    float b = buf[peakindex];
    float c = buf[peakindex+1];
    fraction = (0.5 * (c - a)) / ( 2. * b - a - c);
    
    return(fraction);
}
