#include "vas_delay.h"

vas_delay *vas_delay_new(long _buffer_size)
{
    vas_delay *x = (vas_delay *)malloc(sizeof(vas_delay));
    x-> buffer_size = _buffer_size;
    x-> buffer = (float *) calloc (x-> buffer_size + 1 , sizeof(float));
    x-> circular_pointer = x-> buffer;
    x-> circularPointerMinusVS = x-> buffer;
    x-> delay_sample = .0;
    x-> delay_in_samples = .0;
    x->targetDelayTime = 0;
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
        out[i] = vas_delay_tap_into_buffer(x->buffer_size, x->buffer, x->circular_pointer, x->delay_in_samples);
        vas_delay_decrement_circular_pointer(x->buffer_size, x->buffer, &(x->circular_pointer));
        
        i++;
    }
}

void vas_delay_perform_interpolated(vas_delay *x, float *in, float *out, int vector_size)
{
    int i = 0;
    
    
    while(i < vector_size)
    {
        *(x->circular_pointer) = in[i];
        //x-> delay_sample = vas_delay_interpolate_tap_into_buffer(x, x-> buffer_size, x->delay_in_samples, x-> buffer, x-> circular_pointer);
        x-> delay_sample = vas_delay_tap_into_buffer(x->buffer_size, x->buffer, x->circular_pointer, x->delay_in_samples);
        out[i] = (x-> delay_sample);
        vas_delay_decrement_circular_pointer(x->buffer_size, x->buffer, &(x->circular_pointer));
        i++;
    }
}

void vas_delay_setDelayTime(vas_delay *x, float _delay_in_samples)
{
    if (_delay_in_samples < 0 || _delay_in_samples >= ((x-> buffer_size) - 1))
        _delay_in_samples = 0;
    x->delay_in_samples = floor(_delay_in_samples) ;
}

float vas_delay_tap_into_buffer(long buffer_size, float *buffer_pointer, float *circular_pointer, int index)
{
    return buffer_pointer[(circular_pointer - buffer_pointer + index) % ((buffer_size) + 1)];
}

float vas_delay_interpolate_tap_into_buffer(vas_delay *x, long buffer_size, float delay_in_samples, float *buffer_pointer, float *circular_pointer)
{
    // positions in buffer to interpolate between
    int i, j;
    float si, sj;
    
    
    
    // interpolate between si and sj
    i = floor(x->delay_in_samples);
    // if i=D, then j=0 otherwise, j=i+1
    j = (i + 1) % ((buffer_size) + 1);


    // if i=D, then j=0 otherwise, j=i+1
    //j = (i + 1) % ((buffer_size) + 1);
    
    // get buffer values by taping into buffer without interpolation
    // si(n) = x(n-i)
    si = vas_delay_tap_into_buffer(buffer_size, buffer_pointer, circular_pointer, i);
    // sj(n) = x(n-j)
    sj = vas_delay_tap_into_buffer(buffer_size, buffer_pointer, circular_pointer, j);

    return si + ((delay_in_samples) - i) * (sj - si);
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

