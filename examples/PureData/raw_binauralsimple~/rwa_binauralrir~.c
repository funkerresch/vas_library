#include "rwa_binauralrir~.h"

static t_class *rwa_binauralrir_class;

static void rwa_binauralrir_leaveNumberOfPartionsActive(rwa_binauralrir *x, float i)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    if(x->fullpath[0] != '\0')
    {
        vas_dynamicFirChannel_leaveActivePartitions(binauralEngine->left, i);
        vas_dynamicFirChannel_leaveActivePartitions(binauralEngine->right, i);
    }
}

static void rwa_binauralrir_setSegmentThreshold(rwa_binauralrir *x, float thresh)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setSegmentThreshold(binauralEngine->left, thresh);
    vas_dynamicFirChannel_setSegmentThreshold(binauralEngine->right, thresh);
}

static void rwa_binauralrir_setAzimuth(rwa_binauralrir *x, float azimuth)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setAzimuth(binauralEngine->left, azimuth);
    vas_dynamicFirChannel_setAzimuth(binauralEngine->right, azimuth);
}

static void rwa_binauralrir_setElevation(rwa_binauralrir *x, float elevation)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setElevation(binauralEngine->left, elevation);
    vas_dynamicFirChannel_setElevation(binauralEngine->right, elevation);
}

static t_int *rwa_binauralrir_perform(t_int *w)
{
    rwa_binauralrir *x = (rwa_binauralrir *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    t_float *outR = (t_float *)(w[4]);
    t_float *inputBufferPtr = x->inputBuffer;
    t_float *outputBufferPtrL = outL;
    t_float *outputBufferPtrR = outR;
    
    int n = (int)(w[5]);
    
    while(n--)
    {
        *inputBufferPtr++ = *in++;
        *outputBufferPtrL++ = 0; *outputBufferPtrR++ = 0;
    }
    
    n = (int)(w[5]);
    
    vas_fir_binaural_process(x->convolutionEngine, x->inputBuffer, outL, outR, n);
    
    return (w+6);
}

static void rwa_binauralrir_dsp(rwa_binauralrir *x, t_signal **sp)
{
    dsp_add(rwa_binauralrir_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void rwa_binauralrir_free(rwa_binauralrir *x)
{
    vas_fir_binaural_free(x->convolutionEngine);
}

static void *rwa_binauralrir_new(t_symbol *s, int argc, t_atom *argv)
{
    rwa_binauralrir *x = (rwa_binauralrir *)pd_new(rwa_binauralrir_class);
    
    t_symbol *path = NULL;

    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->outR = outlet_new(&x->x_obj, gensym("signal"));
    x->segmentSize = 256;
    x->filterSize = 0;
    x->useGlobalFilter = 1;
    x->f = 0;
    x->fullpath[0] = '\0';
    sprintf(x->canvasDirectory, "%s", canvas_getcurrentdir()->s_name);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("azimuth"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("elevation"));
    
    if(argc >= 1)
    {
        if(argv[0].a_type == A_FLOAT)
        {
            x->segmentSize = (float)atom_getintarg(0, argc, argv);
        }
        
        if(argv[0].a_type == A_SYMBOL)
            path = atom_getsymbolarg(0, argc, argv);
    }
    
    x->convolutionEngine = vas_fir_binaural_new(VAS_VDSP | VAS_BINAURALSETUP_NOELEVATION | VAS_LOCALFILTER, x->segmentSize, NULL);
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    if(!x->convolutionEngine)
    {
        post("Could not create rwa_binauralrir.");
        free(x);
        return NULL;
    }
    
    if(argc >= 2)
    {
        if(argv[1].a_type == A_SYMBOL)
            path = atom_getsymbolarg(1, argc, argv);
    }
    
    if(path)
    {
        if(!vas_fir_getInitFlag((vas_fir *)x->convolutionEngine))
        {
            rwa_firobject_read((rwa_firobject *)x, path);
            //post("BRIR OVERALL ENERGY: %.14f", binauralEngine->right->filter->overallEnergy[0][0] );
           // post("BRIR Average Segment ENERGY: %.14f", binauralEngine->right->filter->overallEnergy[0][0] / binauralEngine->right->filter->nonZeroCounter[0][0] );
        }
        else
        {
            vas_dynamicFirChannel_initArraysWithGlobalFilter(binauralEngine->left);
            vas_dynamicFirChannel_initArraysWithGlobalFilter(binauralEngine->right);
        }
    }
    
    return (x);
}

void rwa_binauralrir_tilde_setup(void)
{
    rwa_binauralrir_class = class_new(gensym("rwa_binauralrir~"), (t_newmethod)rwa_binauralrir_new, (t_method)rwa_binauralrir_free,
    	sizeof(rwa_binauralrir), CLASS_DEFAULT, A_GIMME, 0);
    
    post("rwa_binauralrir~ v0.56");
   
    CLASS_MAINSIGNALIN(rwa_binauralrir_class, rwa_binauralrir, f);
   
    class_addmethod(rwa_binauralrir_class, (t_method)rwa_binauralrir_dsp, gensym("dsp"), 0);
    class_addmethod(rwa_binauralrir_class, (t_method)rwa_binauralrir_setAzimuth, gensym("azimuth"), A_DEFFLOAT,0);
    class_addmethod(rwa_binauralrir_class, (t_method)rwa_binauralrir_setElevation, gensym("elevation"), A_DEFFLOAT,0);
    class_addmethod(rwa_binauralrir_class, (t_method)rwa_binauralrir_setSegmentThreshold, gensym("thresh"), A_DEFFLOAT,0);
    class_addmethod(rwa_binauralrir_class, (t_method)rwa_firobject_read, gensym("read"), A_DEFSYM,0);
    class_addmethod(rwa_binauralrir_class, (t_method)rwa_binauralrir_leaveNumberOfPartionsActive,  gensym("activepartitions"),A_DEFFLOAT, 0);
}
