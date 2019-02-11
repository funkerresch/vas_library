#include "tr_convolve~.h"
#include "mysofa.h"

void *tr_convolve_class;

void tr_convolve_free(tr_convolve *x)
{
    dsp_free((t_pxobject *)x);
    vas_fir_static_s2s_free(x->convolutionEngine);
}

void tr_convolve_assist(tr_convolve *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) {
		switch (a) {
		case 0:
			sprintf(s,"(Signal) This + Right Inlet");
			break;
		case 1:
			sprintf(s,"(Signal) Left Inlet + This");
			break;
		}
	}
	else
		sprintf(s,"(Signal) Addition Result");
}

void tr_convolve_perform64(tr_convolve *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    vas_fir_static_s2s_process(x->convolutionEngine, ins[0], ins[1], outs[0], outs[1], sampleframes);
}

void tr_convolve_dsp64(tr_convolve *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, tr_convolve_perform64, 0, NULL);
}

void tr_convolve_read(tr_convolve *x, t_symbol *s, long argc, t_atom *argv)
{
    char fullpath[MAX_PATH_CHARS];
    char fileExtension[MAX_PATH_CHARS];

    vas_maxObjectUtilities_openFile(argc, argv, fullpath, fileExtension);
    
    if(!strcmp(fileExtension, "sofa"))
    {
#ifdef USE_LIBMYSOFA
        vas_2chFilter_readSofa((vas_filter *) x->filter, fullpath, x->filter->left->firSetup);
#else
        post("Sofa not supported for this binary, compile again with USE_LIBMYSOFA");
#endif
    }
    
    if(!strcmp(fileExtension, "txt"))
        vas_fir_readText_1IrPerLine((vas_fir *) x->convolutionEngine, fullpath);
    
    vas_fir_setInitFlag((vas_fir *)x->convolutionEngine);
    
    vas_dynamicFirChannel_getSharedFilterValues(x->convolutionEngine->left2right, x->convolutionEngine->right);
    vas_dynamicFirChannel_getSharedFilterValues(x->convolutionEngine->right2left, x->convolutionEngine->left);
    vas_dynamicFirChannel_prepareArrays(x->convolutionEngine->left2right);
    vas_dynamicFirChannel_prepareArrays(x->convolutionEngine->right2left);
}

void *tr_convolve_new(t_symbol *s, long argc, t_atom *argv)
{
    tr_convolve *x = object_alloc(tr_convolve_class);
    if(x != NULL)
    {
        dsp_setup((t_pxobject *)x,2);
        outlet_new((t_pxobject *)x, "signal");
        outlet_new((t_pxobject *)x, "signal");
        x->x_obj.z_misc = Z_NO_INPLACE;
        x->segmentSize = 64;
        x->filterSize = 0;
        
        if(argc >=1 && atom_gettype(argv) == A_LONG)
                x->segmentSize = atom_getlong(argv);
        
        x->convolutionEngine = vas_fir_static_s2s_new(VAS_VDSP | VAS_LOCALFILTER , x->segmentSize);
    }
    
    return (x);
}

void ext_main(void *r)
{
    t_class *c = class_new("tr.convolve~", (method)tr_convolve_new, (method)tr_convolve_free, sizeof(tr_convolve), NULL, A_GIMME, 0);
    post("tr.convolve~ v0.1");
    post("By Thomas Resch, Hochschule für Musik Basel, FHNW, Audiocommunication Group, TU-Berlin");
    class_addmethod(c, (method)tr_convolve_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)tr_convolve_assist,"assist",A_CANT,0);
    class_addmethod(c, (method)tr_convolve_read,            "read",        A_GIMME, 0);
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    tr_convolve_class = c;
}

