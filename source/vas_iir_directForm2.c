//
//  vas_iir_directForm2.c
//  AudioPlugin_VAS_Binaural
//
//  Created by Thomas Resch on 25.02.21.
//

#include "vas_iir_directForm2.h"

vas_iir_directForm2 *vas_iir_directForm2_new(int order) {
    vas_iir_directForm2 *x = vas_mem_alloc(sizeof(vas_iir_directForm2));
    
    x->order = order;
    x->a1 = 0;
    x->a2 = 0;
    x->a3 = 0;
    x->b0 = 1;
    x->b1 = 0;
    x->b2 = 0;
    x->b3 = 0;
    x->vz1 = 0;
    x->vz2 = 0;
    x->vz3 = 0;
    
#ifdef PFFFT_ENABLE_NEON__
    x->coeffs1[0][0] = 0; x->coeffs1[0][1] = 0; x->coeffs1[0][2] = 0; x->coeffs1[0][3] = x->b0; x->coeffs1[0][4] = x->b1; x->coeffs1[0][5] = x->b2; x->coeffs1[0][6] = x->a1; x->coeffs1[0][7] = x->a2;
    x->coeffs1[1][0] = 0; x->coeffs1[1][1] = 0; x->coeffs1[1][2] = x->b0; x->coeffs1[1][3] = x->b1; x->coeffs1[1][4] = x->b2; x->coeffs1[1][5] = 0; x->coeffs1[1][6] = x->a2; x->coeffs1[1][7] = 0;
    x->coeffs1[2][0] = 0; x->coeffs1[2][1] = x->b0; x->coeffs1[2][2] = x->b1; x->coeffs1[2][3] = x->b2; x->coeffs1[2][4] = 0; x->coeffs1[2][5] = 0; x->coeffs1[2][6] = 0; x->coeffs1[2][7] = 0;
    x->coeffs1[3][0] = x->b0; x->coeffs1[3][1] = x->b1; x->coeffs1[3][2] = x->b2; x->coeffs1[3][3] = 0; x->coeffs1[3][4] = 0; x->coeffs1[3][5] = 0; x->coeffs1[3][6] = 0; x->coeffs1[3][7] = 0;
    
    for (int ii = 0; ii < 8; ii++)
    {
        // add a1*row[0] to row[1]
        x->coeffs1[1][ii] += x->a1 * x->coeffs1[0][ii];
        // add a1*row[1] + a2*row[0] to row 2
        x->coeffs1[2][ii] += x->a1 * x->coeffs1[1][ii] + x->a2 * x->coeffs1[0][ii];
        // add a1*row[2] + a2*row[1] to row 3
        x->coeffs1[3][ii] += x->a1 * x->coeffs1[2][ii] + x->a2 * x->coeffs1[1][ii];
    }
                                                                                                        
#endif
                                                                                                                                         

#if defined (PFFFT_ENABLE_NEON_____)
    
    x->coeffs[0][0] = 0; x->coeffs[1][0] = 0; x->coeffs[2][0] = 0; x->coeffs[3][0] = x->b0; x->coeffs[4][0] = x->b1; x->coeffs[5][0] = x->b2; x->coeffs[6][0] = x->a1; x->coeffs[7][0] = x->a2;
    x->coeffs[0][1] = 0; x->coeffs[1][1] = 0; x->coeffs[2][1] = x->b0; x->coeffs[3][1] = x->b1; x->coeffs[4][1] = x->b2; x->coeffs[5][1] = 0; x->coeffs[6][1] = x->a2; x->coeffs[7][1] = 0;
    x->coeffs[0][2] = 0; x->coeffs[1][2] = x->b0; x->coeffs[2][2] = x->b1; x->coeffs[3][2] = x->b2; x->coeffs[4][2] = 0; x->coeffs[5][2] = 0; x->coeffs[6][2] = 0; x->coeffs[7][2] = 0;
    x->coeffs[0][3] = x->b0; x->coeffs[1][3] = x->b1; x->coeffs[2][3] = x->b2; x->coeffs[3][3] = 0; x->coeffs[4][3] = 0; x->coeffs[5][3] = 0; x->coeffs[6][3] = 0; x->coeffs[7][3] = 0;


    for (int ii = 0; ii < 8; ii++)
    {
        // add a1*row[0] to row[1]
        x->coeffs[ii][1] += x->a1 * x->coeffs[ii][0];
        // add a1*row[1] + a2*row[0] to row 2
        x->coeffs[ii][2] += x->a1 * x->coeffs[ii][1] + x->a2 * x->coeffs[ii][0];
        // add a1*row[2] + a2*row[1] to row 3
        x->coeffs[ii][3] += x->a1 * x->coeffs[ii][2] + x->a2 * x->coeffs[ii][1];
    }
    

#endif
    
    x->x1 = 0;
    x->x2 = 0;
    x->y1 = 0;
    x->y2 = 0;
    
    return x;
}

