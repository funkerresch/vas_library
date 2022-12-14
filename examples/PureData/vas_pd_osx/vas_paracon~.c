#include "vas_paracon~.h"
#include "vas_pdmaxobject.h"

static t_class *vas_paracon_class;

static t_int *vas_paracon_perform(t_int *w)
{
    vas_paracon *x = (vas_paracon *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    t_float *outR = (t_float *)(w[4]);
    int n = (int)(w[5]);
    
    vas_util_fcopy(in, x->inputBuffer, n);
    vas_fir_parallel_process(x->partConvEngine, x->inputBuffer, outL, outR, n);
    
    return (w+6);
}

static void vas_paracon_dsp(vas_paracon *x, t_signal **sp)
{
    dsp_add(vas_paracon_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void vas_paracon_free(vas_paracon *x)
{
   // vas_fir_partconv_free(x->convolutionEngine);
    
    outlet_free(x->outL);
    outlet_free(x->outR);
    
    if(x->leftArray)
        vas_mem_free(x->leftArray);
    if(x->rightArray)
        vas_mem_free(x->rightArray);
}

static void *vas_paracon_new(t_symbol *s, int argc, t_atom *argv)
{
    int offset = 0;
    int end = 0;
    vas_paracon *x = (vas_paracon *)pd_new(vas_paracon_class);
    
    t_symbol *path = NULL;
    x->outL = outlet_new(&x->x_obj, gensym("signal"));
    x->outR = outlet_new(&x->x_obj, gensym("signal"));
    
    x->segmentSize = 256;
    x->filterSize = 0;
    x->f = 0;
    x->fullpath[0] = '\0';
    
    x->leftArray = NULL;
    x->rightArray = NULL;
    
    sprintf(x->canvasDirectory, "%s", canvas_getcurrentdir()->s_name);

    if(argc >= 1)
    {
        if(argv[0].a_type == A_FLOAT)
            x->segmentSize = (float)atom_getintarg(0, argc, argv);
        
        if(argv[0].a_type == A_SYMBOL)
            path = atom_getsymbolarg(0, argc, argv);
    }
    
    x->partConvEngine = vas_fir_parallel_new(12);

    if(!x->partConvEngine)
    {
        post("Could not create vas_part_conv~");
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
    
    if(path) // just test case so far
    {
        vas_pdmaxobject_read((vas_pdmaxobject *)x, path, x->segmentSize, 0, 2048);
        x->convolutionEngine = x->partConvEngine->convolutionEngine[1];
        vas_pdmaxobject_read((vas_pdmaxobject *)x, path, x->segmentSize, 2048, -1);
        x->convolutionEngine = x->partConvEngine->convolutionEngine[0];
    }

    return (x);
}

// not ready yet, vas_paracon_read just used as a test case, only vas_paracon_set is working

void vas_paracon_read(vas_paracon *x,  t_symbol *s, float segmentSize)
{
    vas_pdmaxobject_read((vas_pdmaxobject *)x, s, 512, 0, 4096); // reads 8 partitions from 0 until 4096
    x->convolutionEngine = x->partConvEngine->convolutionEngine[1];
    vas_pdmaxobject_read((vas_pdmaxobject *)x, s, 2048, 4096, -1); // reads the rest
    x->convolutionEngine = x->partConvEngine->convolutionEngine[0];
}

void vas_paracon_set(vas_paracon *x,  t_symbol *left, t_symbol *right, float minSegmentSize, float maxSegmentSize)
{
    int maxLength = 0;
    minSegmentSize = vas_util_roundUp2NextPowerOf2(minSegmentSize);
    if(maxSegmentSize > minSegmentSize)
        maxSegmentSize = vas_util_roundUp2NextPowerOf2(maxSegmentSize);
    else
        maxSegmentSize = 0;
    
    x->segmentSize = minSegmentSize;
    vas_pdmaxobject_getFloatArrayAndLength(left, &x->leftArray, &x->leftArrayLength);
    vas_pdmaxobject_getFloatArrayAndLength(right, &x->rightArray, &x->rightArrayLength);
    
    maxLength = x->leftArrayLength;
    if(x->rightArrayLength > maxLength)
        maxLength = x->rightArrayLength;
    
    maxLength = vas_fir_parallel_init(x->partConvEngine, minSegmentSize, maxSegmentSize, maxLength);
        
    size_t size = maxLength * sizeof(float);
    float *leftIr = (float *)vas_mem_alloc( size);
    float *rightIr = (float *)vas_mem_alloc( size);
    memset(leftIr, 0, size);
    memset(rightIr, 0, size);
       
    for (int i=0; i<x->leftArrayLength; i++)
        leftIr[i] = x->leftArray[i].w_float;
    
    for (int i=0; i<x->rightArrayLength;i++)
        rightIr[i] = x->rightArray[i].w_float;
    
    vas_fir_parallel_setFilter(x->partConvEngine, leftIr, rightIr, maxLength);
     
    vas_mem_free(leftIr);
    vas_mem_free(rightIr);
}

void vas_paracon_tilde_setup(void)
{
    vas_paracon_class = class_new(gensym("vas_paracon~"), (t_newmethod)vas_paracon_new, (t_method)vas_paracon_free,
    	sizeof(vas_paracon), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_paracon~ v0.5");
   
    CLASS_MAINSIGNALIN(vas_paracon_class, vas_paracon, f);
   
    class_addmethod(vas_paracon_class, (t_method)vas_paracon_dsp, gensym("dsp"), 0);
   // class_addmethod(vas_paracon_class, (t_method)vas_pdmaxobject_read, gensym("read"), A_DEFSYM, A_DEFFLOAT, 0);
    class_addmethod(vas_paracon_class, (t_method)vas_paracon_set, gensym("set"), A_DEFSYM, A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
}
