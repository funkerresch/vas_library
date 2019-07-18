/**
 * @file vas_filter.c
 * @author Thomas Resch
 * @date 4 Jan 2018
 * @brief C - "Base" class for all filters <br>
 *
 * All functions shared by the "real" filter classes (binaural, staticFir etc..) <br>
 * are implemented in vas_filter. <br>
 * The class vas_filter itself should be considered virtual <br>
 *
 */

#define VAS_USE_LIBMYSOFA

#include "vas_fir.h"
#ifdef VAS_USE_LIBMYSOFA
#include "mysofa.h"
#endif
#include "string.h"

void vas_filter_metaData_init(vas_fir_metaData *x)
{
    x->audioFormat = 0;
    x->azimuthStride = 1;
    x->elevationStride = 1;
    x->directionFormat = 0;
    x->filterLength = 0;
    x->lineFormat = 0;
}

void vas_fir_setInitFlag(vas_fir *x)
{
    vas_dynamicFirChannel_setInitFlag(x->left);
    vas_dynamicFirChannel_setInitFlag(x->right);
}

int vas_fir_getInitFlag(vas_fir *x)
{
    if(x->left->init)
        return 1;
    else
        return 0;
}

void vas_fir_removeInitFlag(vas_fir *x)
{
    vas_dynamicFirChannel_removeInitFlag(x->left);
    vas_dynamicFirChannel_removeInitFlag(x->right);
}

void vas_fir_prepareChannelsWithSharedFilter(vas_fir *x,  vas_dynamicFirChannel *left, vas_dynamicFirChannel *right)
{
    vas_dynamicFirChannel_getSharedFilterValues(left, x->left);
    vas_dynamicFirChannel_getSharedFilterValues(right, x->right);
    vas_dynamicFirChannel_prepareArrays(left);
    vas_dynamicFirChannel_prepareArrays(right);
}

#ifdef VAS_USE_LIBMYSOFA
int vas_fir_readSofa(vas_fir *x, char *fullpath, vas_dynamicFirChannel_config *firSetup)
{
    int err = 0;
    struct MYSOFA_EASY *hrtf;
    int filterLength;
    
    hrtf = mysofa_open(fullpath, 44100, &filterLength, &err);
    
    if(err)
    {
#if defined(MAXMSPSDK) || defined(PUREDATA)
        post("Can't open %s", fullpath);
        post("error %d", err);
#else
        printf("Can't open %s", fullpath);
        printf("error %d", err);
#endif
        return err;
    }
    else
    {
#if defined(MAXMSPSDK) || defined(PUREDATA)
        post("Opened %s", fullpath);
        post("Filtersize is: %d", filterLength);
#else
        printf("Opened %s", fullpath);
        printf("Filtersize is: %d", filterLength);
#endif
    }
    
    vas_dynamicFirChannel_setFilterSize(x->left, filterLength);
    vas_dynamicFirChannel_setFilterSize(x->right, filterLength);
    vas_dynamicFirChannel_prepareArrays(x->left);
    vas_dynamicFirChannel_prepareArrays(x->right);
    
    float leftIR[filterLength]; // [-1. till 1]
    float rightIR[filterLength];
    float leftDelay;          // unit is sec.
    float rightDelay;         // unit is sec.
    
    int eleRange = firSetup->eleRange;
    int aziRange = firSetup->aziRange;
    int eleStride = firSetup->eleStride;
    int aziStride = firSetup->aziStride;
    int eleZero = firSetup->eleZero;
    
#if defined(MAXMSPSDK) || defined(PUREDATA)
    post("EleRange: %d", eleRange);
    post("AziRange: %d", aziRange);
#else
    printf("EleRange: %d", eleRange);
    printf("AziRange: %d", aziRange);
#endif
    
    for(int eleCount = 0; eleCount < eleRange; eleCount++)
    {
        for(int aziCount = 0; aziCount < aziRange; aziCount++)
        {
            int azi = aziCount * aziStride;
            int ele = eleCount * eleStride;
            
            float azim = vas_utilities_degrees2radians(azi);
            float elev = vas_utilities_degrees2radians(ele - eleZero*eleStride);
            float xx = cosf(elev)*cosf(azim);
            float y = cosf(elev)*sinf(azim);
            float z = sinf(elev);
            
            mysofa_getfilter_float(hrtf , xx, y, z, leftIR, rightIR, &leftDelay, &rightDelay);
            
            vas_dynamicFirChannel_prepareFilter(x->left, leftIR,  eleCount, aziCount);
            vas_dynamicFirChannel_prepareFilter(x->right, rightIR,  eleCount, aziCount);
        }
    }
    
    mysofa_close(hrtf);
    return err;
}
#endif

vas_dynamicFirChannel *extractChannel2(vas_fir *x, char *line)
{
    if(strstr(line, "left") != NULL)
    {
        return x->left;
    }
    
    if(strstr(line, "right") != NULL)
    {
        return x->right;
    }
    
    return NULL;
}

