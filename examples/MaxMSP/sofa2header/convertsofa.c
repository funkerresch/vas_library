/**
	@file
	sofa2headerfile - show use of text reading and editing

	@ingroup	examples
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include "mysofa.h"

typedef struct sofa2headerfile
{
	t_object ob;			// the object itself (must be first)
	t_object *t_editor;
	long t_size;
    char  **t_text;
    char *ptr2handle;
    t_symbol *arrayName;
    int eleMin;
    int eleMax;
    int eleRange;
    int eleZero;
    int eleZeroArrayIndex;
    int eleStride;
    int aziStride;
    int aziRange;
    int eleArrayRange;
    int aziArrayRange;
    
    float *real;
    float *imag;
    float complex;
    
    float farray[1000];
    char value2write[50];
    int farrayIndex;
} sofa2headerfile;

void *sofa2headerfile_new(t_symbol *s, long argc, t_atom *argv);
void sofa2headerfile_free(sofa2headerfile *x);
void sofa2headerfile_assist(sofa2headerfile *x, void *b, long m, long a, char *s);
void sofa2headerfile_read(sofa2headerfile *x, t_symbol *s);
void convertIR(sofa2headerfile *x, t_symbol *s, long argc, t_atom *argv);

void *sofa2headerfile_class;

float degrees2radians(float degrees)
{
    return degrees * (M_PI/180);
}

float radians2degrees(float radians)
{
    return radians * (180/M_PI);
}

void sofa2headerfile_assist(sofa2headerfile *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
		sprintf(s, "Message In");
}

void sofa2headerfile_free(sofa2headerfile *x)
{
	if (x->t_text)
		sysmem_freehandle(x->t_text);
}

void sofa2headerfile_read(sofa2headerfile *x, t_symbol *s)
{
    t_atom args[1];
    atom_setsym(args, gensym("tosofa"));
	defer((t_object *)x, (method)convertIR, s, 1, args);
}

void sofa2text_read(sofa2headerfile *x, t_symbol *s)
{
    t_atom args[1];
    atom_setsym(args, gensym("totext"));
    defer((t_object *)x, (method)convertIR, s, 1, args);
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        return "";
    return dot + 1;
}

static void sofa2text(int *err, char *filename, char *fullpath, sofa2headerfile *x)
{
    struct MYSOFA_EASY *hrtf;
    int filterLength;
    hrtf = mysofa_open(fullpath, 44100, &filterLength, err);
    
    if(hrtf==NULL)
        error("Could not open %s", filename);
    else
    {
        post("Open %s successfully.", filename);
        
        char writeHeaderChar[10000];
        long err;
        t_handle h;
        t_filehandle fh;
        t_fourcc type = FOUR_CHAR_CODE('TEXT');
        t_fourcc outtype;
        
        char filename[MAX_PATH_CHARS];
        short path;
        filename[0] = 0;
        
        if (saveasdialog_extended(filename, &path, &outtype, &type, 1))
            return;
        
        h = sysmem_newhandle(0);
        
        err = path_createsysfile(filename, path, 'TEXT', &fh);
        if (err)
            return;
        
        float leftIR[filterLength]; // [-1. till 1]
        float rightIR[filterLength];
        float leftDelay;          // unit is sec.
        float rightDelay;         // unit is sec.
        int azimuth = 0;
        int elevation = 0;
        
        sprintf(writeHeaderChar, "metadata {\n");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "length %d\n", filterLength);
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        if(x->eleRange > 1 && x->aziRange > 1)
        {
            sprintf(writeHeaderChar, "directionformat multi\n");
            sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        }
        else if(x->eleRange == 1 && x->aziRange > 1)
        {
            sprintf(writeHeaderChar, "directionformat multiazimuth\n");
            sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        }
        else if(x->eleRange == 1 && x->aziRange == 1)
        {
            sprintf(writeHeaderChar, "directionformat single\n");
            sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        }
        else
        {
            error("Error in configuration");
            sysfile_close(fh);
            sysmem_freehandle(h);
            return;
        }
        
        sprintf(writeHeaderChar, "audioformat stereo\n");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "lineformat ir\n");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "elevationstride %d\n", x->eleStride);
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "azimuthstride %d\n", x->aziStride);
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "}");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "\nleft {");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        /*metadata {
        length 16
        directionformat multi
        audioformat stereo
        lineformat ir
        elevationstride 90
        azimuthstride 90
        }*/
        
        for( ; elevation < x->eleRange; elevation+=x->eleStride)
        {
            for(azimuth = 0; azimuth < x->aziRange; azimuth+= x->aziStride)
            {
                float azim = degrees2radians(azimuth);
                float elev = degrees2radians(elevation + x->eleMin) ;
                
                float xx = cosf(elev)*cosf(azim);
                float y = cosf(elev)*sinf(azim);
                float z = sinf(elev);
                
                mysofa_getfilter_float(hrtf , xx, y, z, leftIR, rightIR, &leftDelay, &rightDelay);
                
                sprintf(writeHeaderChar, "\n%d, %d, ", elevation, azimuth);
                sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                
                for (int hrtfIndex = 0 ; hrtfIndex < filterLength ; hrtfIndex++)
                {
                    sprintf(writeHeaderChar, "%f, ", leftIR[hrtfIndex]);
                    sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                }
            }
        }
        
        sprintf(writeHeaderChar, "%s", "};");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "\n");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "right {");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        elevation = 0;
        
        for(; elevation < x->eleRange; elevation+=x->eleStride)
        {
            for(azimuth = 0; azimuth < x->aziRange; azimuth+=x->aziStride)
            {
                float azim = degrees2radians(azimuth);
                float elev = degrees2radians(elevation + x->eleMin);
                
                float xx = cosf(elev)*cosf(azim);
                float y = cosf(elev)*sinf(azim);
                float z = sinf(elev);
                
                mysofa_getfilter_float(hrtf , xx, y, z, leftIR, rightIR, &leftDelay, &rightDelay);
                
                sprintf(writeHeaderChar, "\n%d, %d, ", elevation, azimuth);
                sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                
                for (int hrtfIndex = 0 ; hrtfIndex < filterLength ; hrtfIndex++)
                {
                    sprintf(writeHeaderChar, "%f, ", rightIR[hrtfIndex]);
                    sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                }
            }
        }
        
        sprintf(writeHeaderChar, "%s", "};");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));

        
        err = sysfile_writetextfile(fh, h, TEXT_LB_NATIVE);
        sysfile_close(fh);
        sysmem_freehandle(h);
    }
}

