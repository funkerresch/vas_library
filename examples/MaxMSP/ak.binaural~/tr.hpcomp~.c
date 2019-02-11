#include "tr_hpcomp~.h"

void *tr_hpcomp_class;

void tr_hpcomp_free(tr_hpcomp *x)
{
    dsp_free((t_pxobject *)x);
    vas_filter_headphoneCompensation_free(x->filter);
}

void tr_hpcomp_assist(tr_hpcomp *x, void *b, long m, long a, char *s)
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

void tr_hpcomp_perform64(tr_hpcomp *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    vas_filter_headphoneCompensation_process(x->filter, ins[0], ins[1], outs[0], outs[1], sampleframes);
}

void tr_hpcomp_dsp64(tr_hpcomp *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    object_method(dsp64, gensym("dsp_add64"), x, tr_hpcomp_perform64, 0, NULL);
}

void tr_hpcomp_read(tr_hpcomp *x, t_symbol *s, long argc, t_atom *argv)
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
        vas_fir_readSofa((vas_fir *) x->filter, fullpath, x->filter->left->filter->firSetup);
#else
    post("Sofa not supported for this binary, compile again with USE_LIBMYSOFA");
#endif
    }
    
    if(!strcmp(fileExtension, "txt"))
        vas_fir_readText_1IrPerLine((vas_fir *) x->filter, fullpath);
    
    vas_fir_setInitFlag((vas_fir *)x->filter);
}

void *tr_hpcomp_new(t_symbol *s, long argc, t_atom *argv)
{
    tr_hpcomp *x = object_alloc(tr_hpcomp_class);
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
        
        x->filter = vas_filter_headphoneCompensation_new(VAS_VDSP | VAS_LOCALFILTER | VAS_STATICFILTER, x->segmentSize);
    }
    
    return (x);
}

void ext_main(void *r)
{
    t_class *c = class_new("tr.hpcomp~", (method)tr_hpcomp_new, (method)tr_hpcomp_free, sizeof(tr_hpcomp), NULL, A_GIMME, 0);
    post("tr.hpcomp~ v0.1");
    post("By Thomas Resch, Forschung & Entwicklung, Hochschule für Musik Basel, FHNW");
    post("Audiocommunication Group, TU-Berlin");
    class_addmethod(c, (method)tr_hpcomp_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)tr_hpcomp_assist,"assist",A_CANT,0);
    class_addmethod(c, (method)tr_hpcomp_read,            "read",        A_GIMME, 0);
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    tr_hpcomp_class = c;
}

