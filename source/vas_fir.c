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

#include "vas_fir.h"
#ifdef VAS_USE_LIBMYSOFA
#include "mysofa.h"
#endif
#include "string.h"

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
    x->left->filter->referenceCounter++;
    x->right->filter->referenceCounter++;
}

void vas_fir_setDirectionFormat(vas_fir *x, int directionFormat)
{
    if(directionFormat == VAS_IR_DIRECTIONFORMAT_SINGLE)
    {
        x->metaData.directionFormat = VAS_IR_DIRECTIONFORMAT_SINGLE;
        x->metaData.eleMin = 0;
        x->metaData.eleMax = 1;
    }

    if(directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH)
    {
        x->metaData.directionFormat = VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH;
        x->metaData.eleMin = 0;
        x->metaData.eleMax = 1;
    }
    
    if(directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI)
    {
        x->metaData.directionFormat = VAS_IR_DIRECTIONFORMAT_MULTI;
        x->metaData.eleMin = -90;
        x->metaData.eleMax = 90;
    }
}

void vas_fir_setMetaData(vas_fir *x, int directionFormat, int length)
{
    x->metaData.filterLength = length;
    vas_fir_setDirectionFormat(x, directionFormat);
    x->metaData.audioFormat = VAS_IR_AUDIOFORMAT_STEREO;
    x->metaData.lineFormat = VAS_IR_LINEFORMAT_IR;
    x->metaData.azimuthStride = 3;
    x->metaData.elevationStride = 3;
}

#ifdef VAS_USE_LIBMYSOFA

void* vas_fir_readSofa_getMetaData(vas_fir *x, char *fullpath)
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
        return NULL;
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
        x->metaData.filterLength = filterLength;
        x->metaData.elevationStride = 3;
        x->metaData.azimuthStride = 3;
        return hrtf;
    }
}

