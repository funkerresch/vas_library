#include "vas_binauralspace~.h"
#include "vas_fir_read.h"

#ifdef __cplusplus
extern "C" {
#endif
extern vas_fir_list IRs;
#ifdef __cplusplus
}
#endif

static t_class *vas_binauralspace_class;

vas_binauralspace_reflection *vas_binauralspace_reflection_new(vas_fir *sharedFilter, vas_ringBuffer *ringBuffer)
{
    vas_binauralspace_reflection *x = ( vas_binauralspace_reflection * )vas_mem_alloc(sizeof(vas_binauralspace_reflection));
    x->tap = vas_delayTap_crossfade_new(ringBuffer);
    x->convolutionEngine = (vas_fir *)vas_fir_binaural_new(0);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)sharedFilter, x->convolutionEngine->left, x->convolutionEngine->right);
    vas_fir_setInitFlag(x->convolutionEngine);
    
    x->active = false;
    x->delayTime = 0;
    x->azimuth = 0;
    x->elevation = 0;
    x->damping = -1; // if damping < 0, use 1/r
    
    return x;
}

static void vas_binauralspace_aziDirection(vas_binauralspace *x, float aziDirection)
{
    if(aziDirection >= 1)
        x->aziDirection = 1;
    if(aziDirection < 1)
        x->aziDirection = 0;
}

static void vas_binauralspace_setAzimuth(vas_binauralspace *x, float azimuth)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    if(x->aziDirection)
        azimuth = 360 - azimuth;
    
    vas_fir_binaural_setAzimuth(binauralEngine, azimuth);
}

static void vas_binauralspace_setReflectionDelayTime(vas_binauralspace *x, float reflectionNumber, float delayTime)
{
    int refNumber = (int)floorf(reflectionNumber);
    vas_delayTap_crossfade_setDelayTime(x->tap[refNumber], delayTime);
}

static void vas_binauralspace_setReflectionAzimuth(vas_binauralspace *x, float reflectionNumber, float azimuth)
{
    int refNumber = (int)floorf(reflectionNumber);
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->binauralReflectionEngine[refNumber];
    if(x->aziDirection)
        azimuth = 360 - azimuth;
    
    vas_fir_binaural_setAzimuth(binauralEngine, azimuth);
}

static void vas_binauralspace_setReflectionElevation(vas_binauralspace *x, float reflectionNumber, float elevation)
{
    int refNumber = (int)floorf(reflectionNumber);
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->binauralReflectionEngine[refNumber];
    vas_fir_binaural_setElevation(binauralEngine, elevation);
}

static void vas_binauralspace_setElevation(vas_binauralspace *x, float elevation)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_fir_binaural_setElevation(binauralEngine, elevation);
}

static t_int *vas_binauralspace_perform(t_int *w)
{
    vas_binauralspace *x = (vas_binauralspace *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    t_float *outR = (t_float *)(w[4]);
    t_float *inputBufferPtr = x->inputBuffer;
    
    int n = (int)(w[5]);
    
    vas_ringBuffer_process(x->ringbuffer, in, n);
    vas_util_fcopy(inputBufferPtr, in, n);
    vas_fir_binaural_process(x->convolutionEngine, x->inputBuffer, outL, outR, n);
    
    for(int i=0; i<VAS_REFLECTIONS_MAXSIZE; i++)
    {
        vas_delayTap_crossfade_process(x->tap[i], x->inputBuffer, n);
        vas_fir_binaural_processOutputInPlace((vas_fir_binaural *)x->binauralReflectionEngine[i], x->inputBuffer, outL, outR, n);
    }
    
    vas_util_fcopy(outL, outR, n);
    
    return (w+6);
}

static void vas_binauralspace_dsp(vas_binauralspace *x, t_signal **sp)
{
    dsp_add(vas_binauralspace_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void vas_binauralspace_free(vas_binauralspace *x)
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

void vas_binauralspace_loadTestIr(vas_binauralspace *x)
{
    vas_fir_test_4096_1024_azimuthStride3(x->convolutionEngine);
}

static void *vas_binauralspace_new(t_symbol *s, int argc, t_atom *argv)
{
    int offset = 0;
    int end = 0;
    vas_binauralspace *x = (vas_binauralspace *)pd_new(vas_binauralspace_class);
    
    x->ringbuffer = vas_ringBuffer_new(VAS_RINGBUFFER_MAXSIZE);
    
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
    for(int i=0;i < VAS_REFLECTIONS_MAXSIZE; i++)
    {
        x->binauralReflectionEngine[i] = (vas_fir*) vas_fir_binaural_new(0);
        x->tap[i] = vas_delayTap_crossfade_new(x->ringbuffer);
    }

    if(!x->convolutionEngine)
    {
        post("Could not create vas_binauralspace.");
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
    
    if(vas_fir_getInitFlag(x->convolutionEngine))
    {
        post("Init shared filter for reflections");
        for(int i=0;i < VAS_REFLECTIONS_MAXSIZE; i++)
        {
            x->binauralReflectionEngine[i] = (vas_fir *)vas_fir_binaural_new(0);
            vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->convolutionEngine, x->binauralReflectionEngine[i]->left, x->binauralReflectionEngine[i]->right);
            vas_fir_setInitFlag(x->binauralReflectionEngine[i]);
            
            vas_binauralspace_setReflectionAzimuth(x, i, (i+1)*40);
            vas_binauralspace_setReflectionDelayTime(x, i, (i+1)*5000);
        }
    }

    return (x);
}

void vas_binauralspace_tilde_setup(void)
{
    vas_binauralspace_class = class_new(gensym("vas_binauralspace~"), (t_newmethod)vas_binauralspace_new, (t_method)vas_binauralspace_free,
    	sizeof(vas_binauralspace), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_binauralspace~ v0.73");
   
    CLASS_MAINSIGNALIN(vas_binauralspace_class, vas_binauralspace, f);
   
    class_addmethod(vas_binauralspace_class, (t_method)vas_binauralspace_dsp, gensym("dsp"), 0);
    class_addmethod(vas_binauralspace_class, (t_method)vas_binauralspace_setAzimuth, gensym("azimuth"), A_DEFFLOAT,0);
    class_addmethod(vas_binauralspace_class, (t_method)vas_binauralspace_setElevation, gensym("elevation"), A_DEFFLOAT,0);
    class_addmethod(vas_binauralspace_class, (t_method)vas_binauralspace_setReflectionDelayTime, gensym("delaytime"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(vas_binauralspace_class, (t_method)vas_binauralspace_setReflectionAzimuth, gensym("reflectionazimuth"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(vas_binauralspace_class, (t_method)vas_binauralspace_setReflectionElevation, gensym("reflectionelevation"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(vas_binauralspace_class, (t_method)vas_binauralspace_aziDirection, gensym("azidirection"), A_DEFFLOAT,0);
    class_addmethod(vas_binauralspace_class, (t_method)vas_pdmaxobject_read, gensym("read"), A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT,0);
}



