#include "vas_delay.h"

vas_delay *vas_delay_new(long bufferSize)
{
    vas_delay *x = (vas_delay *)malloc(sizeof(vas_delay));
    x->bufferSize = bufferSize;
    x->buffer = (float *) calloc (x-> bufferSize + 1 , sizeof(float));
    x->circular_pointer = x->buffer;
    x->delayTimeInSamples = 0;

    return x;
}

void vas_delay_free(vas_delay *x)
{
    free(x->buffer);
    free(x);
}

void vas_delay_perform(vas_delay *x, float *in, float *out, int vectorSize)
{
    int i = 0;
    
    while(i < vectorSize)
    {
        *(x->circular_pointer) = in[i];
        out[i] = vas_delay_tap_into_buffer(x->bufferSize, x->buffer, x->circular_pointer, x->delayTimeInSamples);
        vas_delay_decrement_circular_pointer(x->bufferSize, x->buffer, &(x->circular_pointer));
        i++;
    }
}

void vas_delay_setDelayTime(vas_delay *x, float _delay_in_samples)
{
    if (_delay_in_samples < 0 || _delay_in_samples >= ((x-> bufferSize) - 1))
        _delay_in_samples = 0;
    x->delayTimeInSamples = floor(_delay_in_samples) ;
}

float vas_delay_tap_into_buffer(long buffer_size, float *buffer_pointer, float *circular_pointer, int index)
{
    return buffer_pointer[(circular_pointer - buffer_pointer + index) % ((buffer_size) + 1)];
}

void vas_delay_wrap_circular_pointer_to_start(long buffer_size, float *buffer_pointer, float **circular_pointer)
{
    // when *circular_pointer=buffer_pointer+buffer_size+1, it wraps around to *circular_pointer=buffer_pointer
    if (*circular_pointer > buffer_pointer + buffer_size)
        *circular_pointer -= buffer_size + 1;
    // when *circular_pointer=buffer_pointer-1, it wraps around to *circular_pointer=buffer_pointer+buffer_size
    if (*circular_pointer < buffer_pointer)
        *circular_pointer += buffer_size + 1;
}

void vas_delay_decrement_circular_pointer(long buffer_size, float *buffer_pointer, float **circular_pointer)
{
    // decrement pointer and wrap modulo-buffer_size+1
    (*circular_pointer)--;
    // when *circular_pointer=buffer_pointer-1, it wraps around to *circular_pointer=buffer_pointer+buffer_size
    vas_delay_wrap_circular_pointer_to_start(buffer_size, buffer_pointer, circular_pointer);
}