int vas_fir_readSofa_getFilter(vas_fir *x, void *filter)
{
    int err = 0;
    struct MYSOFA_EASY *hrtf = filter;
    int filterLength = x->metaData.filterLength;
    
    float leftIR[filterLength]; // [-1. till 1]
    float rightIR[filterLength];
    float leftDelay;          // unit is sec.
    float rightDelay;         // unit is sec.
    
    int eleRange = 180;
    int aziRange = 360;
    int eleStride = 3;
    int aziStride = 3;
    int eleZero = 90;
    
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
/*
 char *fullPath;
 int filterLength;
 int filterOffset;
 int filterEnd;
 int directionFormat;
 int audioFormat;
 int lineFormat;
 int azimuthStride;
 int elevationStride;
 int eleZero;
 int eleRange;
 int eleMin;
 int eleMax;
 int aziRange;
 */
static int vas_filter_checkMetaData(vas_fir *x)
{
    int error = 0;
    if(x->metaData.filterLength <= 0)
        error |= VAS_ERROR_METADATA;
    if(x->metaData.filterOffset < 0)
        error |= VAS_ERROR_METADATA;
    if(x->metaData.filterEnd < 0)
        error |= VAS_ERROR_METADATA;
    if(x->metaData.directionFormat <=  VAS_UNDEFINED || x->metaData.directionFormat > VAS_IR_DIRECTIONFORMAT_MULTI)
        error |= VAS_ERROR_METADATA;
    if(x->metaData.filterLength <= 0)
        error |= VAS_ERROR_METADATA;
    if(x->metaData.filterLength <= 0)
        error |= VAS_ERROR_METADATA;
    if(x->metaData.filterLength <= 0)
        error |= VAS_ERROR_METADATA;

    return error;
}

static int vas_filter_extractMetaDataFromText1(vas_fir *x, FILE *filePtr, char **line)
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
            x->metaData.filterLength = atoi(value);
            
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("filterlength: %d", x->metaData.filterLength);
#endif
#endif
        }
        
        else if(strstr(*line, "elevationstride"))
        {
            value = vas_strsep(&lineAdr, " ");
            value = vas_strsep(&lineAdr, " ");
            x->metaData.elevationStride = atoi(value);
            if(x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_SINGLE || x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH)
            {
                x->metaData.eleRange = 1;
                x->metaData.eleZero = 0;
            }
            else
            {
                x->metaData.eleRange = 180 /x->metaData.elevationStride;
                x->metaData.eleZero = x->metaData.eleRange/2;
            }
 
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("elestride: %d", x->metaData.elevationStride);
#endif
#endif
        }
        
        else if(strstr(*line, "azimuthstride"))
        {
            value = vas_strsep(&lineAdr, " ");
            value = vas_strsep(&lineAdr, " ");
            x->metaData.azimuthStride = atoi(value);
            if(x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_SINGLE)
                x->metaData.aziRange = 1;
            else
                x->metaData.aziRange = 360/x->metaData.azimuthStride;

#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("azistride: %d", x->metaData.azimuthStride);
#endif
#endif
        }
        // { .aziRange = 1, .aziStride = 1, .eleMin = 0, .eleMax = 1, .eleRange = 1, .eleStride = 1, .eleZero = 0};
        else if(strstr(*line, "directionformat"))
        {
            if(strstr(*line, "single"))
                vas_fir_setDirectionFormat(x, VAS_IR_DIRECTIONFORMAT_SINGLE);

            if(strstr(*line, "multiazimuth") || strstr(*line, "azimuth"))
                vas_fir_setDirectionFormat(x, VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH);

            if(strstr(*line, "multi") || strstr(*line, "multiall"))
                vas_fir_setDirectionFormat(x, VAS_IR_DIRECTIONFORMAT_MULTI);
            
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("directionFormat: %d", x->metaData.directionFormat);
#endif
#endif
        }
        
        else if(strstr(*line, "audioformat"))
        {
            if(strstr(*line, "mono"))
                x->metaData.audioFormat = VAS_IR_AUDIOFORMAT_MONO;
            if(strstr(*line, "stereo"))
                x->metaData.audioFormat = VAS_IR_AUDIOFORMAT_STEREO;
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("audioformat: %d", x->metaData.audioFormat);
#else
            printf("audioformat: %d", x->description.audioFormat);
#endif
#endif
        }
        
        else if(strstr(*line, "lineformat"))
        {
            if(strstr(*line, "ir"))
                x->metaData.lineFormat = VAS_IR_LINEFORMAT_IR;
            if(strstr(*line, "value"))
                x->metaData.lineFormat = VAS_IR_LINEFORMAT_VALUE;
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
            post("lineformat: %d", x->metaData.lineFormat);
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

void vas_fir_setMetaData_manually(vas_fir *x, int filterLength, int segmentSize, int directionFormat, int eleStride, int aziStride, int audioFormat, int lineFormat)
{
    x->metaData.filterLength = filterLength;
    vas_fir_setDirectionFormat(x, directionFormat);
    x->metaData.elevationStride = eleStride;
    if(x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_SINGLE || x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH)
    {
        x->metaData.eleRange = 1;
        x->metaData.eleZero = 0;
    }
    else
    {
        x->metaData.eleRange = 180 /x->metaData.elevationStride;
        x->metaData.eleZero = x->metaData.eleRange/2;
    }
 
    x->metaData.azimuthStride = aziStride;
    if(x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_SINGLE)
        x->metaData.aziRange = 1;
    else
        x->metaData.aziRange = 360/x->metaData.azimuthStride;

    x->metaData.audioFormat = audioFormat;
    x->metaData.lineFormat = lineFormat;
    
    vas_fir_initFilter2(x, segmentSize, 0);
    
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
    post("filterlength: %d", x->metaData.filterLength);
    post("elestride: %d", x->metaData.elevationStride);
    post("azistride: %d", x->metaData.azimuthStride);
    post("audioformat: %d", x->metaData.audioFormat);
    
#else
    printf("filterlength: %d", x->description.filterLength);
    printf("elestride: %d", x->description.elevationStride);
    printf("azistride: %d", x->description.azimuthStride);
    printf("audioformat: %d", x->description.audioFormat);
#endif
#endif
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
                if(currentIrIndex2Read < x->metaData.filterLength)
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

static void vas_filter_read_lineFormat_ir1(vas_fir *x, FILE *filePtr, char **line, int offset)
{
    int currentAzimuth2Read = 0;
    int currentIndex2Write = 0;
    int currentElevation2Read = 0;
    int currentIrIndex2Read = 0;
    int filterLenghtMinusOffset = 0;
    char *filterCoefficients = NULL;
    char angle[50];
    float fvalue = 0;
    float *currentIr = NULL;
    bool getChannel = 1;
    char *lineAdr;
    
    vas_dynamicFirChannel *currentChannel2Read = x->left;
    
    size_t len = 0;
    size_t read = 0;
    
    if(offset >= x->metaData.filterLength)
    {
#if(defined(MAXMSPSDK) || defined(PUREDATA))
        post("Offset greater than filter length.");
#else
        printf("Offset greater than filter length.");
#endif
        offset = 0;
    }
    
    filterLenghtMinusOffset = x->metaData.filterLength-offset;
    
    currentIr = (float *)vas_mem_alloc( filterLenghtMinusOffset * sizeof(float));
    x->left->filter->filterLength = filterLenghtMinusOffset;
    x->right->filter->filterLength = filterLenghtMinusOffset;

    while ((read = vas_getline(line, &len, filePtr)) != -1)
    {
        lineAdr = *line;
        if( (x->metaData.audioFormat >= VAS_IR_AUDIOFORMAT_STEREO) && getChannel)
        {
            currentChannel2Read = extractChannel2(x, lineAdr);
            getChannel = false;
        }
        else
        {
            if(x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI)
            {
                vas_filter_extractAngleFromText(angle, &currentElevation2Read, &lineAdr);
            }
            
            if(x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH
               || x->metaData.directionFormat == VAS_IR_DIRECTIONFORMAT_MULTI)
            {
                vas_filter_extractAngleFromText(angle, &currentAzimuth2Read, &lineAdr);
            }
            
            currentIrIndex2Read = 0;
            currentIndex2Write = 0;
            
            while( (filterCoefficients = vas_strsep(&lineAdr,", ")) != NULL )
            {
                if(strlen(filterCoefficients) != 0 )
                {
                    if(*filterCoefficients == '}')
                        getChannel = true;
                    else
                    {
                        fvalue = atof(filterCoefficients);
                        if(currentIrIndex2Read < x->metaData.filterLength && currentIrIndex2Read >= offset)
                        {
                            currentIr[currentIndex2Write] = fvalue;
                            currentIndex2Write++;
                            currentIrIndex2Read++;
                        }
                        else
                            currentIrIndex2Read++;
                    }
                }
            }
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
            vas_dynmaicFirChannel_resetMinMaxAverageSegmentPower(currentChannel2Read, currentElevation2Read/x->metaData.elevationStride, currentAzimuth2Read/x->metaData.azimuthStride);
#endif
            vas_dynamicFirChannel_prepareFilter(currentChannel2Read, currentIr, currentElevation2Read/x->metaData.elevationStride, currentAzimuth2Read/x->metaData.azimuthStride);
        }
    }
   
    if(currentIr)
        vas_mem_free(currentIr);
#ifdef VAS_WITH_AVERAGE_SEGMENTPOWER
#if defined(MAXMSPSDK) || defined(PUREDATA)
    post("Min Average Segment Power Left: %.14f", x->left->filter->minAverageSegmentPower[0][0]);
    post("Max Average Segment Power Left: %.14f", x->left->filter->maxAverageSegmentPower[0][0]);
   // post("Number of Segments Left: %d", x->left->filter->numberOfSegments);
    post("Segments below Threshhold Left: %d", x->left->filter->zeroCounter[0][0]);
    
    post("Min Average Segment Power Right: %.14f", x->right->filter->minAverageSegmentPower[0][0]);
    post("Max Average Segment Power Right: %.14f", x->right->filter->maxAverageSegmentPower[0][0]);
   // post("Number of Segments Right: %d", x->right->filter->numberOfSegments);
    post("Segments below Threshhold Right: %d", x->right->filter->zeroCounter[0][0]);
#endif
#endif
}

void vas_fir_initFilter2(vas_fir *x, int segmentSize, int offset)
{
    vas_dynamicFirChannel_init1(x->left, &x->metaData, segmentSize, offset);
    vas_dynamicFirChannel_init1(x->right,&x->metaData, segmentSize, offset);
}

FILE *vas_fir_readText_metaData1(vas_fir *x, char *fullpath)
{
    char * line = NULL;
    size_t size = strlen(fullpath);
    FILE *filePtr = fopen(fullpath, "r");
    
    if (!filePtr)
    {
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
        post("Could not open: %s", fullpath);
#endif
#endif
        return NULL;
    }
    else
    {
        x->metaData.fullPath = vas_mem_alloc(sizeof(char) * size);
        strcpy(x->metaData.fullPath, fullpath);
        vas_filter_extractMetaDataFromText1(x, filePtr, &line);
        
        if(line)
            free(line);
        
        return filePtr;
    }
}

void vas_fir_readText_Ir1(vas_fir *x, FILE *filePtr, int offset)
{
    char * line = NULL;
    
    if (!filePtr)
    {
#ifdef VERBOSE
#if(defined(MAXMSPSDK) || defined(PUREDATA))
        post("Could not open");
#endif
#endif
        return;
    }
    else
    {
        if(x->metaData.lineFormat == VAS_IR_LINEFORMAT_IR)
            vas_filter_read_lineFormat_ir1(x, filePtr, &line, offset);
        if(x->metaData.lineFormat == VAS_IR_LINEFORMAT_VALUE)
            vas_filter_read_lineFormat_value(x, filePtr, &line);
        
        fclose(filePtr);
        if(line)
            free(line);
    }
}

void vas_fir_test_4096_1024_azimuthStride3(vas_fir *x)
{
    vas_fir_setMetaData_manually(x, 8192, 32, VAS_IR_DIRECTIONFORMAT_MULTI_AZIMUTH, 1, 3, VAS_IR_AUDIOFORMAT_STEREO, VAS_IR_LINEFORMAT_IR);
    float currentIr[8192];
    
    for(int i = 0; i < 8192; i++)
        currentIr[i] = 0;
    
    currentIr[0] = 0.1;
    currentIr[2000] = 0.1;
    currentIr[4000] = 0.1;
    currentIr[8000] = 0.1;
     
    for(int i = 0; i < 360; i+=x->metaData.azimuthStride)
    {
        if(i % 6 == 0)
        {
            currentIr[0] = 0.1;
            currentIr[2000] = 0.1;
            currentIr[4000] = 0.1;
            currentIr[8000] = 0.1;
            
            currentIr[1000] = 0;
            currentIr[3000] = 0;
            currentIr[6000] = 0;
        }
        else
        {
            currentIr[1000] = 0.1;
            currentIr[3000] = 0.1;
            currentIr[6000] = 0.1;
            
            currentIr[0] = 0;
            currentIr[2000] = 0;
            currentIr[4000] = 0;
            currentIr[8000] = 0;
        }
        //float damping = i * 0.1;
        //currentIr[0] = 1./(damping+1.);
        //post("%d %f", i, currentIr[0]);
    
        vas_dynamicFirChannel_prepareFilter(x->left, currentIr, 0, i/x->metaData.azimuthStride);
        vas_dynamicFirChannel_prepareFilter(x->right, currentIr, 0, i/x->metaData.azimuthStride);
    }
    
    vas_fir_setInitFlag(x);
}
