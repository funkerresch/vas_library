#include "tr_hproom1~.h"

void *tr_hproom1_class;

void tr_hproom1_free(tr_hproom1 *x)
{
    dsp_free((t_pxobject *)x);
    vas_fir_binaural_6reflections_free(x->binauralEngine);
}

void tr_hproom1_assist(tr_hproom1 *x, void *b, long m, long a, char *s)
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

void tr_hproom1_setGroundDelay(tr_hproom1 *x, long delayTime)
{
    vas_interpolDelay_setDelayTime(x->binauralEngine->groundDelay, delayTime);
}

void tr_hproom1_setAzimuth(tr_hproom1 *x, long azimuth)
{
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->left, azimuth);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->right, azimuth);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->frontReflectionLeft, azimuth+20);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->frontReflectionRight, azimuth+20);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->leftReflectionLeft, azimuth+85);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->leftReflectionRight, azimuth+85);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->rightReflectionLeft, azimuth+270);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->rightReflectionRight, azimuth+270);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->groundReflectionLeft, azimuth+10);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->groundReflectionRight, azimuth+10);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->ceilingReflectionLeft, azimuth-10);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->ceilingReflectionRight, azimuth-10);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->backReflectionLeft, azimuth+170);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->backReflectionRight, azimuth+170);
}

void tr_hproom1_setElevation(tr_hproom1 *x, long elevation)
{
    vas_dynamicFirChannel_setElevation(x->binauralEngine->left, elevation);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->right, elevation);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->frontReflectionLeft, elevation+3);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->frontReflectionRight, elevation+3);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->leftReflectionLeft, elevation+10);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->leftReflectionRight, elevation+10);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->leftReflectionLeft, elevation+15);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->leftReflectionRight, elevation+15);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->backReflectionLeft, elevation+2);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->backReflectionRight, elevation+2);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->groundReflectionLeft, elevation-60);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->groundReflectionRight, elevation-60);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->ceilingReflectionLeft, elevation+60);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->ceilingReflectionRight, elevation-60);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->groundReflectionLeft, elevation+45);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->groundReflectionRight, elevation+45);
}

void tr_hproom1_setQ(tr_hproom1 *x, double freq)
{
    //vas_iir_2highpass2lowpass_setHighpassCutoff(x->filter->groundFilter, freq);
    
}

void tr_hproom1_setFreq(tr_hproom1 *x, double freq)
{
    vas_iir_biquad_setFrequency(x->binauralEngine->groundFilterHp, freq);
}

void tr_hproom1_perform64(tr_hproom1 *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    vas_fir_binaural_6reflections_process(x->binauralEngine, ins[0], outs[0], outs[1], sampleframes);
}

void tr_hproom1_dsp64(tr_hproom1 *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, tr_hproom1_perform64, 0, NULL);
}

void tr_hproom1_read(tr_hproom1 *x, t_symbol *s, long argc, t_atom *argv)
{
    char filename[MAX_PATH_CHARS];
    char fullpath[MAX_PATH_CHARS];
    const char *fileExtension;
    short path;
    
    t_fourcc type = FOUR_CHAR_CODE('FILE');
    
    if(argc)
    {
        if(atom_gettype(argv) == A_SYM)
            sprintf(filename, "%s", atom_getsym(argv)->s_name);
    }
    else
        sprintf(filename, "");
    
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
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->groundReflectionLeft, x->binauralEngine->groundReflectionRight);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->ceilingReflectionLeft, x->binauralEngine->ceilingReflectionRight);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->backReflectionLeft, x->binauralEngine->backReflectionRight);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->frontReflectionLeft, x->binauralEngine->frontReflectionRight);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->leftReflectionLeft, x->binauralEngine->leftReflectionRight);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->rightReflectionLeft, x->binauralEngine->rightReflectionRight);
}

void *tr_hproom1_new(t_symbol *s, long argc, t_atom *argv)
{
    tr_hproom1 *x = object_alloc(tr_hproom1_class);
    if(x != NULL)
    {
        dsp_setup((t_pxobject *)x,1);
        outlet_new((t_pxobject *)x, "signal");
        outlet_new((t_pxobject *)x, "signal");
        x->x_obj.z_misc = Z_NO_INPLACE;
        x->segmentSize = 64;
        x->filterSize = 0;
        
        if(argc >=1 && atom_gettype(argv) == A_LONG)
                x->segmentSize = atom_getlong(argv);
        
        x->binauralEngine = vas_fir_binaural_6reflections_new(VAS_VDSP | VAS_LOCALFILTER | VAS_BINAURALSETUP_STD, x->segmentSize, NULL);
    }
    
    return (x);
}

void ext_main(void *r)
{
    t_class *c = class_new("tr.hproom1~", (method)tr_hproom1_new, (method)tr_hproom1_free, sizeof(tr_hproom1), NULL, A_GIMME, 0);
    post("tr.hproom1~ v0.11");
    post("By Thomas Resch, Forschung & Entwicklung, Hochschule für Musik Basel, FHNW");
    post("Audiocommunication Group, TU-Berlin");
    class_addmethod(c, (method)tr_hproom1_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)tr_hproom1_assist,"assist",A_CANT,0);
    class_addmethod(c,(method)tr_hproom1_setAzimuth, "azimuth", A_DEFLONG, 0);
    class_addmethod(c,(method)tr_hproom1_setElevation, "elevation", A_DEFLONG, 0);
    class_addmethod(c,(method)tr_hproom1_setGroundDelay, "grounddelay", A_DEFLONG, 0);
    class_addmethod(c,(method)tr_hproom1_setQ, "hp", A_FLOAT, 0);
    class_addmethod(c,(method)tr_hproom1_setFreq, "lp", A_FLOAT, 0);
    class_addmethod(c, (method)tr_hproom1_read,            "read",        A_GIMME, 0);
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    tr_hproom1_class = c;
}

