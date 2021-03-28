#include "vas_binaural~.h"
#include "vas_maxObjectUtil.h"

void *vas_binaural_class;

void vas_binaural_assist(vas_binaural *x, void *b, long m, long a, char *s)
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

void vas_binaural_setAzimuth(vas_binaural *x, long azimuth)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setAzimuth(binauralEngine->left, azimuth);
    vas_dynamicFirChannel_setAzimuth(binauralEngine->right, azimuth);
}

void vas_binaural_setElevation(vas_binaural *x, long elevation)
{
    vas_fir_binaural *binauralEngine = (vas_fir_binaural *)x->convolutionEngine;
    vas_dynamicFirChannel_setElevation(binauralEngine->left, elevation);
    vas_dynamicFirChannel_setElevation(binauralEngine->right, elevation);
}

void vas_binaural_perform64(vas_binaural *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    vas_util_double2SinglePrecision(ins[0], x->inputBuffer, sampleframes);
    vas_fir_binaural_process(x->convolutionEngine, x->inputBuffer, x->outL, x->outR, sampleframes);
    vas_util_single2DoublePrecision(x->outL, outs[0], sampleframes);
    vas_util_single2DoublePrecision(x->outR, outs[1], sampleframes);
}

void vas_binaural_dsp64(vas_binaural *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, vas_binaural_perform64, 0, NULL);
}

void vas_binaural_read(vas_binaural *x, t_symbol *s)
{
    vas_pdmaxobject_read((vas_pdmaxobject *)x, s, x->segmentSize, 0, 0);
}

void *vas_binaural_new(t_symbol *s, long argc, t_atom *argv)
{
    vas_binaural *x = object_alloc(vas_binaural_class);
    if(x != NULL)
    {
        dsp_setup((t_pxobject *)x,1);
        outlet_new((t_pxobject *)x, "signal");
        outlet_new((t_pxobject *)x, "signal");
        //x->x_obj.z_misc = Z_NO_INPLACE;
        x->segmentSize = 256;
        x->filterSize = 0;
        x->fullpath[0] = '\0';
        x->aziDirection = 1;
        x->eleDirection = 1;
        t_symbol *path = NULL;
        int offset = 0;
        int end = 0;

        if(argc >= 1)
        {
            if(argv[0].a_type == A_LONG)
            {
                x->segmentSize = atom_getlong(argv);
            }
            
            if(argv[0].a_type == A_SYM)
                path = atom_getsym(argv);
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
            if(argv[1].a_type == A_SYM)
                path = atom_getsym(argv+1);
        }
        
        if(argc >= 3)
        {
            if(argv[2].a_type == A_LONG)
                offset = atom_getlong(argv+2);
        }
        
        if(argc >= 4)
        {
            if(argv[3].a_type == A_LONG)
                end = atom_getlong(argv+3);
        }
        
        if(path)
        { 
            vas_pdmaxobject_read((vas_pdmaxobject *)x, path, x->segmentSize, offset, end);
        }
    }
    
    return (x);
}

void vas_binaural_free(vas_binaural *x)
{
    dsp_free((t_pxobject *)x);
    vas_fir_binaural_free(x->convolutionEngine);
}

void ext_main(void *r)
{
    t_class *c = class_new("vas.binaural~", (method)vas_binaural_new, (method)vas_binaural_free, sizeof(vas_binaural), NULL, A_GIMME, 0);
    post("vas.binaural~ v0.5");
    post("By Thomas Resch, Forschung & Entwicklung, Hochschule für Musik Basel, FHNW");
    post("Audiocommunication Group, TU-Berlin");
    class_addmethod(c, (method)vas_binaural_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)vas_binaural_assist,"assist",A_CANT,0);
    class_addmethod(c,(method)vas_binaural_setAzimuth, "azimuth", A_DEFLONG, 0);
    class_addmethod(c,(method)vas_binaural_setElevation, "elevation", A_DEFLONG, 0);
    class_addmethod(c, (method)vas_binaural_read,            "read",        A_DEFSYM, 0);
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    vas_binaural_class = c;
}