static int vas_filter_extractMetaDataFromText(vas_fir *x, FILE *filePtr, char **line)
{
    size_t len;
    size_t read;
    char *value;
    int error = 0;
    char *lineAdr;
    
    while ((read = vas_getline(line, &len, filePtr)) != -1)
    {
        lineAdr = *line;
        if(strstr(*line, "metadata") != NULL)
            ;
        else if(strstr(*line, "length"))
        {
            value = vas_strsep(&lineAdr, " ");
            value = vas_strsep(&lineAdr, " ");
            x->description.filterLength = atoi(value);
            
            vas_dynamicFirChannel_setFilterSize(x->left, x->description.filterLength);
            vas_dynamicFirChannel_setFilterSize(x->right, x->description.filterLength);
                
            vas_dynamicFirChannel_prepareArrays(x->left);
            vas_dynamicFirChannel_prepareArrays(x->right);
            
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("filterlength: %d", x->description.filterLength);
#endif
#endif
        }
        
        else if(strstr(*line, "elevationstride"))
        {
            value = vas_strsep(&lineAdr, " ");
            value = vas_strsep(&lineAdr, " ");
            x->description.elevationStride = atoi(value);
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("elestride: %d", x->description.elevationStride);
#endif
#endif
        }
        
        else if(strstr(*line, "azimuthstride"))
        {
            value = vas_strsep(&lineAdr, " ");
            value = vas_strsep(&lineAdr, " ");
            x->description.azimuthStride = atoi(value);
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("azistride: %d", x->description.azimuthStride);
#endif
#endif
        }
        
        else if(strstr(*line, "directionformat"))
        {
            if(strstr(*line, "single"))
                x->description.directionFormat = VAS_IR_DIRECTIONFORMAT_SINGLE;
            if(strstr(*line, "multiazimuth"))
                x->description.directionFormat = VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH;
            if(strstr(*line, "azimuth"))
                x->description.directionFormat = VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH;
            if(strstr(*line, "multi"))
                x->description.directionFormat = VAS_IR_DIRECTIONFORMAT_MULTI;
            if(strstr(*line, "multiall"))
                x->description.directionFormat = VAS_IR_DIRECTIONFORMAT_MULTI;
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("directionFormat: %d", x->description.directionFormat);
#endif
#endif
        }
        
        else if(strstr(*line, "audioformat"))
        {
            if(strstr(*line, "mono"))
                x->description.audioFormat = VAS_IR_AUDIOFORMAT_MONO;
            if(strstr(*line, "stereo"))
                x->description.audioFormat = VAS_IR_AUDIOFORMAT_STEREO;
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("audioformat: %d", x->description.audioFormat);
#else
            printf("audioformat: %d", x->description.audioFormat);
#endif
#endif
        }
        
        else if(strstr(*line, "lineformat"))
        {
            if(strstr(*line, "ir"))
                x->description.lineFormat = VAS_IR_LINEFORMAT_IR;
            if(strstr(*line, "value"))
                x->description.lineFormat = VAS_IR_LINEFORMAT_VALUE;
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("lineformat: %d", x->description.lineFormat);
#else
            printf("lineformat: %d", x->description.lineFormat);
#endif
#endif
        }
        
        else if(strstr(*line, "}") >= 0)
            return error;

        else;
        
        
    }
    
    return error;
}

static void vas_filter_extractAngleFromText(char *angle, int *currentAngle, char **line)
{
    int index = 0;
    while( **line != ',')
    {
        angle[index] = **line;
        (*line)++;
        index++;
    }
    
    (*line)++; // skip white space after angle
    angle[index] = '\0';
    *currentAngle = atoi(angle);
}

static void vas_filter_read_lineFormat_value(vas_fir *x, FILE *filePtr, char **line)
{
    int currentIrIndex2Read = 0;
    char *filterCoefficients = NULL;
    float fvalue = 0;
    float *currentIr = NULL;
    bool getChannel = 1;
    
    size_t len = 0;
    size_t read = 0;
    
    vas_dynamicFirChannel *currentChannel2Read = x->left;
    
    currentIr = (float *)vas_mem_alloc(x->left->filterSize * sizeof(float));
    
    while ((read = vas_getline(line, &len, filePtr)) != -1)
    {
        if(getChannel)
        {
            currentIrIndex2Read = 0;
            currentChannel2Read = extractChannel2(x, *line);
            getChannel = false;
        }
        else
        {
            filterCoefficients = *line;
            if(*filterCoefficients == '}')
            {
                getChannel = true;
                vas_dynamicFirChannel_prepareFilter(currentChannel2Read, currentIr, 0, 0);
            }
            else
            {
                fvalue = atof(filterCoefficients);
                if(currentIrIndex2Read < x->description.filterLength)
                {
                    currentIr[currentIrIndex2Read] = fvalue;
                    currentIrIndex2Read++;
                }
            }
        }
    }
    
    if(currentIr)
        vas_mem_free(currentIr);
}