static void sofa2header(int *err, char *filename, char *fullpath, sofa2headerfile *x)
{
    struct MYSOFA_EASY *hrtf;
    int filterLength;
    hrtf = mysofa_open(fullpath, 44100, &filterLength, err);
    
    if(hrtf==NULL)
        error("Could not open %s", filename);
    else
    {
        post("Open %s successfully.", filename);
        
        char writeHeaderChar[10000];
        long err;
        t_handle h;
        t_filehandle fh;
        t_fourcc type = FOUR_CHAR_CODE('TEXT');
        t_fourcc outtype;
        
        char filename[MAX_PATH_CHARS];
        short path;
        filename[0] = 0;
        
        if (saveasdialog_extended(filename, &path, &outtype, &type, 1))
            return;
        
        h = sysmem_newhandle(0);
        
        err = path_createsysfile(filename, path, 'TEXT', &fh);
        if (err)
            return;
        
        float leftIR[filterLength]; // [-1. till 1]
        float rightIR[filterLength];
        float leftDelay;          // unit is sec.
        float rightDelay;         // unit is sec.
        int azimuth = 0;
        int elevation = 0;
        
        sprintf(writeHeaderChar, "float %sLeft[%d][%d][%d] = {\n",x->arrayName->s_name, x->eleArrayRange, x->aziArrayRange, filterLength );
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        for( ; elevation < x->eleRange; elevation+=x->eleStride)
        {
            sprintf(writeHeaderChar, "%s\n", "{");
            sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
            
            for(azimuth = 0; azimuth < x->aziRange; azimuth+= x->aziStride)
            {
                sprintf(writeHeaderChar, "%s", "{");
                sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                
                float azim = degrees2radians(azimuth);
                float elev = degrees2radians(elevation + x->eleMin) ;
                
                float xx = cosf(elev)*cosf(azim);
                float y = cosf(elev)*sinf(azim);
                float z = sinf(elev);
                
                mysofa_getfilter_float(hrtf , xx, y, z, leftIR, rightIR, &leftDelay, &rightDelay);
                
                sprintf(writeHeaderChar, "\n/*\nElevation: %d Azimuth: %d\n*/\n", (int)elevation+x->eleMin, azimuth);
                sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                
                for (int hrtfIndex = 0 ; hrtfIndex < filterLength ; hrtfIndex++)
                {
                    sprintf(writeHeaderChar, "%f, ", leftIR[hrtfIndex]);
                    sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                }
                
                sprintf(writeHeaderChar, "%s", "},\n");
                sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
            }
            
            sprintf(writeHeaderChar, "%s", "},\n");
            sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        }
        
        sprintf(writeHeaderChar, "%s", "};");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "\n\n");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        sprintf(writeHeaderChar, "float %sRight[%d][%d][%d] = {\n",x->arrayName->s_name, x->eleArrayRange, x->aziArrayRange, filterLength );
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        elevation = 0;
        
        for(; elevation < x->eleRange; elevation+=x->eleStride)
        {
            sprintf(writeHeaderChar, "%s\n", "{");
            sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
            
            for(azimuth = 0; azimuth < x->aziRange; azimuth+=x->aziStride)
            {
                sprintf(writeHeaderChar, "%s", "{");
                sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                
                float azim = degrees2radians(azimuth);
                float elev = degrees2radians(elevation + x->eleMin);
                
                float xx = cosf(elev)*cosf(azim);
                float y = cosf(elev)*sinf(azim);
                float z = sinf(elev);
                
                mysofa_getfilter_float(hrtf , xx, y, z, leftIR, rightIR, &leftDelay, &rightDelay);
                
                sprintf(writeHeaderChar, "\n/*\nElevation: %d Azimuth: %d\n*/\n", (int)elevation+x->eleMin, azimuth);
                sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                
                for (int hrtfIndex = 0 ; hrtfIndex < filterLength ; hrtfIndex++)
                {
                    sprintf(writeHeaderChar, "%f, ", rightIR[hrtfIndex]);
                    sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
                }
                
                sprintf(writeHeaderChar, "%s", "},\n");
                sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
            }
            
            sprintf(writeHeaderChar, "%s", "},\n");
            sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        }
        
        sprintf(writeHeaderChar, "%s", "};");
        sysmem_ptrandhand(writeHeaderChar,h,strlen(writeHeaderChar));
        
        err = sysfile_writetextfile(fh, h, TEXT_LB_NATIVE);
        sysfile_close(fh);
        sysmem_freehandle(h);
    }
}

