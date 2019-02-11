#include "rwa_hpcomp~.h"

static t_class *rwa_hpcomp_class;

static t_int *rwa_hpcomp_perform(t_int *w)
{
    rwa_hpcomp *x = (rwa_hpcomp *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *inR = (t_float *)(w[3]);
    t_float *outL = (t_float *)(w[4]);
    t_float *outR = (t_float *)(w[5]);
    t_float *inputBufferPtr = x->inputBuffer;
    t_float *inputBufferPtr2 = x->inputBuffer2;
    int n = (int)(w[6]);
    
    while(n--)
    {
        *inputBufferPtr++ = *in++;
        *inputBufferPtr2++ = *inR++;
    }
    
    n = (int)(w[6]);
    
    vas_fir_headphoneCompensation_process(x->convolutionEngine, x->inputBuffer, x->inputBuffer2, outL, outR, n);
    
    return (w+7);
}

static void rwa_hpcomp_dsp(rwa_hpcomp *x, t_signal **sp)
{
    dsp_add(rwa_hpcomp_perform, 6, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

static void rwa_hpcomp_free(rwa_hpcomp *x)
{
    vas_fir_binaural_free(x->convolutionEngine);
}

static void *rwa_hpcomp_new(t_symbol *s, int argc, t_atom *argv)
{
    rwa_hpcomp *x = (rwa_hpcomp *)pd_new(rwa_hpcomp_class);
    
    t_symbol *path = NULL;
    
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->outR = outlet_new(&x->x_obj, gensym("signal"));
    x->in2  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    x->segmentSize = 256;
    x->filterSize = 0;
    x->useGlobalFilter = 1;
    x->f = 0;
    
    if(argc >= 1)
    {
        if(argv[0].a_type == A_FLOAT)
        {
            x->segmentSize = (float)atom_getintarg(0, argc, argv);
        }
        
        if(argv[0].a_type == A_SYMBOL)
            path = atom_getsymbolarg(0, argc, argv);
    }
    
    x->convolutionEngine = vas_fir_headphoneCompensation_new(VAS_VDSP | VAS_STATICFILTER, x->segmentSize);
    vas_fir_headphoneCompensation *hpcompEngine = (vas_fir_headphoneCompensation *)x->convolutionEngine;
    if(!x->convolutionEngine)
    {
        post("Could not create rwa_hpcomp.");
        free(x);
        return NULL;
    }
    
    if(argc >= 2)
    {
        if(argv[1].a_type == A_SYMBOL)
            path = atom_getsymbolarg(1, argc, argv);
    }
    
    if(path)
    {
        if(!vas_fir_getInitFlag((vas_fir *)x->convolutionEngine))
        {
            rwa_firobject_read((rwa_firobject *)x, path);
            post("Read File %s", path->s_name);
        }
        else
        {
            vas_dynamicFirChannel_initArraysWithGlobalFilter(hpcompEngine->left);
            vas_dynamicFirChannel_initArraysWithGlobalFilter(hpcompEngine->right);
        }
    }
    
    return (x);
}

void rwa_hpcomp_tilde_setup(void)
{
    rwa_hpcomp_class = class_new(gensym("rwa_hpcomp~"), (t_newmethod)rwa_hpcomp_new, (t_method)rwa_hpcomp_free,
                                 sizeof(rwa_hpcomp), CLASS_DEFAULT, A_GIMME, 0);
    
    post("rwa_hpcomp~ v0.5");
    
    CLASS_MAINSIGNALIN(rwa_hpcomp_class, rwa_hpcomp, f);
    
    class_addmethod(rwa_hpcomp_class, (t_method)rwa_hpcomp_dsp, gensym("dsp"), 0);
    class_addmethod(rwa_hpcomp_class, (t_method)rwa_firobject_read, gensym("read"), A_DEFSYM,0);
}
