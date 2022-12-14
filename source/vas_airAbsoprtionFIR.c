//
//  vas_airAbsoprtionFIR.c
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 19.01.21.
//

#include "vas_airAbsoprtionFIR.h"
#include "vas_airAbsorption.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    void vas_airAbsorptionFIR_update_ir(vas_airAbsorptionFIR *x, float distance, int N) {
        /*// design filter based on distance and current atmospheric properties
       // 'Frequency sampling' method
       VAS_COMPLEX *H = vas_mem_alloc(sizeof(VAS_COMPLEX));
        H->realp = vas_mem_alloc(N * sizeof(float));
        H->imagp = vas_mem_alloc(N * sizeof(float));
       float df = x->w->sampleRate / N;
        
        vas_util_complexWriteZeros(H, N);

       // sample in (0,fs/2) range
       for (int i = 0; i < N / 2 + 1; ++i)
       {
           float a = (float)vas_airAbsorptionFilter_get_abs_coeff(x->w, df * i);
           H->realp[i] = powf(10, -a * distance / 20);
       }

       //mirror DFT with respect to N/2+1 sample
        for (int i = N / 2 + 1; i < N; ++i) {
            //H[i] = H[N - i]; //this was the original code, but this is just pointer copying, isn't it?!
            H->realp[i] = H->realp[N - i]; //we don't need imag part yet
        }

       //do IFFT to get impulse response
       FourierTransform.FFT(H, FourierTransform.Direction.Backward);

       //Aforge's FFT/IFFT is not normalized, divide by N
       for (int i = 0; i < N; ++i)
           H->realp[i] /= N;

       //impulse response
       x->ir = H;
        x->irLength = N;
       //shift by N/2
       Util.shiftArray<Complex>(x->ir, N / 2);

        vas_util_fwindow(vas_window_blackman(N), x->ir->realp, N);*/
    }
    
    
    /* PUBLIC FUNCTIONS */

    
    vas_airAbsorptionFIR *vas_airAbsorptionFIR_new(vas_airAbsorptionFilter *w) {
        vas_airAbsorptionFIR *x = vas_mem_alloc(sizeof(vas_airAbsorptionFIR));
        x->w = w;
        x->ir = NULL;
        x->irLength = 0;
        return (void*)x;
    }
        
    void vas_airAbsorptionFIR_free(vas_airAbsorptionFIR *x) {
        if(x->ir != NULL) {
#ifdef VAS_USE_VDSP
            if(x->ir->realp != NULL) vas_mem_free(x->ir->realp);
            if(x->ir->imagp != NULL) vas_mem_free(x->ir->imagp);
#endif
            vas_mem_free(x->ir);
        }
        vas_mem_free(x);
    }
    
    void vas_airAbsorbtionFIR_update_t_h_p(vas_airAbsorptionFIR *x) {
        
    }
    
    void vas_airAbsorptionFIR_update_d(vas_airAbsorptionFIR *x) {
        vas_airAbsorptionFIR_update_ir(x, x->w->d, 0); //what is N?!
    }
        
    void vas_airAbsorptionFIR_perform(vas_airAbsorptionFIR *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *out, int vectorSize) {
        /*//this function expects de-interleaved audio buffers.
            
        //convolution using OVERLAP-ADD

        // get length that arrays will be zero-padded to
        //TODO: only do this once in vas_airAbsorptionFIR_update_ir function. have to know vectorSize though!
        int K = vas_util_roundUp2NextPowerOf2(vectorSize + x->irLength - 1);

        //create temporary (zero padded to K) arrays
        VAS_COMPLEX *ir_pad = vas_mem_alloc(sizeof(VAS_COMPLEX));
        ir_pad->realp = vas_mem_alloc(K * sizeof(float));
        ir_pad->imagp = vas_mem_alloc(K * sizeof(float));
        
        System.Array.Copy(m_ImpulseResponse, ir_pad, m_ImpulseResponse.Length);
        VAS_COMPLEX *data_pad = vas_mem_alloc(sizeof(VAS_COMPLEX));
        data_pad->realp = vas_mem_alloc(K * sizeof(float));
        data_pad->imagp = vas_mem_alloc(K * sizeof(float));
        for (int i = 0; i < data.Length; ++i)
           data_pad->realp[i] = in[i];

        //FFT
        FourierTransform.FFT(data_pad, FourierTransform.Direction.Forward);
        FourierTransform.FFT(ir_pad, FourierTransform.Direction.Forward);
        //convolution
        Complex[] ifft = new Complex[K];
        for (int i = 0; i < ifft.Length; ++i)
           ifft[i] = data_pad[i] * ir_pad[i] * K;
        FourierTransform.FFT(ifft, FourierTransform.Direction.Backward);
        //add from buffer
        for (int i = 0; i < data.Length; ++i)
        {
           data[i] = (float)ifft[i].Re;
           if (i < m_buffers[channel].Length)
               data[i] += m_buffers[channel][i];
        }
        //buffer last (K - data.length) samples
        //TODO: how large does the buffer have to be?
        m_buffers[channel] = new float[K - data.Length];
        for (int i = 0; i < m_buffers[channel].Length; ++i)
        m_buffers[channel][i] = (float)ifft[i + data.Length].Re;*/
    }
    
#ifdef __cplusplus
}
#endif
