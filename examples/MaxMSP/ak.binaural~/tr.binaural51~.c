#include "tr_binaural51.h"

void *tr_binaural51_class;

void tr_binaural51_free(tr_binaural51 *x)
{
    dsp_free((t_pxobject *)x);
    vas_fir_binaural_51_free(x->binauralEngine);
}

void tr_binaural51_assist(tr_binaural51 *x, void *b, long m, long a, char *s)
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

void tr_binaural51_setAzimuth(tr_binaural51 *x, long azimuth)
{
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->left2left, azimuth+VAS_AZI_OFFSET_51_LEFT);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->left2right, azimuth+VAS_AZI_OFFSET_51_LEFT);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->center2left, azimuth);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->center2left, azimuth);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->right2left, azimuth+VAS_AZI_OFFSET_51_RIGHT);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->right2right, azimuth+VAS_AZI_OFFSET_51_RIGHT);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->surroundLeft2left, azimuth+VAS_AZI_OFFSET_51_SURROUNDLEFT);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->surroundLeft2left, azimuth+VAS_AZI_OFFSET_51_SURROUNDLEFT);
    
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->surroundRight2left, azimuth+VAS_AZI_OFFSET_51_SURROUNDRIGHT);
    vas_dynamicFirChannel_setAzimuth(x->binauralEngine->surroundRight2left, azimuth+VAS_AZI_OFFSET_51_SURROUNDRIGHT);
}

void tr_binaural51_setElevation(tr_binaural51 *x, long elevation)
{
    vas_dynamicFirChannel_setElevation(x->binauralEngine->left2left, elevation);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->left2right, elevation);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->center2left, elevation);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->center2right, elevation);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->right2left, elevation);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->right2right, elevation);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->surroundLeft2left, elevation);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->surroundLeft2left, elevation);
    
    vas_dynamicFirChannel_setElevation(x->binauralEngine->surroundRight2left, elevation);
    vas_dynamicFirChannel_setElevation(x->binauralEngine->surroundRight2left, elevation);
}

void tr_binaural51_perform64(tr_binaural51 *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    vas_fir_binaural_51_process(x->binauralEngine, ins, outs[0], outs[1], sampleframes);
}

void tr_binaural51_dsp64(tr_binaural51 *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, tr_binaural51_perform64, 0, NULL);
}

void tr_binaural51_read(tr_binaural51 *x, t_symbol *s, long argc, t_atom *argv)
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
        vas_fir_readSofa((vas_fir *) x->binauralEngine, fullpath, x->binauralEngine->left2left->filter->firSetup);
#else
        post("Sofa not supported for this binary, compile again with USE_LIBMYSOFA");
#endif
    }
    
    if(!strcmp(fileExtension, "txt"))
        vas_fir_readText_1IrPerLine((vas_fir *) x->binauralEngine, fullpath);
    
    vas_fir_setInitFlag((vas_fir *)x->binauralEngine);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->center2left, x->binauralEngine->center2right);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->right2left, x->binauralEngine->right2right);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->surroundLeft2left, x->binauralEngine->surroundLeft2right);
    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x->binauralEngine, x->binauralEngine->surroundRight2left, x->binauralEngine->surroundRight2right);
}

void *tr_binaural51_new(t_symbol *s, long argc, t_atom *argv)
{
    tr_binaural51 *x = object_alloc(tr_binaural51_class);
    if(x != NULL)
    {
        dsp_setup((t_pxobject *)x,6);
        outlet_new((t_pxobject *)x, "signal");
        outlet_new((t_pxobject *)x, "signal");
        
        x->x_obj.z_misc = Z_NO_INPLACE;
        x->segmentSize = 64;
        x->filterSize = 0;
        
        if(argc >=1 && atom_gettype(argv) == A_LONG)
            x->segmentSize = atom_getlong(argv);
        
        x->binauralEngine = vas_fir_binaural_51_new(VAS_VDSP | VAS_LOCALFILTER | VAS_BINAURALSETUP_STD, x->segmentSize, NULL);
    }
    
    return (x);
}

void ext_main(void *r)
{
    t_class *c = class_new("tr.binaural51~", (method)tr_binaural51_new, (method)tr_binaural51_free, sizeof(tr_binaural51), NULL, A_GIMME, 0);
    post("tr.binaural51~ v0.11");
    post("By Thomas Resch, Forschung & Entwicklung, Hochschule für Musik Basel, FHNW");
    post("Audiocommunication Group, TU-Berlin");
    class_addmethod(c, (method)tr_binaural51_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)tr_binaural51_assist,"assist",A_CANT,0);
    class_addmethod(c,(method)tr_binaural51_setAzimuth, "azimuth", A_DEFLONG, 0);
    class_addmethod(c,(method)tr_binaural51_setElevation, "elevation", A_DEFLONG, 0);
    class_addmethod(c, (method)tr_binaural51_read,            "read",        A_GIMME, 0);
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    tr_binaural51_class = c;
}


