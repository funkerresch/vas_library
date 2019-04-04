/**
 * @file stp_defines.h
 * @author Thomas Resch <br>
 * Audiocommunication Group, Technical University Berlin <br>
 * University of Applied Sciences Nordwestschweiz (FHNW), Music-Academy, Research and Development <br>
 * Preprocessor Instructions for the stp library
 */

#ifndef stp_defines_h
#define stp_defines_h

/** Data Type Macro for flexible input vector single or double floating point precision */
#define STP_INPUTVECTOR_USE_FLOAT
#define STP_OUTPUTVECTOR_USE_FLOAT

#ifdef STP_INPUTVECTOR_USE_FLOAT
typedef float STP_INPUTVECTOR;
#else
typedef double STP_INPUTVECTOR;
#endif

/** Data Type Macro for flexible output vector single or double floating point precision */
#ifdef STP_OUTPUTVECTOR_USE_FLOAT
typedef float STP_OUTPUTVECTOR;
#else
typedef double STP_OUTPUTVECTOR;
#endif

#endif /* stp_defines_h */