/*static void vas_filter_print_currentIr(vas_fir *x, float *ir, int length)
{
    for(int i = 0;i<length;i++)
        ;// post("%f", ir[i]);
}

static void vas_filter_compress_lineFormat_ir(vas_fir *x, char *fullpath)
{
    char * line = NULL;
    FILE *filePtr = fopen(fullpath, "r");
    
    if (!filePtr)
    {
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
        post("Could not open: %s", fullpath);
#endif
#endif
        return;
    }
    else
    {
        vas_filter_extractMetaDataFromText(x, filePtr, &line);
        
        if(x->description.lineFormat == VAS_IR_LINEFORMAT_IR)
        {
            
        }

    }
}*/

static void vas_filter_read_lineFormat_ir(vas_fir *x, FILE *filePtr, char **line)
{
    int currentAzimuth2Read = 0;
    int currentElevation2Read = 0;
    int currentIrIndex2Read = 0;
    char *filterCoefficients = NULL;
    char angle[50];
    float fvalue = 0;
    float *currentIr = NULL;
    bool getChannel = 1;
    char *lineAdr;
    
    vas_dynamicFirChannel *currentChannel2Read = x->left;
    
    size_t len = 0;
    size_t read = 0;
    
    currentIr = (float *)vas_mem_alloc(x->description.filterLength * sizeof(float));
    x->left->filter->filterLength = x->description.filterLength;
    x->right->filter->filterLength = x->description.filterLength;

    while ((read = vas_getline(line, &len, filePtr)) != -1)
    {
        lineAdr = *line;
        if( (x->description.audioFormat >= VAS_IR_AUDIOFORMAT_STEREO) && getChannel)
        {
            currentChannel2Read = extractChannel2(x, lineAdr);
            getChannel = false;
        }
        else
        {
            if(x->description.directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI)
            {
                vas_filter_extractAngleFromText(angle, &currentElevation2Read, &lineAdr);
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
                post("Elevation: %d", currentElevation2Read);
#else
                printf("Elevation: %d", currentElevation2Read);
#endif
#endif
            }
            
            if(x->description.directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH
               || x->description.directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI)
            {
                
                vas_filter_extractAngleFromText(angle, &currentAzimuth2Read, &lineAdr);
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
                post("Azimuth: %d", currentAzimuth2Read);
#else
                printf("Azimuth: %d", currentAzimuth2Read);
#endif
#endif
            }
            
            currentIrIndex2Read = 0;
            
            while( (filterCoefficients = vas_strsep(&lineAdr,", ")) != NULL )
            {
                if(strlen(filterCoefficients) != 0 )
                {
                    if(*filterCoefficients == '}')
                        getChannel = true;
                    else
                    {
                        fvalue = atof(filterCoefficients);
                        if(currentIrIndex2Read < x->description.filterLength)
                        {
                            currentIr[currentIrIndex2Read] = fvalue;
                            currentIrIndex2Read++;
                        }
                    }
                }
            }
            vas_dynmaicFirChannel_resetMinMaxAverageSegmentPower(currentChannel2Read, currentElevation2Read/x->description.elevationStride, currentAzimuth2Read/x->description.azimuthStride);
            vas_dynamicFirChannel_prepareFilter(currentChannel2Read, currentIr, currentElevation2Read/x->description.elevationStride, currentAzimuth2Read/x->description.azimuthStride);
        }
    }
   
    if(currentIr)
        vas_mem_free(currentIr);
#if defined(MAXMSPSDK) || defined(PUREDATA)
    post("Min Average Segment Power Left: %.14f", x->left->filter->minAverageSegmentPower[0][0]);
    post("Max Average Segment Power Left: %.14f", x->left->filter->maxAverageSegmentPower[0][0]);
    post("Number of Segments Left: %d", x->left->filter->numberOfSegments);
    post("Segments below Threshhold Left: %d", x->left->filter->zeroCounter[0][0]);
    
    post("Min Average Segment Power Right: %.14f", x->right->filter->minAverageSegmentPower[0][0]);
    post("Max Average Segment Power Right: %.14f", x->right->filter->maxAverageSegmentPower[0][0]);
    post("Number of Segments Right: %d", x->right->filter->numberOfSegments);
    post("Segments below Threshhold Right: %d", x->right->filter->zeroCounter[0][0]);
#endif
}

void vas_fir_readText_1IrPerLine(vas_fir *x, char *fullpath)
{
    char * line = NULL;
    FILE *filePtr = fopen(fullpath, "r");
    
    if (!filePtr)
    {
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
        post("Could not open: %s", fullpath);
#endif
#endif
        return;
    }
    else
    {
        vas_filter_extractMetaDataFromText(x, filePtr, &line);
        
        if(x->description.lineFormat == VAS_IR_LINEFORMAT_IR)
            vas_filter_read_lineFormat_ir(x, filePtr, &line);
        if(x->description.lineFormat == VAS_IR_LINEFORMAT_VALUE)
            vas_filter_read_lineFormat_value(x, filePtr, &line);
        
        fclose(filePtr);
        if(line)
            free(line);
        
    }
 }
