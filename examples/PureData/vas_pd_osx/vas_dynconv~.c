#include "vas_dynconv~.h"

static t_class *vas_dynconv_class;

static t_int *vas_dynconv_perform(t_int *w)
{
    vas_dynconv *x = (vas_dynconv *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    t_float *inputBufferPtr = x->inputBuffer;
    
    int n = (int)(w[4]);
    
    while(n--)
        *inputBufferPtr++ = *in++;
    
    n = (int)(w[4]);
    
    vas_fir_dynconv_process(x->convolutionEngine, x->inputBuffer, outL, n);
    
    return (w+5);
}

static void vas_dynconv_dsp(vas_dynconv *x, t_signal **sp)
{
    dsp_add(vas_dynconv_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void vas_dynconv_free(vas_dynconv *x)
{
    vas_fir_reverb_free(x->convolutionEngine);
    
    outlet_free(x->outL);
    
    if(x->leftArray)
        vas_mem_free(x->leftArray);
    if(x->rightArray)
        vas_mem_free(x->rightArray);
}

static void *vas_dynconv_new(t_symbol *s, int argc, t_atom *argv)
{
    vas_dynconv *x = (vas_dynconv *)pd_new(vas_dynconv_class);
    
    t_symbol *path = NULL;
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    
    x->segmentSize = 256;
    x->filterSize = 0;
    x->f = 0;
    x->fullpath[0] = '\0';
    
    x->leftArray = NULL;
    x->rightArray = NULL;
    
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
        rwa_firobject_read2((rwa_firobject *)x, path, x->segmentSize, 0, 0);

    return (x);
}

static void vas_dynconv_setIndex(vas_dynconv *x, float index)
{
    if(index <= 0)
        return;
          
    vas_fir_reverb *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_selectIR(binauralEngine->left, index);
}

void vas_dynconv_tilde_setup(void)
{
    vas_dynconv_class = class_new(gensym("vas_dynconv~"), (t_newmethod)vas_dynconv_new, (t_method)vas_dynconv_free,
    	sizeof(vas_dynconv), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_dynconv~ v0.1");
   
    CLASS_MAINSIGNALIN(vas_dynconv_class, vas_dynconv, f);
   
    class_addmethod(vas_dynconv_class, (t_method)vas_dynconv_dsp, gensym("dsp"), 0);
    class_addmethod(vas_dynconv_class, (t_method)vas_firobject_set_mono_simple, gensym("set"), A_DEFSYM, A_DEFFLOAT, 0);
    class_addmethod(vas_dynconv_class, (t_method)vas_firobject_prepareForInterpolatedIrs, gensym("configure"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(vas_dynconv_class, (t_method)vas_firobject_setAndInterpolateBetweenIndexes1, gensym("setandinterpolate"), A_GIMME, 0);
    class_addmethod(vas_dynconv_class, (t_method)vas_dynconv_setIndex, gensym("index"), A_DEFFLOAT,0);
}
