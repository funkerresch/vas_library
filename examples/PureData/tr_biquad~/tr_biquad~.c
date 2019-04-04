/**
 * @file tr_biquad_pd.c
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * A biquad filter for pure data <br>
 * <br>
 * @brief A Pure Data biquad object<br>
 * <br>
 * tr_biquad~ <br>
 * <br>
 * <br>
 */

#include "m_pd.h"
#include "vas_iir_biquad.h"

static t_class *tr_biquad_tilde_class;

/**
 * @struct tr_biquad_tilde
 * @brief The Pure Data struct of the tr_biquad~ object. <br>
 * @var tr_biquad_tilde::x_obj Necessary for every signal object in Pure Data <br>
 * @var tr_biquad_tilde::f Also necessary for signal objects, float dummy dataspace <br>
 * for converting a float to signal if no signal is connected (CLASS_MAINSIGNALIN) <br>
 * @var tr_biquad_tilde::gain The gain object for the actual signal processing <br>
 * @var tr_biquad_tilde::x_out A signal outlet for the adjusted signal
 * level of the incoming signal
 */

typedef struct tr_biquad_tilde
{
    t_object  x_obj;
    t_sample f;
    t_outlet *x_out;
    vas_iir_biquad *hp; //example hp implementation
} tr_biquad_tilde;

/**
 * @related tr_biquad_tilde
 * @brief Calculates the volume adjusted output vector<br>
 * @param w A pointer to the object, input and output vectors. <br>
 * For more information please refer to the Pure Data Docs <br>
 * The function calls the tr_biquad_perform method. <br>
 * @return A pointer to the signal chain right behind the tr_biquad_tilde object. <br>
 */

t_int *tr_biquad_tilde_perform(t_int *w)
{
    tr_biquad_tilde *x = (tr_biquad_tilde *)(w[1]);
    t_sample  *in = (t_sample *)(w[2]);
    t_sample  *out =  (t_sample *)(w[3]);
    int n =  (int)(w[4]);

    vas_iir_biquad_process(x->hp, in, out, n);

    /* return a pointer to the dataspace for the next dsp-object */
    return (w+5);
}

/**
 * @related tr_biquad_tilde
 * @brief Adds tr_biquad_tilde_perform to the signal chain. <br>
 * @param x A pointer the tr_biquad_tilde object <br>
 * @param sp A pointer the input and output vectors <br>
 * For more information please refer to the <a href = "https://github.com/pure-data/externals-howto" > Pure Data Docs </a> <br>
 */

void tr_biquad_tilde_dsp(tr_biquad_tilde *x, t_signal **sp)
{
    dsp_add(tr_biquad_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

/**
 * @related tr_biquad_tilde
 * @brief Frees our object. <br>
 * @param x A pointer the tr_biquad_tilde object <br>
 * For more information please refer to the <a href = "https://github.com/pure-data/externals-howto" > Pure Data Docs </a> <br>
 */

void tr_biquad_tilde_free(tr_biquad_tilde *x)
{
    outlet_free(x->x_out);
    // vas_iir_biquad_free missing
}

/**
 * @related tr_biquad_tilde
 * @brief Creates a new tr_biquad_tilde object.<br>
 * @param f Sets the initial gain value. <br>
 * For more information please refer to the <a href = "https://github.com/pure-data/externals-howto" > Pure Data Docs </a> <br>
 */

void *tr_biquad_tilde_new(t_floatarg f)
{
    tr_biquad_tilde *x = (tr_biquad_tilde *)pd_new(tr_biquad_tilde_class);
    
    //The main inlet is created automatically
    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->hp = vas_iir_biquad_new(VAS_IIR_BIQUAD_HIGHPASS, 5000, 10);


    return (void *)x;
}

/**
 * @related tr_biquad_tilde
 * @brief Sets the f0 parameter. <br>
 * @param x A pointer the tr_biquad_tilde object <br>
 * @param level Sets the level parameter <br>
 * For more information please refer to the <a href = "https://github.com/pure-data/externals-howto" > Pure Data Docs </a> <br>
 */

void tr_biquad_tilde_setFrequency(tr_biquad_tilde *x, float level)
{
    //missing
}

/**
 * @related tr_biquad_tilde
 * @brief Setup of tr_biquad_tilde <br>
 * For more information please refer to the <a href = "https://github.com/pure-data/externals-howto" > Pure Data Docs </a> <br>
 */

void tr_biquad_tilde_setup(void)
{
      tr_biquad_tilde_class = class_new(gensym("tr_biquad~"),
            (t_newmethod)tr_biquad_tilde_new,
            (t_method)tr_biquad_tilde_free,
        sizeof(tr_biquad_tilde),
            CLASS_DEFAULT,
            A_DEFFLOAT, 0);

      class_addmethod(tr_biquad_tilde_class, (t_method)tr_biquad_tilde_dsp, gensym("dsp"), 0);

      // this adds the gain message to our object
      class_addmethod(tr_biquad_tilde_class, (t_method)tr_biquad_tilde_setFrequency, gensym("gain"), A_DEFFLOAT,0);

      CLASS_MAINSIGNALIN(tr_biquad_tilde_class, tr_biquad_tilde, f);
}
