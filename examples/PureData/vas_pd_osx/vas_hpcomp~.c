#include "vas_hpcomp~.h"

static t_class *vas_hpcomp_class;

static t_int *vas_hpcomp_perform(t_int *w)
{
    vas_hpcomp *x = (vas_hpcomp *)(w[1]) ;
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

static void vas_hpcomp_dsp(vas_hpcomp *x, t_signal **sp)
{
    dsp_add(vas_hpcomp_perform, 6, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

static void vas_hpcomp_free(vas_hpcomp *x)
{
    vas_fir_binaural_free(x->convolutionEngine);
    outlet_free(x->outL);
    outlet_free(x->outR);
    inlet_free(x->in2);
}

static void *vas_hpcomp_new(t_symbol *s, int argc, t_atom *argv)
{
    vas_hpcomp *x = (vas_hpcomp *)pd_new(vas_hpcomp_class);
    
    t_symbol *path = NULL;
  
    x->in2  = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
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
        post("Could not create hpcomp~");
        free(x);
        return NULL;
    }
    
    if(argc >= 2)
    {
        if(argv[1].a_type == A_SYMBOL)
            path = atom_getsymbolarg(1, argc, argv);
    }
    
    if(path)
        vas_pdmaxobject_read((vas_pdmaxobject *)x, path, x->segmentSize, 0, 0);
    
    return (x);
}

void vas_hpcomp_tilde_setup(void)
{
    vas_hpcomp_class = class_new(gensym("vas_hpcomp~"), (t_newmethod)vas_hpcomp_new, (t_method)vas_hpcomp_free,
                                 sizeof(vas_hpcomp), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_hpcomp~ v0.6");
    
    CLASS_MAINSIGNALIN(vas_hpcomp_class, vas_hpcomp, f);
    
    class_addmethod(vas_hpcomp_class, (t_method)vas_hpcomp_dsp, gensym("dsp"), 0);
    class_addmethod(vas_hpcomp_class, (t_method)vas_pdmaxobject_read, gensym("read"), A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
}
