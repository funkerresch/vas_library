/**
 * @file vas_maxObjectUtil.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Tools for calculating convolution based virtual acoustics (mainly dynamic binaural synthesis) <br>
 * <br>
 * @brief Max/MSP Utilties for open files <br>
 * <br>
 * Max/MSP SDK suggests using the Max/MSP - routines
 * instead of fopen
 */

#ifndef vas_maxObjectUtil_h
#define vas_maxObjectUtil_h

#ifdef MAXMSPSDK
#include "ext.h"
#include "ext_obex.h"


#include <stdio.h>

void vas_maxObjectUtilities_getFullPath(t_symbol *ir, char *fullPath);

void vas_maxObjectUtilities_openFile1(t_symbol *ir, char *fullPath);

void vas_maxObjectUtilities_openFile(long argc, t_atom *argv, char *fullPath, char *fileExtension);


#endif /* vas_maxObjectUtil_h */
#endif
