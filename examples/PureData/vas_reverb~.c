#include "vas_reverb~.h"

static t_class *vas_reverb_class;

static t_int *vas_reverb_perform(t_int *w)
{
    vas_reverb *x = (vas_reverb *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    t_float *outR = (t_float *)(w[4]);
    t_float *inputBufferPtr = x->inputBuffer;
    
    int n = (int)(w[5]);
    
    while(n--)
        *inputBufferPtr++ = *in++;
    
    n = (int)(w[5]);
    
    vas_fir_reverb_process(x->convolutionEngine, x->inputBuffer, outL, outR, n);
    
    return (w+6);
}

static void vas_reverb_dsp(vas_reverb *x, t_signal **sp)
{
    dsp_add(vas_reverb_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void vas_reverb_free(vas_reverb *x)
{
    vas_fir_reverb_free(x->convolutionEngine);
    outlet_free(x->outL);
    outlet_free(x->outR);
}

static void *vas_reverb_new(t_symbol *s, int argc, t_atom *argv)
{
    vas_reverb *x = (vas_reverb *)pd_new(vas_reverb_class);
    
    t_symbol *path = NULL;
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->outR = outlet_new(&x->x_obj, gensym("signal"));
    
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
        rwa_firobject_read2((rwa_firobject *)x, path, x->segmentSize, 0, 0);

    return (x);
}

void vas_reverb_tilde_setup(void)
{
    vas_reverb_class = class_new(gensym("vas_reverb~"), (t_newmethod)vas_reverb_new, (t_method)vas_reverb_free,
    	sizeof(vas_reverb), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_reverb~ v0.6");
   
    CLASS_MAINSIGNALIN(vas_reverb_class, vas_reverb, f);
   
    class_addmethod(vas_reverb_class, (t_method)vas_reverb_dsp, gensym("dsp"), 0);
    class_addmethod(vas_reverb_class, (t_method)rwa_firobject_read2, gensym("read"), A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(vas_reverb_class, (t_method)vas_firobject_set, gensym("set"), A_DEFSYM, A_DEFSYM, 0);
}
