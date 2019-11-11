#include "rwa_binauralsimple~.h"

static t_class *rwa_binauralsimple_class;

static void rwa_binauralsimple_setAzimuth(rwa_binauralsimple *x, float azimuth)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setAzimuth(binauralEngine->left, azimuth);
    vas_dynamicFirChannel_setAzimuth(binauralEngine->right, azimuth);
}

static void rwa_binauralsimple_setElevation(rwa_binauralsimple *x, float elevation)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setElevation(binauralEngine->left, elevation);
    vas_dynamicFirChannel_setElevation(binauralEngine->right, elevation);
}

static t_int *rwa_binauralsimple_perform(t_int *w)
{
    rwa_binauralsimple *x = (rwa_binauralsimple *)(w[1]) ;
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

static void rwa_binauralsimple_dsp(rwa_binauralsimple *x, t_signal **sp)
{
    dsp_add(rwa_binauralsimple_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void rwa_binauralsimple_free(rwa_binauralsimple *x)
{
    vas_fir_binaural_free(x->convolutionEngine);
    inlet_free(x->azi);
    inlet_free(x->ele);
    outlet_free(x->outL);
    outlet_free(x->outR);
}

static void *rwa_binauralsimple_new(t_symbol *s, int argc, t_atom *argv)
{
    rwa_binauralsimple *x = (rwa_binauralsimple *)pd_new(rwa_binauralsimple_class);
    
    t_symbol *path = NULL;
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->outR = outlet_new(&x->x_obj, gensym("signal"));
    x->azi = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("azimuth"));
    x->ele = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("elevation"));
    
    x->segmentSize = 256;
    x->filterSize = 0;
    x->f = 0;
    x->fullpath[0] = '\0';
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
    
    x->convolutionEngine = vas_fir_binaural_new(0);

    if(!x->convolutionEngine)
    {
        post("Could not create rwa_binauralrir.");
        free(x);
        return NULL;
    }
    
    if(argc >= 2)
    {
        if(argv[1].a_type == A_SYMBOL)
            path = atom_getsymbolarg(1, argc, argv);
    }
    
    if(path)
        rwa_firobject_read2((rwa_firobject *)x, path, x->segmentSize, 0);

    return (x);
}

void rwa_binauralsimple_tilde_setup(void)
{
    rwa_binauralsimple_class = class_new(gensym("rwa_binauralsimple~"), (t_newmethod)rwa_binauralsimple_new, (t_method)rwa_binauralsimple_free,
    	sizeof(rwa_binauralsimple), CLASS_DEFAULT, A_GIMME, 0);
    
    post("rwa_binauralsimple~ v0.7");
   
    CLASS_MAINSIGNALIN(rwa_binauralsimple_class, rwa_binauralsimple, f);
   
    class_addmethod(rwa_binauralsimple_class, (t_method)rwa_binauralsimple_dsp, gensym("dsp"), 0);
    class_addmethod(rwa_binauralsimple_class, (t_method)rwa_binauralsimple_setAzimuth, gensym("azimuth"), A_DEFFLOAT,0);
    class_addmethod(rwa_binauralsimple_class, (t_method)rwa_binauralsimple_setElevation, gensym("elevation"), A_DEFFLOAT,0);
    class_addmethod(rwa_binauralsimple_class, (t_method)rwa_firobject_read2, gensym("read"), A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
}