void vas_iir_directForm2_1stOrder_setCoefficients(vas_iir_directForm2 *x, float b0, float a1, float b1 )
{
    x->a1 = a1;
    
    x->b0 = b0;
    x->b1 = b1;
}

void vas_iir_directForm2_2ndOrder_setCoefficients(vas_iir_directForm2 *x, float b0, float a1, float b1, float a2, float b2 )
{
    x->a1 = a1;
    x->a2 = a2;
    
    x->b0 = b0;
    x->b1 = b1;
    x->b2 = b2;
    
#ifdef PFFFT_ENABLE_NEON__
    x->coeffs1[0][0] = 0; x->coeffs1[0][1] = 0; x->coeffs1[0][2] = 0; x->coeffs1[0][3] = x->b0; x->coeffs1[0][4] = x->b1; x->coeffs1[0][5] = x->b2; x->coeffs1[0][6] = x->a1; x->coeffs1[0][7] = x->a2;
    x->coeffs1[1][0] = 0; x->coeffs1[1][1] = 0; x->coeffs1[1][2] = x->b0; x->coeffs1[1][3] = x->b1; x->coeffs1[1][4] = x->b2; x->coeffs1[1][5] = 0; x->coeffs1[1][6] = x->a2; x->coeffs1[1][7] = 0;
    x->coeffs1[2][0] = 0; x->coeffs1[2][1] = x->b0; x->coeffs1[2][2] = x->b1; x->coeffs1[2][3] = x->b2; x->coeffs1[2][4] = 0; x->coeffs1[2][5] = 0; x->coeffs1[2][6] = 0; x->coeffs1[2][7] = 0;
    x->coeffs1[3][0] = x->b0; x->coeffs1[3][1] = x->b1; x->coeffs1[3][2] = x->b2; x->coeffs1[3][3] = 0; x->coeffs1[3][4] = 0; x->coeffs1[3][5] = 0; x->coeffs1[3][6] = 0; x->coeffs1[3][7] = 0;
    
    for (int ii = 0; ii < 8; ii++)
    {
        // add a1*row[0] to row[1]
        x->coeffs1[1][ii] += x->a1 * x->coeffs1[0][ii];
        // add a1*row[1] + a2*row[0] to row 2
        x->coeffs1[2][ii] += x->a1 * x->coeffs1[1][ii] + x->a2 * x->coeffs1[0][ii];
        // add a1*row[2] + a2*row[1] to row 3
        x->coeffs1[3][ii] += x->a1 * x->coeffs1[2][ii] + x->a2 * x->coeffs1[1][ii];
        
        printf("%f", x->coeffs1[1][ii]);
    }
                                                                                                        
#endif
    
#if defined (PFFFT_ENABLE_NEON_____)
    x->coeffs[0][0] = 0; x->coeffs[1][0] = 0; x->coeffs[2][0] = 0; x->coeffs[3][0] = x->b0; x->coeffs[4][0] = x->b1; x->coeffs[5][0] = x->b2; x->coeffs[6][0] = x->a1; x->coeffs[7][0] = x->a2;
    x->coeffs[0][1] = 0; x->coeffs[1][1] = 0; x->coeffs[2][1] = x->b0; x->coeffs[3][1] = x->b1; x->coeffs[4][1] = x->b2; x->coeffs[5][1] = 0; x->coeffs[6][1] = x->a2; x->coeffs[7][1] = 0;
    x->coeffs[0][2] = 0; x->coeffs[1][2] = x->b0; x->coeffs[2][2] = x->b1; x->coeffs[3][2] = x->b2; x->coeffs[4][2] = 0; x->coeffs[5][2] = 0; x->coeffs[6][2] = 0; x->coeffs[7][2] = 0;
    x->coeffs[0][3] = x->b0; x->coeffs[1][3] = x->b1; x->coeffs[2][3] = x->b2; x->coeffs[3][3] = 0; x->coeffs[4][3] = 0; x->coeffs[5][3] = 0; x->coeffs[6][3] = 0; x->coeffs[7][3] = 0;
    
    for (int ii = 0; ii < 8; ii++)
    {
        // add a1*row[0] to row[1]
        x->coeffs[ii][1] += x->a1 * x->coeffs[ii][0];
        // add a1*row[1] + a2*row[0] to row 2
        x->coeffs[ii][2] += x->a1 * x->coeffs[ii][1] + x->a2 * x->coeffs[ii][0];
        // add a1*row[2] + a2*row[1] to row 3
        x->coeffs[ii][3] += x->a1 * x->coeffs[ii][2] + x->a2 * x->coeffs[ii][1];
    }
#endif
    
}

