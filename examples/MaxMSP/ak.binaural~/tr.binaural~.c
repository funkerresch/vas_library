#include "tr_binaural~.h"

void *tr_binaural_class;

void tr_binaural_assist(tr_binaural *x, void *b, long m, long a, char *s)
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

void tr_binaural_setAzimuth(tr_binaural *x, long azimuth)
{
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->left, azimuth);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->right, azimuth);
}

void tr_binaural_setElevation(tr_binaural *x, long elevation)
{
    vas_dynamicFirChannel_setElevation(x->binauralEngine->left, elevation);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->right, elevation);
}

void tr_binaural_perform64(tr_binaural *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    vas_fir_binaural_process(x->binauralEngine, ins[0], outs[0], outs[1], sampleframes);
}

void tr_binaural_dsp64(tr_binaural *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, tr_binaural_perform64, 0, NULL);
}

void tr_binaural_read(tr_binaural *x, t_symbol *s)
{
    char filename[MAX_PATH_CHARS];
    char fullpath[MAX_PATH_CHARS];
    const char *fileExtension;
    short path;
    
    t_fourcc type = FOUR_CHAR_CODE('TEXT');
    sprintf(filename, "%s", s->s_name);
    
    if (!strcmp(filename, ""))
    {
        filename[0] = 0;
        if (open_dialog(filename, &path, &type, &type, 1))
            return;
    }
    else
    {
        strcpy(filename,s->s_name);
        if (locatefile_extended(filename,&path,&type,&type,1))
        {
            object_error((t_object *)x, "can't find file %s",filename);
            return;
        }
    }
    
    path_toabsolutesystempath(path, filename, fullpath);
    fileExtension = vas_util_getFileExtension(filename);
    
    if(!strcmp(fileExtension, "sofa"))
    {
#ifdef USE_LIBMYSOFA
        vas_fir_readSofa((vas_fir *) x->binauralEngine, fullpath, x->binauralEngine->left->filter->firSetup);
#else
    post("Sofa not supported for this binary, compile again with USE_LIBMYSOFA");
#endif
    }
    
    if(!strcmp(fileExtension, "txt"))
        vas_fir_readText_1IrPerLine((vas_fir *) x->binauralEngine, fullpath);
    
    vas_fir_setInitFlag((vas_fir *)x->binauralEngine);
}

void *tr_binaural_new(t_symbol *s, long argc, t_atom *argv)
{
    tr_binaural *x = object_alloc(tr_binaural_class);
    if(x != NULL)
    {
        dsp_setup((t_pxobject *)x,1);
        outlet_new((t_pxobject *)x, "signal");
        outlet_new((t_pxobject *)x, "signal");
        x->x_obj.z_misc = Z_NO_INPLACE;
        x->segmentSize = 64;
        x->filterSize = 0;
        x->useGlobalFilter = 0;
        t_symbol *path = NULL;
        int flags = 0 | VAS_BINAURALSETUP_NOELEVATION | VAS_VDSP;
        
        if(x->useGlobalFilter)
            flags |= VAS_GLOBALFILTER;
        else
            flags |= VAS_LOCALFILTER;
        
        if(argc >=1 && atom_gettype(argv) == A_LONG)
                x->segmentSize = atom_getlong(argv);
        
        if(argc >= 1 && atom_gettype(argv) == A_SYM)
            path = atom_getsym(argv);
        
        x->binauralEngine = vas_fir_binaural_new(flags, x->segmentSize, NULL);
        
        if(argc >= 2 && atom_gettype(argv+1) == A_SYM)
            path = atom_getsym(argv+1);
        
        if(path)
        {
            if(!vas_fir_getInitFlag((vas_fir *)x->binauralEngine))
            {
                tr_binaural_read(x, path);
                post("Read File %s", path->s_name);
            }
            else
            {
                vas_dynamicFirChannel_initArraysWithGlobalFilter(x->binauralEngine->left);
                vas_dynamicFirChannel_initArraysWithGlobalFilter(x->binauralEngine->right);
            }
        }
    }
    
    return (x);
}

void tr_binaural_free(tr_binaural *x)
{
    dsp_free((t_pxobject *)x);
    vas_fir_binaural_free(x->binauralEngine);
}

void ext_main(void *r)
{
    t_class *c = class_new("tr.binaural~", (method)tr_binaural_new, (method)tr_binaural_free, sizeof(tr_binaural), NULL, A_GIMME, 0);
    post("tr.binaural~ v0.2");
    post("By Thomas Resch, Forschung & Entwicklung, Hochschule für Musik Basel, FHNW");
    post("Audiocommunication Group, TU-Berlin");
    class_addmethod(c, (method)tr_binaural_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)tr_binaural_assist,"assist",A_CANT,0);
    class_addmethod(c,(method)tr_binaural_setAzimuth, "azimuth", A_DEFLONG, 0);
    class_addmethod(c,(method)tr_binaural_setElevation, "elevation", A_DEFLONG, 0);
    class_addmethod(c, (method)tr_binaural_read,            "read",        A_DEFSYM, 0);
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    tr_binaural_class = c;
}

