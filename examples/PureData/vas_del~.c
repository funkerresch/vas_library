#include "vas_del~.h"

static t_class *vas_del_class;

static t_int *vas_del_perform(t_int *w)
{
    vas_del *x = (vas_del *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    int n = (int)(w[4]);
    vas_delay_crossfade_process(x->delayEngine, in, outL, n);
    return (w+5);
}

static void vas_del_dsp(vas_del *x, t_signal **sp)
{
    dsp_add(vas_del_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void vas_del_free(vas_del *x)
{
    vas_mem_free(x->delayEngine);
    outlet_free(x->outL);
}

static void *vas_del_new(t_symbol *s, int argc, t_atom *argv)
{
    vas_del *x = (vas_del *)pd_new(vas_del_class);
    x->delayEngine = vas_delay_crossfade_new(44100);
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->lastDelayTime = 0;
    x->targetDelayTime = 0;
    x->f = 0;
    return (x);
}

static void vas_del_setDelayTime(vas_del *x, float delayTime)
{
    x->targetDelayTime = delayTime;
    if(fabs(x->targetDelayTime-x->lastDelayTime) > 256)
    {
        vas_delay_crossfade_setDelayTime(x->delayEngine, delayTime);
        x->lastDelayTime = x->targetDelayTime;
    }
}

void vas_del_tilde_setup(void)
{
    vas_del_class = class_new(gensym("vas_del~"), (t_newmethod)vas_del_new, (t_method)vas_del_free,
    	sizeof(vas_del), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_del~ v0.1");
   
    CLASS_MAINSIGNALIN(vas_del_class, vas_del, f);
   
    class_addmethod(vas_del_class, (t_method)vas_del_dsp, gensym("dsp"), 0);
    class_addmethod(vas_del_class, (t_method)vas_del_setDelayTime, gensym("delaytime"), A_DEFFLOAT,0);
}
