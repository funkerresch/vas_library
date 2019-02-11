//
//  rwa_object.c
//  rwa_binauralsimple~
//
//  Created by Admin on 23.01.19.
//  Copyright © 2019 Intrinsic Audio. All rights reserved.
//

#include "rwa_firobject.h"

void rwa_firobject_read(rwa_firobject *x, t_symbol *s)
{
    const char *fileExtension;
    const char *filename = s->s_name;
    
    unsigned long length = strlen(x->canvasDirectory);
    
    if(x->canvasDirectory[length-1] == '/')
        sprintf(x->fullpath, "%s%s", x->canvasDirectory, filename);
    else
        sprintf(x->fullpath, "%s/%s", x->canvasDirectory, filename);
    
    post("IR Path: %s", x->fullpath);
    fileExtension = vas_util_getFileExtension(filename);
    
    if(!strcmp(fileExtension, "sofa"))
    {
#ifdef USE_LIBMYSOFA
        vas_fir_readSofa(x->filter, fullpath, x->filter->left->firSetup);
#else
        post("Sofa not supported for this binary, compile again with USE_LIBMYSOFA");
#endif
    }
    if(!strcmp(fileExtension, "txt"))
    {
        post("Read Text");
        vas_fir_readText_1IrPerLine((vas_fir *)x->convolutionEngine, x->fullpath);
    }
    
    vas_fir_setInitFlag((vas_fir *)x->convolutionEngine);
}
