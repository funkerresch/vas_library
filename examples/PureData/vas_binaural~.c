#include "vas_binaural~.h"
#include "vas_fir_read.h"

#ifdef __cplusplus
extern "C" {
#endif
extern vas_fir_list IRs;
#ifdef __cplusplus
}
#endif

static t_class *vas_binaural_class;

#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
static void vas_binaural_leaveNumberOfPartionsActive(vas_binaural *x, float i)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    if(x->fullpath[0] != '\0')
    {
        vas_dynamicFirChannel_leaveActivePartitions(binauralEngine->left, i);
        vas_dynamicFirChannel_leaveActivePartitions(binauralEngine->right, i);
    }
}

static void vas_binaural_setSegmentThreshold(vas_binaural *x, float thresh)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setSegmentThreshold(binauralEngine->left, thresh);
    vas_dynamicFirChannel_setSegmentThreshold(binauralEngine->right, thresh);
}

static void vas_binaural_postInactivePartionIndexes(vas_binaural *x)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    for(int i=0; i < binauralEngine->right->filter->numberOfSegments; i++ )
    {
        if( binauralEngine->right->filter->segmentIsZero[0][0][i])
            post("%d", i);
    }
}
#endif

static void vas_binaural_aziDirection(vas_binaural *x, float aziDirection)
{
    if(aziDirection >= 1)
        x->aziDirection = 1;
    if(aziDirection < 1)
        x->aziDirection = 0;
}

static void vas_binaural_setAzimuth(vas_binaural *x, float azimuth)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    if(x->aziDirection)
        azimuth = 360 - azimuth;
    
    vas_dynamicFirChannel_setAzimuth(binauralEngine->left, azimuth);
    vas_dynamicFirChannel_setAzimuth(binauralEngine->right, azimuth);
}

static void vas_binaural_setElevation(vas_binaural *x, float elevation)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setElevation(binauralEngine->left, elevation);
    vas_dynamicFirChannel_setElevation(binauralEngine->right, elevation);
}

static t_int *vas_binaural_perform(t_int *w)
{
    vas_binaural *x = (vas_binaural *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    t_float *outR = (t_float *)(w[4]);
    t_float *inputBufferPtr = x->inputBuffer;
    
    int n = (int)(w[5]);
    
    while(n--)
        *inputBufferPtr++ = *in++;
 
    n = (int)(w[5]);
    
    vas_fir_binaural_process(x->convolutionEngine, x->inputBuffer, outL, outR, n);
    
    return (w+6);
}

static void vas_binaural_dsp(vas_binaural *x, t_signal **sp)
{
    dsp_add(vas_binaural_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void vas_binaural_free(vas_binaural *x)
{
    vas_fir *convolutionEngine = (vas_fir *)x->convolutionEngine;
    if(convolutionEngine->left->filter->referenceCounter == 1 || convolutionEngine->left->useSharedFilter == false  )
        vas_fir_list_removeNode1(&IRs, (vas_fir *)x->convolutionEngine);
    
    vas_fir_binaural_free(x->convolutionEngine);
    inlet_free(x->azi);
    inlet_free(x->ele);
    outlet_free(x->outL);
    outlet_free(x->outR);
}

void vas_binaural_loadTestIr(vas_binaural *x)
{
    vas_fir_test_4096_1024_azimuthStride3(x->convolutionEngine);
}

static void *vas_binaural_new(t_symbol *s, int argc, t_atom *argv)
{
    int offset = 0;
    int end = 0;
    vas_binaural *x = (vas_binaural *)pd_new(vas_binaural_class);
    
    t_symbol *path = NULL;
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->outR = outlet_new(&x->x_obj, gensym("signal"));
    x->azi = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("azimuth"));
    x->ele = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("elevation"));
    
    x->segmentSize = 256;
    x->filterSize = 0;
    x->f = 0;
    x->fullpath[0] = '\0';
    x->aziDirection = 1;
    x->eleDirection = 1;
    sprintf(x->canvasDirectory, "%s", canvas_getcurrentdir()->s_name);

    if(argc >= 1)
    {
        if(argv[0].a_type == A_FLOAT)
        {
            x->segmentSize = (float)atom_getintarg(0, argc, argv);
        }
        
        if(argv[0].a_type == A_SYMBOL)
            path = atom_getsymbolarg(0, argc, argv);
    }
    
    x->convolutionEngine = vas_fir_binaural_new(0);

    if(!x->convolutionEngine)
    {
        post("Could not create vas_binaural.");
        free(x);
        return NULL;
    }
    
    if(argc >= 2)
    {
        if(argv[1].a_type == A_SYMBOL)
            path = atom_getsymbolarg(1, argc, argv);
    }
    
    if(argc >= 3)
    {
        if(argv[2].a_type == A_FLOAT)
            offset = atom_getfloatarg(2, argc, argv);
    }
    
    if(argc >= 4)
    {
        if(argv[3].a_type == A_FLOAT)
            end = atom_getfloatarg(3, argc, argv);
    }
    
    if(path)
        vas_pdmaxobject_read((vas_pdmaxobject *)x, path, x->segmentSize, offset, end);

    return (x);
}

void vas_binaural_tilde_setup(void)
{
    vas_binaural_class = class_new(gensym("vas_binaural~"), (t_newmethod)vas_binaural_new, (t_method)vas_binaural_free,
    	sizeof(vas_binaural), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_binaural~ v0.75");
   
    CLASS_MAINSIGNALIN(vas_binaural_class, vas_binaural, f);
   
    class_addmethod(vas_binaural_class, (t_method)vas_binaural_dsp, gensym("dsp"), 0);
    class_addmethod(vas_binaural_class, (t_method)vas_binaural_setAzimuth, gensym("azimuth"), A_DEFFLOAT,0);
    class_addmethod(vas_binaural_class, (t_method)vas_binaural_setElevation, gensym("elevation"), A_DEFFLOAT,0);
    class_addmethod(vas_binaural_class, (t_method)vas_binaural_aziDirection, gensym("azidirection"), A_DEFFLOAT,0);
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
    class_addmethod(vas_binaural_class, (t_method)vas_binaural_setSegmentThreshold, gensym("thresh"), A_DEFFLOAT,0);
    class_addmethod(vas_binaural_class, (t_method)vas_binaural_postInactivePartionIndexes, gensym("zeroindexes"),0);
    class_addmethod(vas_binaural_class, (t_method)vas_binaural_leaveNumberOfPartionsActive,  gensym("activepartitions"),A_DEFFLOAT, 0);
#endif
    class_addmethod(vas_binaural_class, (t_method)vas_pdmaxobject_read, gensym("read"), A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT,0);

   // class_addmethod(vas_binaural_class, (t_method)vas_binaural_loadTestIr,  gensym("testIr"), 0);
}



