#include "vas_del~.h"

static t_class *vas_del_class;

static t_int *vas_del_perform(t_int *w)
{
    vas_del *x = (vas_del *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    vas_ringBuffer_process(x->buffer, in, n);
    vas_delayTap_crossfade_process(x->delayEngine,  out, n);
    return (w+5);
}

static void vas_del_dsp(vas_del *x, t_signal **sp)
{
    dsp_add(vas_del_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void vas_del_free(vas_del *x)
{
    vas_delayTap_crossfade_free(x->delayEngine);
    vas_ringBuffer_free(x->buffer);
    outlet_free(x->outL);
}

static void *vas_del_new(t_symbol *s, int argc, t_atom *argv)
{
    vas_del *x = (vas_del *)pd_new(vas_del_class);
    x->buffer = vas_ringBuffer_new(44100);
    x->delayEngine = vas_delayTap_crossfade_new(x->buffer);
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->f = 0;
    return (x);
}

static void vas_del_setDelayTime(vas_del *x, float delayTime)
{
    vas_delayTap_crossfade_setDelayTime(x->delayEngine, delayTime);
}

void vas_del_tilde_setup(void)
{
    vas_del_class = class_new(gensym("vas_del~"), (t_newmethod)vas_del_new, (t_method)vas_del_free,
    	sizeof(vas_del), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_del~ v0.2");
   
    CLASS_MAINSIGNALIN(vas_del_class, vas_del, f);
   
    class_addmethod(vas_del_class, (t_method)vas_del_dsp, gensym("dsp"), 0);
    class_addmethod(vas_del_class, (t_method)vas_del_setDelayTime, gensym("delaytime"), A_DEFFLOAT,0);
}
