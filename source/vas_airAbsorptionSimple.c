//
//  vas_airAbsorptionSimple.c
//  AudioPlugin_VAS_Binaural
//
//  Created by Hannes on 19.01.21.
//

#include "vas_airAbsorptionSimple.h"

#ifdef __cplusplus
extern "C" {
#endif
        
    void vas_airAbsorptionSimple_calculate_a1_a2_a3(vas_airAbsorptionSimple *x) {
        
        //expects f_O and f_N to be calculated before execution.
        
        const double temp_normalized = x->w->t / T0;

        const double temp_norm_inv_cube =
            1.0 / (temp_normalized * temp_normalized * temp_normalized);
        const double nitrogen_relax_coefficient =
            temp_norm_inv_cube * 0.1068 * exp(-3352.0 / x->w->t);
        const double oxygen_relax_coefficient =
            temp_norm_inv_cube * 0.01275 * exp(-2239.10 / x->w->t);

        const double pressure_coefficient = (double)1.84e-11 / x->w->p;
        // factor multiplied to the absorption quantities
        const double outer_coefficient = 8.686 * sqrt(temp_normalized);

        // Re-arrange the equation as a cubic polynomial with the absorption_coefficient
        // as the constant factor -a4
        // 0 = a1*f^2 + a2*n*f^2/(n^2+f^2) + a3*o*f^2/(o^2+f^2) + a4 where
        // f is the variable frequency, n and o is nitrogen/oxygen relaxation frequencies
        x->a1 = (float) ( outer_coefficient * pressure_coefficient );
        x->a2 = (float) ( outer_coefficient * nitrogen_relax_coefficient );
        x->a3 = (float) ( outer_coefficient * oxygen_relax_coefficient );
    }

    double vas_airAbsorptionSimple_find_first_root(double a, double b, double c, double d) {
        // Trigonmetric Cubic Solver
        // a, b, c, and d are all real so at least one real root must exist
        // a is > 0.
        // Note: assumes the first root is the largest and correct solution.
        // Convert to depressed cubic using change of variable.
        double p = (3 * a*c - b*b) / (3 * a*a);
        double q = ((2.0 * b*b*b) - (9 * b*a*c) + (27 * a*d*a)) / (27 * a*a*a);

        const double theta = (3 * q * sqrt(-3 / p)) / (2 * p);
        double t0 = 2 * sqrt(-p / 3) * cos(acos(theta) / 3);

        // descriminate = -(4p^3 + 27q^2)

        // For debugging here are the other real roots:
        //const double t2 = -2 * sqrt(-p / 3) * cos(acos(-theta) / 3);
        //const double t1 = -t0 - t2;

        const double root = t0 - b / (3 * a);
        
        return root;
    }

    double vas_airAbsorptionSimple_find_root_newton(double a, double b, double c, double d, double epsilon,
        double x) {
        
        //const double discriminant = 18.0*a*b*c*d - 4.0*b*b*b*d + b*b*c*c - 4.0*a*c*c*c - 27.0*a*a*d*d;
        //assert(discriminant > 0.00001);

        double a_prime = a * 3;
        double b_prime = b * 2;
        double c_prime = c;

        double x1 = x;
        double delta = epsilon;
        while (fabs(delta) >= epsilon) {
            double cubic = ((a * x1 + b) *  x1 + c) * x1 + d;
            double quadratic = (a_prime * x1 + b_prime) * x1 + c_prime;
            delta = cubic / quadratic;
            x1 = x1 - delta;
        }
        return x1;
    }

    //cutoff gain == 3.0 default
    float vas_airAbsorptionSimple_get_cutoff_freq(vas_airAbsorptionSimple *x, const double distance, const double cutoff_gain) {

        const double absorption_coefficient = cutoff_gain / distance;
        const double a4 = -absorption_coefficient;

        const double nitrogen_sq = x->w->f_N * x->w->f_N;
        const double oxygen_sq = x->w->f_O * x->w->f_O;

        // expand the denominators (which can then be ignored) and collecting terms
        const double a = x->a1;
        const double b = x->a1 * (nitrogen_sq + oxygen_sq) + x->a2 * x->w->f_N +
            x->a3 * x->w->f_O + a4;
        const double c = x->a1 * nitrogen_sq * oxygen_sq + x->a2 * oxygen_sq * x->w->f_N +
            x->a3 * nitrogen_sq * x->w->f_O + a4 * (nitrogen_sq + oxygen_sq);
        const double d = a4 * oxygen_sq * nitrogen_sq;

        const double root = vas_airAbsorptionSimple_find_first_root(a, b, c, d);
        const double frequency_hz = sqrt(root);

        return (float)frequency_hz;
    }
    
    
    /* PUBLIC FUNCTIONS */
    
    vas_airAbsorptionSimple *vas_airAbsorbtionSimple_new(vas_airAbsorptionFilter *w) {
        vas_airAbsorptionSimple *x = vas_mem_alloc(sizeof(vas_airAbsorptionSimple));
        x->w = w;
        x->filter = vas_iir_butterworth_new(x->w->sampleRate);
        x->a1 = 0;
        x->a2 = 0;
        x->a3 = 0;
        vas_airAbsorbtionSimple_update_t_h_p(x);
        return x;
    }
    
    void vas_airAbsorbtionSimple_free(vas_airAbsorptionSimple *x) {
        vas_mem_free(x->filter);
        vas_mem_free(x);
    }
    
    void vas_airAbsorbtionSimple_perform(vas_airAbsorptionSimple *x, VAS_INPUTBUFFER* in, VAS_OUTPUTBUFFER* out, int vectorSize) {
        
        //if there is no attenuation, don't touch the samples.
        if(x->filter->fc >= x->w->sampleRate/2) {
            if(in == out) return;
            else {
                vas_util_fcopy(in, out, vectorSize);
            }
        }
        
        else {
            vas_iir_butterworth_process(x->filter, in, vectorSize);
        }
    }
    
    //call this whenever temperature, humidity or pressure have been chagned
    void vas_airAbsorbtionSimple_update_t_h_p(vas_airAbsorptionSimple *x) {
        vas_airAbsorptionSimple_calculate_a1_a2_a3(x);
        vas_airAbsorptionSimple_update_d(x);
    }
    
    //call this whenever distance has been changed but NOT temperature, humidity and pressure
    void vas_airAbsorptionSimple_update_d(vas_airAbsorptionSimple *x) {
        //get frequency where the sound is attenuated by 3dB with the current distance
        float f = vas_airAbsorptionSimple_get_cutoff_freq(x, x->w->d, 3.0f);
        vas_iir_butterworth_setFrequency(x->filter, f);
    }


#ifdef __cplusplus
}
#endif