void vas_iir_directForm2_3rdOrder_setCoefficients(vas_iir_directForm2 *x, float b0, float a1, float b1, float a2, float b2, float a3, float b3 )
{
    x->a1 = a1;
    x->a2 = a2;
    x->a3 = a3;
    
    x->b0 = b0;
    x->b1 = b1;
    x->b2 = b2;
    x->b3 = b3;
}

void vas_iir_directForm2_1stOrder_process(vas_iir_directForm2 *x, VAS_INPUTBUFFER *data, int n) {
    float *in = data;
    float v;
    float y;
    
    while(n--) {
        v = *in - x->a1 * x->vz1;
        y = x->b0 * v + x->b1 * x->vz1;
        x->vz1 = v;
        *in = y;
        in++;
    }
}

/*
 
 vector float    coeff_xp3 = { coeffs[0][0], coeffs[1][0], coeffs[2][0], coeffs[3][0] };
 vector float    coeff_xp2 = { coeffs[0][1], coeffs[1][1], coeffs[2][1], coeffs[3][1] };
 vector float    coeff_xp1 = { coeffs[0][2], coeffs[1][2], coeffs[2][2], coeffs[3][2] };
 vector float    coeff_x0  = { coeffs[0][3], coeffs[1][3], coeffs[2][3], coeffs[3][3] };
 vector float    coeff_xm1 = { coeffs[0][4], coeffs[1][4], coeffs[2][4], coeffs[3][4] };
 vector float    coeff_xm2 = { coeffs[0][5], coeffs[1][5], coeffs[2][5], coeffs[3][5] };
 vector float    coeff_ym1 = { coeffs[0][6], coeffs[1][6], coeffs[2][6], coeffs[3][6] };
 vector float    coeff_ym2 = { coeffs[0][7], coeffs[1][7], coeffs[2][7], coeffs[3][7] };
 
 */

