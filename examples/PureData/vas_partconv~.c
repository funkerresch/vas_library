#include "vas_partconv~.h"
#include "vas_pdmaxobject.h"

static t_class *vas_partconv_class;

static t_int *vas_partconv_perform(t_int *w)
{
    vas_partconv *x = (vas_partconv *)(w[1]) ;
    t_float *in = (t_float *)(w[2]);
    t_float *outL = (t_float *)(w[3]);
    t_float *outR = (t_float *)(w[4]);
    t_float *inputBufferPtr = x->inputBuffer;
    
    int n = (int)(w[5]);
    
    while(n--)
        *inputBufferPtr++ = *in++;
    
    n = (int)(w[5]);
    
    vas_fir_partitioned_process(x->partConvEngine, x->inputBuffer, outL, outR, n);
    
    return (w+6);
}

static void vas_partconv_dsp(vas_partconv *x, t_signal **sp)
{
    dsp_add(vas_partconv_perform, 5, x,  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void vas_partconv_free(vas_partconv *x)
{
   // vas_fir_partconv_free(x->convolutionEngine);
    
    outlet_free(x->outL);
    outlet_free(x->outR);
    
    if(x->leftArray)
        vas_mem_free(x->leftArray);
    if(x->rightArray)
        vas_mem_free(x->rightArray);
}

static void *vas_partconv_new(t_symbol *s, int argc, t_atom *argv)
{
    int offset = 0;
    int end = 0;
    vas_partconv *x = (vas_partconv *)pd_new(vas_partconv_class);
    
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
        {
            x->segmentSize = (float)atom_getintarg(0, argc, argv);
        }
        
        if(argv[0].a_type == A_SYMBOL)
            path = atom_getsymbolarg(0, argc, argv);
    }
    
    x->partConvEngine = vas_fir_partitioned_new(0);

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
    
    if(path)
    {
        vas_pdmaxobject_read((vas_pdmaxobject *)x, path, x->segmentSize, 0, 2048);
        x->convolutionEngine = x->partConvEngine->convolutionEngine[1];
        vas_pdmaxobject_read((vas_pdmaxobject *)x, path, x->segmentSize, 2048, -1);
        x->convolutionEngine = x->partConvEngine->convolutionEngine[0];
    }

    return (x);
}

void vas_partconv_read(vas_partconv *x,  t_symbol *s, float segmentSize)
{
    vas_pdmaxobject_read((vas_pdmaxobject *)x, s, 512, 0, 4096);
    x->convolutionEngine = x->partConvEngine->convolutionEngine[1];
    vas_pdmaxobject_read((vas_pdmaxobject *)x, s, 2048, 4096, 20480);
    x->convolutionEngine = x->partConvEngine->convolutionEngine[0];
}

void vas_partconv_set(vas_partconv *x,  t_symbol *left, t_symbol *right, float minSegmentSize, float maxSegmentSize)
{
    int maxLength = 0;
    minSegmentSize = vas_utilities_roundUp2NextPowerOf2(minSegmentSize);
    if(maxSegmentSize > minSegmentSize)
        maxSegmentSize = vas_utilities_roundUp2NextPowerOf2(maxSegmentSize);
    else
        maxSegmentSize = 0;
    
    x->segmentSize = minSegmentSize;
    vas_pdmaxobject_getFloatArrayAndLength(left, &x->leftArray, &x->leftArrayLength);
    vas_pdmaxobject_getFloatArrayAndLength(right, &x->rightArray, &x->rightArrayLength);
    
    maxLength = x->leftArrayLength;
    if(x->rightArrayLength > maxLength)
        maxLength = x->rightArrayLength;
    
    maxLength = vas_fir_partitioned_init(x->partConvEngine, minSegmentSize, maxSegmentSize, maxLength);
        
    size_t size = maxLength * sizeof(float);
    float *leftIr = (float *)vas_mem_alloc( size);
    float *rightIr = (float *)vas_mem_alloc( size);
    memset(leftIr, 0, size);
    memset(rightIr, 0, size);
       
    for (int i=0; i<x->leftArrayLength; i++)
        leftIr[i] = x->leftArray[i].w_float;
    
    for (int i=0; i<x->rightArrayLength;i++)
        rightIr[i] = x->rightArray[i].w_float;
    
    vas_fir_partitioned_setFilter(x->partConvEngine, leftIr, rightIr, maxLength);
     
    vas_mem_free(leftIr);
    vas_mem_free(rightIr);
}

void vas_partconv_tilde_setup(void)
{
    vas_partconv_class = class_new(gensym("vas_partconv~"), (t_newmethod)vas_partconv_new, (t_method)vas_partconv_free,
    	sizeof(vas_partconv), CLASS_DEFAULT, A_GIMME, 0);
    
    post("vas_partconv~ v0.7");
   
    CLASS_MAINSIGNALIN(vas_partconv_class, vas_partconv, f);
   
    class_addmethod(vas_partconv_class, (t_method)vas_partconv_dsp, gensym("dsp"), 0);
    class_addmethod(vas_partconv_class, (t_method)vas_pdmaxobject_read, gensym("read"), A_DEFSYM, A_DEFFLOAT, 0);
    class_addmethod(vas_partconv_class, (t_method)vas_partconv_set, gensym("set"), A_DEFSYM, A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, 0);
}