void convertIR(sofa2headerfile *x, t_symbol *s, long argc, t_atom *argv)
{
	char filename[MAX_PATH_CHARS];
    char fullpath[MAX_PATH_CHARS];
    const char *fileExtension;
	short path;
	t_fourcc type = FOUR_CHAR_CODE('FILE');
    
    int err;

	if (s == gensym("")) {
		filename[0] = 0;

		if (open_dialog(filename, &path, &type, &type, 1))
			return;
	} else {
		strcpy(filename,s->s_name);
		if (locatefile_extended(filename,&path,&type,&type,1)) {
			object_error((t_object *)x, "can't find file %s",filename);
			return;
		}
	}
    
    path_toabsolutesystempath(path, filename, fullpath);
    fileExtension = get_filename_ext(filename);
    
    if(!strcmp(fileExtension, "sofa"))
    {
        if(atom_getsym(argv) == gensym("tosofa"))
            sofa2header(&err, filename, fullpath, x);
        if(atom_getsym(argv) == gensym("totext"))
            sofa2text(&err, filename, fullpath, x);
    }
}

void sofa2hheaderfile_configure(sofa2headerfile *x, t_symbol *s, long argc, t_atom *argv)
{
    if(argc < 4)
        return;
    
    for(int i = 0;i < 4; i++)
    {
        if(atom_gettype(argv+i) != A_LONG)
        {
            error("configure expects 4 integer values: eleRange, eleGrid; aziRange, aziGrid");
            return;
        }
    }
    
    int eleRange = atom_getlong(&argv[0]);
    if(eleRange >= 1 && eleRange <= 180)
        x->eleRange = eleRange;
    
    int eleStride = atom_getlong(&argv[1]);
    if(eleStride >= 1 && eleStride < 90)
        x->eleStride = eleStride;
    
    int aziRange = atom_getlong(&argv[2]);
    if(aziRange >= 1 && aziRange <= 360)
           x->aziRange = aziRange;
       
    int aziStride = atom_getlong(&argv[3]);
    if(aziStride >= 1 && aziStride < 180)
           x->aziStride = aziStride;
    
    x->eleArrayRange = x->eleRange/x->eleStride;
    x->aziArrayRange = x->aziRange/x->aziStride;
    x->eleMin = - (x->eleRange/2);
    x->eleMax = x->eleRange/2;
 }

void *sofa2headerfile_new(t_symbol *s, long argc, t_atom *argv)
{
	sofa2headerfile *x = NULL;

	x = (sofa2headerfile *)object_alloc(sofa2headerfile_class);

	x->t_text = sysmem_newhandle(0);
    x->ptr2handle = NULL;
	x->t_size = 0;
	x->t_editor = NULL;
    x->eleMin = -90;
    x->eleMax = 90;
    x->eleStride = 3;
    x->aziStride = 3;
    x->eleRange = abs(x->eleMin) + x->eleMax;
    x->aziRange = 360;
    x->eleArrayRange = x->eleRange/x->eleStride;
    x->aziArrayRange = x->aziRange/x->aziStride;
    x->eleZeroArrayIndex = abs(x->eleMin) / x->eleStride;
    x->arrayName = gensym("hrtf");
    x->farrayIndex = 0;

	return x;
}

void ext_main(void *r)
{
    t_class *c;
    
    c = class_new("convertsofa", (method)sofa2headerfile_new, (method)sofa2headerfile_free, (long)sizeof(sofa2headerfile),
                  0L, A_GIMME, 0);
    
    class_addmethod(c, (method)sofa2hheaderfile_configure, "configuregrid", A_GIMME, 0);
    class_addmethod(c, (method)sofa2headerfile_read, "convert2header", A_DEFSYM, 0);
    class_addmethod(c, (method)sofa2text_read, "convert2text", A_DEFSYM, 0);
    
    class_register(CLASS_BOX, c);
    sofa2headerfile_class = c;
}