void vas_iir_directForm2_2ndOrder_process_neon(vas_iir_directForm2 *x, VAS_INPUTBUFFER *in, int n) {
#if defined (PFFFT_ENABLE_NEON___)
    float32x4_t coeff_xp3 = vld1q_f32(x->coeffs);
    float32x4_t coeff_xp2 = vld1q_f32(x->coeffs+4);
    float32x4_t coeff_xp1 = vld1q_f32(x->coeffs+8);
    float32x4_t coeff_x0 = vld1q_f32(x->coeffs+12);
    float32x4_t coeff_xm1 = vld1q_f32(x->coeffs+16);
    float32x4_t coeff_xm2 = vld1q_f32(x->coeffs+20);
    float32x4_t coeff_ym1 = vld1q_f32(x->coeffs+24);
    float32x4_t coeff_ym2 = vld1q_f32(x->coeffs+28);
    
    float32x4_t value_xm2 = vld1q_dup_f32(&x->x2);
    float32x4_t value_xm1 = vld1q_dup_f32(&x->x1);
    float32x4_t value_ym2 = vld1q_dup_f32(&x->y2);
    float32x4_t value_ym1 = vld1q_dup_f32(&x->y1);
#endif
    
#ifdef PFFFT_ENABLE_NEON
    float32x4_t    coeff_xp3 = { x->coeffs1[0][0], x->coeffs1[1][0], x->coeffs1[2][0], x->coeffs1[3][0] };
    float32x4_t    coeff_xp2 = { x->coeffs1[0][1], x->coeffs1[1][1], x->coeffs1[2][1], x->coeffs1[3][1] };
    float32x4_t    coeff_xp1 = { x->coeffs1[0][2], x->coeffs1[1][2], x->coeffs1[2][2], x->coeffs1[3][2] };
    float32x4_t    coeff_x0  = { x->coeffs1[0][3], x->coeffs1[1][3], x->coeffs1[2][3], x->coeffs1[3][3] };
    float32x4_t    coeff_xm1 = { x->coeffs1[0][4], x->coeffs1[1][4], x->coeffs1[2][4], x->coeffs1[3][4] };
    float32x4_t    coeff_xm2 = { x->coeffs1[0][5], x->coeffs1[1][5], x->coeffs1[2][5], x->coeffs1[3][5] };
    float32x4_t    coeff_ym1 = { x->coeffs1[0][6], x->coeffs1[1][6], x->coeffs1[2][6], x->coeffs1[3][6] };
    float32x4_t    coeff_ym2 = { x->coeffs1[0][7], x->coeffs1[1][7], x->coeffs1[2][7], x->coeffs1[3][7] };
    
    float32x4_t value_xm2 = vld1q_dup_f32(&x->x2);
    float32x4_t value_xm1 = vld1q_dup_f32(&x->x1);
    float32x4_t value_ym2 = vld1q_dup_f32(&x->y2);
    float32x4_t value_ym1 = vld1q_dup_f32(&x->y1);
    
    while(n) {
        
        // new input samples
//        float32x4_t    x0123 = *in;    // load four floats
        float32x4_t value_x0 = vld1q_dup_f32(in);
        float32x4_t value_xp1 = vld1q_dup_f32(in+1);
        float32x4_t value_xp2 = vld1q_dup_f32(in+2);
        float32x4_t value_xp3 = vld1q_dup_f32(in+3);
        
//        float32x4_t y0123 = {in[0], in[1], in[2], in[3]};
// float32x4_t acc = vmlaq_f32(v3, v1, v2);  // acc = v3 + v1 * v2
//        // calculate output samples
        float32x4_t y0123 ={0,0,0,0};

        y0123 = vmlaq_f32(y0123, coeff_xp3 , value_xp3);
        y0123 = vmlaq_f32(y0123, coeff_xp2 , value_xp2);
        y0123 = vmlaq_f32(y0123, coeff_xp1 , value_xp1);
        y0123 = vmlaq_f32(y0123, coeff_x0 , value_x0);
        y0123 = vmlaq_f32(y0123, coeff_xm1 , value_xm1);
        y0123 = vmlaq_f32(y0123, coeff_xm2 , value_xm2);
        y0123 = vmlaq_f32(y0123, coeff_ym1 , value_ym1);
        y0123 = vmlaq_f32(y0123, coeff_ym2 , value_ym2);
//
//
//        // write output samples
        vst1q_f32(in, y0123);
//        *out++ = y0123;
//
//        // update recurrence
        value_xm2 = value_xp2;
        value_xm1 = value_xp3;
        
        
        value_ym2 = vld1q_dup_f32(in+2);
        value_ym1 = vld1q_dup_f32(in+3);
        in+=4;
        //out+=4;
        n-=4;
        
    }
    
    x->x2 = vgetq_lane_f32(value_xm2, 0);
    x->x1 = vgetq_lane_f32(value_xm1, 0);
    x->y2 = vgetq_lane_f32(value_ym2, 0);
    x->y1 = vgetq_lane_f32(value_ym1, 0);

#endif
}

void vas_iir_directForm2_2ndOrder_process(vas_iir_directForm2 *x, VAS_INPUTBUFFER *data, int n) {
    float *in = data;
    
    float v;
    float y;
    
    while(n--) {
        v = *in - x->a1 * x->vz1 - x->a2 * x->vz2;
        y = x->b0 * v + x->b1 * x->vz1 + x->b2 *x->vz2;
        x->vz2 = x->vz1;
        x->vz1 = v;
        *in = y;
        in++;
    }


}

void vas_iir_directForm2_3rdOrder_process(vas_iir_directForm2 *x, VAS_INPUTBUFFER *data, int n) {
    float *in = data;
    float v;
    float y;
    
    while(n--) {
        v = *in - x->a1 * x->vz1 - x->a2 * x->vz2 - x->a3 *x->vz3;
        y = x->b0 * v + x->b1 * x->vz1 + x->b2 *x->vz2 + x->b3 * x->vz3;
        x->vz3 = x->vz2;
        x->vz2 = x->vz1;
        x->vz1 = v;
        *in = y;
        in++;
    }
}

void vas_iir_directForm2_process(vas_iir_directForm2 *x, VAS_INPUTBUFFER *data, int n) {
    if(x->order == 3)
        vas_iir_directForm2_3rdOrder_process(x, data, n);
    if(x->order == 2)
        vas_iir_directForm2_2ndOrder_process(x, data, n);
    if(x->order == 1)
        vas_iir_directForm2_1stOrder_process(x, data, n);
}

void vas_iir_directForm2_free(vas_iir_directForm2 *x)
{
    vas_mem_free(x);
}
