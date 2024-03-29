//
//  vas_maxObjectUtil.c
//  ak.binaural~
//
//  Created by Admin on 15.03.18.
//

#ifdef MAXMSPSDK
#include "vas_maxObjectUtil.h"


void vas_util_getFileExtension1(const char *filename, char *fileExtension)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename)
        sprintf(fileExtension, "");
    sprintf(fileExtension, "%s", dot + 1);
}

void vas_maxObjectUtilities_getFullPath(t_symbol *ir, char *fullPath)
{
    char filename[MAX_PATH_CHARS];
    short path;
    t_fourcc type = FOUR_CHAR_CODE('TEXT');
    
    sprintf(filename, "%s", ir->s_name);
    
    if (locatefile_extended(filename,&path,&type,&type,1))
    {
        error("Can't find file %s",filename);
        return;
    }
    
    path_toabsolutesystempath(path, filename, fullPath);
}

void vas_maxObjectUtilities_openFile1(t_symbol *ir, char *fullPath)
{
    char filename[MAX_PATH_CHARS];
    char fileExtension[MAX_PATH_CHARS];
    t_fourcc type;
    short path;
 
    sprintf(filename, "%s", ir->s_name);
    vas_util_getFileExtension1(filename, fileExtension);
    
    if(!strcmp("txt", fileExtension))
        type = FOUR_CHAR_CODE('TEXT');
    else
        type = FOUR_CHAR_CODE('FILE');
    
    if (!strcmp(filename, ""))
    {
        filename[0] = 0;
        if (open_dialog(filename, &path, &type, &type, 1))
            return;
    }
    else
    {
        if (locatefile_extended(filename,&path,&type,&type,1))
        {
            error("Can't find file %s",filename);
            return;
        }
    }
    
    path_toabsolutesystempath(path, filename, fullPath);
}

void vas_maxObjectUtilities_openFile(long argc, t_atom *argv, char *fullPath, char *fileExtension)
{
    char filename[MAX_PATH_CHARS];
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
        if (locatefile_extended(filename,&path,&type,&type,1))
        {
            error("Can't find file %s",filename);
            return;
        }
    }
    
    path_toabsolutesystempath(path, filename, fullPath);
    vas_util_getFileExtension1(filename, fileExtension);
}
#endif
