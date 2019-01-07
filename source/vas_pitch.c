//
//  vas_pitch.c
//  tr.binaural~
//
//  Created by Admin on 17.09.18.
//

#include "vas_pitch.h"

static int vas_pitch_absoluteThreshold(vas_pitch *x)
{
    ssize_t size = x->yin_bufferSize;
    int tau;
    for (tau = 2; tau < size; tau++) {
        if (x->yin_buffer[tau] < YIN_DEFAULT_THRESHOLD) {
            while (tau + 1 < size && x->yin_buffer[tau + 1] < x->yin_buffer[tau]) {
                tau++;
            }
            break;
        }
    }
    return (tau == size || x->yin_buffer[tau] >= YIN_DEFAULT_THRESHOLD) ? -1 : tau;
}

static void vas_pitch_difference(vas_pitch *x, float *data, int vectorSize)
{
    int index, tau;
    double delta;
    x->yin_bufferSize =  vectorSize/ 2;

    for (tau = 0; tau < x->yin_bufferSize; tau++) {
        x->yin_buffer[tau] = 0;
    }
    for (tau = 1; tau < x->yin_bufferSize; tau++) {
        for (index = 0; index < x->yin_bufferSize; index++) {
            delta = data[index] - data[index + tau];
            x->yin_buffer[tau] += delta * delta;
        }
    }
}

static void vas_pitch_cumulativeMeanNormalizedDifference(vas_pitch *x)
{
    int tau;
    x->yin_buffer[0] = 1;
    double running_sum = 0;

    for (tau = 1; tau < x->yin_bufferSize; tau++) {
        running_sum += x->yin_buffer[tau];
        x->yin_buffer[tau] *= tau / running_sum;
    }
}

static float vas_pitch_parabolicInterpolation(vas_pitch *xx, float *array, int arraySize, int x)
{
    int x_adjusted;
    
    if (x < 1) {
        x_adjusted = (array[x] <= array[x + 1]) ? x : x + 1;
    } else if (x > arraySize - 1) {
        x_adjusted = (array[x] <= array[x - 1]) ? x : x - 1;
    } else {
        double den = array[x + 1] + array[x - 1] - 2 * array[x];
        double delta = array[x - 1] - array[x + 1];
        return (!den) ? x
        : (x + delta / (2 * den));
           
    }
    return x_adjusted;
    
}

static void vas_pitch_process(vas_pitch *x, AK_INPUTVECTOR *in, AK_OUTPUTVECTOR *out, int vectorSize, int flags)
{
    
    
}

/*
std::pair<double, double>
parabolic_interpolation(const std::vector<double> &array, double x)
{
    int x_adjusted;
    
    if (x < 1) {
        x_adjusted = (array[x] <= array[x + 1]) ? x : x + 1;
    } else if (x > signed(array.size()) - 1) {
        x_adjusted = (array[x] <= array[x - 1]) ? x : x - 1;
    } else {
        double den = array[x + 1] + array[x - 1] - 2 * array[x];
        double delta = array[x - 1] - array[x + 1];
        return (!den) ? std::make_pair(x, array[x])
        : std::make_pair(x + delta / (2 * den),
                         array[x] - delta * delta / (8 * den));
    }
    return std::make_pair(x_adjusted, array[x_adjusted]);
}
*/

/*

 



double
get_pitch_yin(const std::vector<double> &audio_buffer, int sample_rate)
{
    int tau_estimate;
    
    std::vector<double> yin_buffer = difference(audio_buffer);
    cumulative_mean_normalized_difference(yin_buffer);
    tau_estimate = absolute_threshold(yin_buffer);
    
    return (tau_estimate != -1)
    ? sample_rate / std::get<0>(parabolic_interpolation(
                                                        yin_buffer, tau_estimate))
    : -1;
}*/
