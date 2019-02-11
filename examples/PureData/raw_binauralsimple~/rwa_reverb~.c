#include "rwa_reverb~.h"

static t_class *rwa_reverb_class;

static t_int *rwa_reverb_perform(t_int *w)
{
    rwa_reverb *x = (rwa_reverb *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    t_float *outR = (t_float *)(w[4]);
    t_float *inputBufferPtr = x->inputBuffer;
    int n = (int)(w[5]);
    
    while(n--)
        *inputBufferPtr++ = *in++;
    
    n = (int)(w[5]);
    
    vas_fir_binaural_process(x->convolutionEngine, x->inputBuffer, outL, outR, n);
    
    return (w+6);
}

static void rwa_reverb_dsp(rwa_reverb *x, t_signal **sp)
{
    dsp_add(rwa_reverb_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void rwa_reverb_free(rwa_reverb *x)
{
    vas_fir_binaural_free(x->convolutionEngine);
}

static void *rwa_reverb_new(t_symbol *s, int argc, t_atom *argv)
{
    rwa_reverb *x = (rwa_reverb *)pd_new(rwa_reverb_class);
    
    t_symbol *path = NULL;

    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->outR = outlet_new(&x->x_obj, gensym("signal"));
    x->segmentSize = 256;
    x->filterSize = 0;
    x->useGlobalFilter = 1;
    x->f = 0;
    
    sprintf(x->canvasDirectory, "%s", canvas_getcurrentdir()->s_name);
    
    if(argc >= 1)
    {
        if(argv[0].a_type == A_FLOAT)
        {
            x->segmentSize = (float)atom_getintarg(0, argc, argv);
        }
        
        if(argv[0].a_type == A_SYMBOL)
            path = atom_getsymbolarg(0, argc, argv);
    }
    
    x->convolutionEngine = vas_fir_binaural_new(VAS_VDSP | VAS_STATICFILTER, x->segmentSize, NULL);
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    if(!x->convolutionEngine)
    {
        post("Could not create rwa_reverb.");
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
           // post("OVERALL ENERGY: %.14f", binauralEngine->right->filter->overallEnergy[0][0] );
        }
        else
        {
            vas_dynamicFirChannel_initArraysWithGlobalFilter(binauralEngine->left);
            vas_dynamicFirChannel_initArraysWithGlobalFilter(binauralEngine->right);
        }
    }
    
    return (x);
}

void rwa_reverb_tilde_setup(void)
{
    rwa_reverb_class = class_new(gensym("rwa_reverb~"), (t_newmethod)rwa_reverb_new, (t_method)rwa_reverb_free,
    	sizeof(rwa_reverb), CLASS_DEFAULT, A_GIMME, 0);
    
    post("rwa_reverb~ v0.5");
   
    CLASS_MAINSIGNALIN(rwa_reverb_class, rwa_reverb, f);
   
    class_addmethod(rwa_reverb_class, (t_method)rwa_reverb_dsp, gensym("dsp"), 0);
    class_addmethod(rwa_reverb_class, (t_method)rwa_firobject_read, gensym("read"), A_DEFSYM,0);
}
