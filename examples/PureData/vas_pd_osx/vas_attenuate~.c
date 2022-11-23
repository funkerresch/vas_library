#include "vas_attenuate~.h"

static t_class *vas_attenuate_class;

static t_int *vas_attenuate_perform(t_int *w)
{
    vas_attenuate *x = (vas_attenuate *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    vas_attenuation_perform(x->attenuate, in, out, n);
    return (w+5);
}

static void vas_attenuate_dsp(vas_attenuate *x, t_signal **sp)
{
    dsp_add(vas_attenuate_perform, 4, x,  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void vas_attenuate_free(vas_attenuate *x)
{
    vas_attenuation_free(x->attenuate);
    outlet_free(x->outL);
}

static void *vas_attenuate_new(t_symbol *s, int argc, t_atom *argv)
{
    vas_attenuate *x = (vas_attenuate *)pd_new(vas_attenuate_class);
    x->attenuate = vas_attenuation_new(250);
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->f = 0;
    return (x);
}

static void vas_attenuate_setDistance(vas_attenuate *x, float distance)
{
    vas_attenuation_setDistance(x->attenuate, distance);
}

static void vas_attenuate_setMaxDistance(vas_attenuate *x, float maxDistance)
{
    vas_attenuation_setMaxDistance(x->attenuate, maxDistance);
}

static void vas_attenuate_setMinDistance(vas_attenuate *x, float minDistance)
{
    vas_attenuation_setMinDistance(x->attenuate, minDistance);
}

void vas_attenuate_tilde_setup(void)
{
    vas_attenuate_class = class_new(gensym("vas_attenuate~"), (t_newmethod)vas_attenuate_new, (t_method)vas_attenuate_free,
    	sizeof(vas_attenuate), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_attenuate~ v0.2");
   
    CLASS_MAINSIGNALIN(vas_attenuate_class, vas_attenuate, f);
   
    class_addmethod(vas_attenuate_class, (t_method)vas_attenuate_dsp, gensym("dsp"), 0);
    class_addmethod(vas_attenuate_class, (t_method)vas_attenuate_setDistance, gensym("distance"), A_DEFFLOAT,0);
    class_addmethod(vas_attenuate_class, (t_method)vas_attenuate_setMaxDistance, gensym("maxdistance"), A_DEFFLOAT,0);
    class_addmethod(vas_attenuate_class, (t_method)vas_attenuate_setMinDistance, gensym("mindistance"), A_DEFFLOAT,0);
}
