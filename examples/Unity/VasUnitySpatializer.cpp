#include "AudioPluginUtil.h"
#include "vas_fir_binauralReflection.h"
#include "vas_delay_crossfade.h"
#include "vas_iir_biquad.h"
#include "vas_delay_crossfade.h"

#ifndef DEBUG_UNITY
#define DEBUG_UNITY
typedef void (*FuncPtr)( const char * );
FuncPtr Debug = NULL;
#endif

#define VAS_MAXNUMBEROFRAYS 6
#define VAS_MAXREFLECTIONORDER 10
//#define VAS_DEBUG_TO_UNITY

#define VAS_SPAT_CONFIG_SIMPLE 1
#define VAS_SPAT_CONFIG_MANUAL 2
#define VAS_SPAT_CONFIG_AUTO 3

using namespace std;

namespace Spatializer
{
    enum
    {
        P_AUDIOSRCATTN,
        P_FIXEDVOLUME,
        P_CUSTOMFALLOFF,
        P_SPATID,
        P_H_SOURCEDIRECTIVITY,
        P_H_FULLPOWERRANGE,
        P_V_SOURCEDIRECTIVITY,
        P_V_FULLPOWERRANGE,
        P_DIRECTIVITYDAMPING,
        P_NUMBEROFRAYS,
        P_REFLECTIONORDER,
        P_INVERSEAZI,
        P_INVERSEELE,
        P_BYPASS,
        P_LISTENERORIENTATIONONLY,
        P_SEGMENTSIZE_EARLYPART,
        P_SEGMENTSIZE_LATEPART,
        
        P_REF_1_1_X, //9
        P_REF_1_1_Y,
        P_REF_1_1_Z,
        
        P_REF_1_2_X, //13
        P_REF_1_2_Y,
        P_REF_1_2_Z,
        
        P_REF_1_3_X,
        P_REF_1_3_Y,
        P_REF_1_3_Z,
        
        P_REF_1_4_X,
        P_REF_1_4_Y,
        P_REF_1_4_Z,
        
        P_REF_1_5_X,
        P_REF_1_5_Y,
        P_REF_1_5_Z,
        
        P_REF_1_6_X,
        P_REF_1_6_Y,
        P_REF_1_6_Z,
        
        P_REF_1_7_X,
        P_REF_1_7_Y,
        P_REF_1_7_Z,
        
        P_REF_1_8_X,
        P_REF_1_8_Y,
        P_REF_1_8_Z,
        
        P_REF_1_9_X,
        P_REF_1_9_Y,
        P_REF_1_9_Z,
        
        P_REF_1_10_X,
        P_REF_1_10_Y,
        P_REF_1_10_Z,
        
        P_REF_2_1_X, //9
        P_REF_2_1_Y,
        P_REF_2_1_Z,
        
        P_REF_2_2_X, //13
        P_REF_2_2_Y,
        P_REF_2_2_Z,
        
        P_REF_2_3_X,
        P_REF_2_3_Y,
        P_REF_2_3_Z,
        
        P_REF_2_4_X,
        P_REF_2_4_Y,
        P_REF_2_4_Z,
        
        P_REF_2_5_X,
        P_REF_2_5_Y,
        P_REF_2_5_Z,
        
        P_REF_2_6_X,
        P_REF_2_6_Y,
        P_REF_2_6_Z,
        
        P_REF_2_7_X,
        P_REF_2_7_Y,
        P_REF_2_7_Z,
        
        P_REF_2_8_X,
        P_REF_2_8_Y,
        P_REF_2_8_Z,
        
        P_REF_2_9_X,
        P_REF_2_9_Y,
        P_REF_2_9_Z,
        
        P_REF_2_10_X,
        P_REF_2_10_Y,
        P_REF_2_10_Z,
        
        P_REF_3_1_X, //9
        P_REF_3_1_Y,
        P_REF_3_1_Z,
        
        P_REF_3_2_X, //13
        P_REF_3_2_Y,
        P_REF_3_2_Z,
        
        P_REF_3_3_X,
        P_REF_3_3_Y,
        P_REF_3_3_Z,
        
        P_REF_3_4_X,
        P_REF_3_4_Y,
        P_REF_3_4_Z,
        
        P_REF_3_5_X,
        P_REF_3_5_Y,
        P_REF_3_5_Z,
        
        P_REF_3_6_X,
        P_REF_3_6_Y,
        P_REF_3_6_Z,
        
        P_REF_3_7_X,
        P_REF_3_7_Y,
        P_REF_3_7_Z,
        
        P_REF_3_8_X,
        P_REF_3_8_Y,
        P_REF_3_8_Z,
        
        P_REF_3_9_X,
        P_REF_3_9_Y,
        P_REF_3_9_Z,
        
        P_REF_3_10_X,
        P_REF_3_10_Y,
        P_REF_3_10_Z,
        
        P_REF_4_1_X, //9
        P_REF_4_1_Y,
        P_REF_4_1_Z,
        
        P_REF_4_2_X, //13
        P_REF_4_2_Y,
        P_REF_4_2_Z,
        
        P_REF_4_3_X,
        P_REF_4_3_Y,
        P_REF_4_3_Z,
        
        P_REF_4_4_X,
        P_REF_4_4_Y,
        P_REF_4_4_Z,
        
        P_REF_4_5_X,
        P_REF_4_5_Y,
        P_REF_4_5_Z,
        
        P_REF_4_6_X,
        P_REF_4_6_Y,
        P_REF_4_6_Z,
        
        P_REF_4_7_X,
        P_REF_4_7_Y,
        P_REF_4_7_Z,
        
        P_REF_4_8_X,
        P_REF_4_8_Y,
        P_REF_4_8_Z,
        
        P_REF_4_9_X,
        P_REF_4_9_Y,
        P_REF_4_9_Z,
        
        P_REF_4_10_X,
        P_REF_4_10_Y,
        P_REF_4_10_Z,
        
        P_REF_5_1_X, //9
        P_REF_5_1_Y,
        P_REF_5_1_Z,
        
        P_REF_5_2_X, //13
        P_REF_5_2_Y,
        P_REF_5_2_Z,
        
        P_REF_5_3_X,
        P_REF_5_3_Y,
        P_REF_5_3_Z,
        
        P_REF_5_4_X,
        P_REF_5_4_Y,
        P_REF_5_4_Z,
        
        P_REF_5_5_X,
        P_REF_5_5_Y,
        P_REF_5_5_Z,
        
        P_REF_5_6_X,
        P_REF_5_6_Y,
        P_REF_5_6_Z,
        
        P_REF_5_7_X,
        P_REF_5_7_Y,
        P_REF_5_7_Z,
        
        P_REF_5_8_X,
        P_REF_5_8_Y,
        P_REF_5_8_Z,
        
        P_REF_5_9_X,
        P_REF_5_9_Y,
        P_REF_5_9_Z,
        
        P_REF_5_10_X,
        P_REF_5_10_Y,
        P_REF_5_10_Z,
        
        P_REF_6_1_X, //9
        P_REF_6_1_Y,
        P_REF_6_1_Z,
        
        P_REF_6_2_X, //13
        P_REF_6_2_Y,
        P_REF_6_2_Z,
        
        P_REF_6_3_X,
        P_REF_6_3_Y,
        P_REF_6_3_Z,
        
        P_REF_6_4_X,
        P_REF_6_4_Y,
        P_REF_6_4_Z,
        
        P_REF_6_5_X,
        P_REF_6_5_Y,
        P_REF_6_5_Z,
        
        P_REF_6_6_X,
        P_REF_6_6_Y,
        P_REF_6_6_Z,
        
        P_REF_6_7_X,
        P_REF_6_7_Y,
        P_REF_6_7_Z,
        
        P_REF_6_8_X,
        P_REF_6_8_Y,
        P_REF_6_8_Z,
        
        P_REF_6_9_X,
        P_REF_6_9_Y,
        P_REF_6_9_Z,
        
        P_REF_6_10_X,
        P_REF_6_10_Y,
        P_REF_6_10_Z,
        
        P_NUM
    };
    
    static int reflectionOffset = P_REF_1_1_X;

    struct EffectData
    {
        float p[P_NUM]; // Parameters
        float input[VAS_MAXVECTORSIZE];
        float outputL[VAS_MAXVECTORSIZE];
        float outputR[VAS_MAXVECTORSIZE];
        float lastReflectionInput[VAS_MAXVECTORSIZE];
        float tmp[VAS_MAXVECTORSIZE];
        
        int reflectionOrder = 1;
        int numberOfRays = 6;
        int config = 1;
        int init = 0;
        int initReverbTail = 0;
        
        vas_fir_binaural *binauralEngine;
        vas_fir_reverb *reverbEngine;
        vas_iir_biquad *directivityDamping;
        vas_fir_binauralReflection *reflections[VAS_MAXNUMBEROFRAYS][VAS_MAXREFLECTIONORDER];
        
        char debugString[512];
    };
    
    extern "C"
    {
        void *currentInstance[100];
        int instanceCounter = 0;
        extern vas_fir_list IRs;
    
        /*union {
            struct
            {
                float        _11, _12, _13, _14;
                float        _21, _22, _23, _24;
                float        _31, _32, _33, _34;
                float        _41, _42, _43, _44;
            };
            float m[4][4];
            float m2[16];
        };

        void GetRotation(float& Yaw, float& Pitch, float& Roll) const
        {
            if (_11 == 1.0f)
            {
                Yaw = atan2f(_13, _34);
                Pitch = 0;
                Roll = 0;

            }else if (_11 == -1.0f)
            {
                Yaw = atan2f(_13, _34);
                Pitch = 0;
                Roll = 0;
            }else
            {

                Yaw = atan2(-_31,_11);
                Pitch = asin(_21);
                Roll = atan2(-_23,_22);
            }
        }
         If matrix has size, n by m [i.e. i goes from 0 to (n-1) and j from 0 to (m-1) ], then:

         matrix[ i ][ j ] = array[ i*m + j ].
         
         */
    
        void HeadingAndElevationFromTranformationMatrix(float *matrix, float *heading, float *elevation)
        {
            if (matrix[0] == 1.0f)
            {
                *heading = vas_utilities_radians2degrees(atan2f(matrix[2], matrix[11]));
                *elevation = vas_utilities_radians2degrees(atan2f(-matrix[6],matrix[5]+0.001f));
            }
            else if (matrix[0]  == -1.0f)
            {
                *heading = vas_utilities_radians2degrees(atan2f(matrix[2], matrix[11]));
                *elevation = vas_utilities_radians2degrees(atan2f(-matrix[6],matrix[5]+0.001f));
            }
            else
            {
                float radians = atan2(matrix[8], matrix[0]);
                radians += M_PI;
                    
                *heading = vas_utilities_radians2degrees(radians);
                *elevation = vas_utilities_radians2degrees(atan2f(-matrix[6],matrix[5]+0.001f));
            }
        }
        
		VAS_EXPORT void SetDebugFunction( FuncPtr fp )
        {
            Debug = fp;
        }
        
		VAS_EXPORT void *GetInstance(int id)
        {
#ifdef VAS_DEBUG_TO_UNITY
            if(Debug != NULL)
                Debug("Trying to find Renderer");
#endif
            int i=0;
            while(i<instanceCounter)
            {
                EffectData *current = static_cast<EffectData *>(currentInstance[i]);
                if( current->p[3] == id)
                {
#ifdef VAS_DEBUG_TO_UNITY
                    if(Debug != NULL)
                        Debug("Found Renderer");
#endif
                    return current;
                    
                }
                i++;
            }
            return NULL;
        }
    
		VAS_EXPORT void SetConfig(EffectData *effectData, int config)
        {
            effectData->config = config;
        }
    
        void readIR(vas_fir *x, char *fullpath, int segmentSize, int offset)
        {
            const char *fileExtension;
            fileExtension = vas_util_getFileExtension(fullpath);
            if(!strcmp(fileExtension, "sofa"))
            {
#ifdef VAS_USE_LIBMYSOFA
                void *filter = vas_fir_readSofa_getMetaData(x, fullpath);
                if(filter)
                {
                    vas_fir_initFilter1((vas_fir *)x, segmentSize);
                    vas_fir_readSofa_getFilter(x, filter);
                   // vas_fir_setInitFlag((vas_fir *)x);
                }
#else
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug("Sofa not supported for this binary, compile again with VAS_USE_LIBMYSOFA");
#endif
#endif
            }
            if(!strcmp(fileExtension, "txt"))
            {
                vas_fir *existingFilter = vas_fir_list_find(&IRs, fullpath);
                 
                if(existingFilter != NULL)
                {
                    if(segmentSize == existingFilter->left->filter->segmentSize)
                    {
#ifdef VAS_DEBUG_TO_UNITY
                        if(Debug != NULL)
                            Debug("Use existing filter");
#endif
                        size_t size = strlen(existingFilter->description.fullPath);
                        x->description.fullPath = (char *)vas_mem_alloc(sizeof(char) * size);
                        strcpy(x->description.fullPath, existingFilter->description.fullPath);
                        vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, x->left, x->right);
                        vas_fir_setInitFlag((vas_fir *)x);
                        return;
                    }
                }
                
                else
                {
#ifdef VAS_DEBUG_TO_UNITY
                    if(Debug != NULL)
                        Debug("Read text file");
#endif
                    FILE *file = vas_fir_readText_metaData1((vas_fir *)x, fullpath);
                    vas_fir_initFilter2((vas_fir *)x, segmentSize, offset);
                    vas_fir_readText_Ir1((vas_fir *)x, file, offset);
                    vas_fir_list_addNode(&IRs, vas_fir_listNode_new(x));
                    vas_fir_setInitFlag((vas_fir *)x);
                }
            }
        }
        
		VAS_EXPORT void LoadHRTF(EffectData *effectData, char *fullpath)
        {
            vas_fir *x;
            if(effectData != NULL)
            {
                int segmentSize = effectData->p[P_SEGMENTSIZE_EARLYPART];
                x = (vas_fir *)effectData->binauralEngine;
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug(fullpath);
#endif
                readIR(x, fullpath, segmentSize, 0);
                    
                if(effectData->config == VAS_SPAT_CONFIG_AUTO)
                {
                    for(int i = 0; i < effectData->numberOfRays; i++)
                    {
                        for (int j = 0; j < effectData->reflectionOrder; j ++)
                        {
                            effectData->reflections[i][j] = vas_fir_binauralReflection_new(effectData->binauralEngine, 100000);
                            vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x, effectData->reflections[i][j]->binauralEngine->left, effectData->reflections[i][j]->binauralEngine->right);
                            vas_fir_setInitFlag((vas_fir *)effectData->reflections[i][j]->binauralEngine);
                        }
                    }
                }
                effectData->init = 1;
            }
            else
            {
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug("Could not find Instance");
#endif
            }
        }
    
		VAS_EXPORT void LoadReverbTail(EffectData *effectData, char *fullpath)
        {
            vas_fir *x;
            if(effectData != NULL)
            {
                int segmentSize = effectData->p[P_SEGMENTSIZE_LATEPART];
                int offset = segmentSize - effectData->p[P_SEGMENTSIZE_EARLYPART];
                x = (vas_fir *)effectData->reverbEngine;
                if(Debug != NULL)
                    Debug(fullpath);
                
                readIR(x, fullpath, segmentSize, offset);
                effectData->initReverbTail = 1;
            }
            else
            {
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug("Could not find Instance");
#endif
            }
        }
    }

    inline bool IsHostCompatible(UnityAudioEffectState* state)
    {
        // Somewhat convoluted error checking here because hostapiversion is only supported from SDK version 1.03 (i.e. Unity 5.2) and onwards.
        // Since we are only checking for version 0x010300 here, we can't use newer fields in the UnityAudioSpatializerData struct, such as minDistance and maxDistance.
        return
            state->structsize >= sizeof(UnityAudioEffectState) &&
            state->hostapiversion >= 0x010300;
    }

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
    {
        //char name[12];
       // char description[30];
        
        int paramNumber;
        int numparams = P_NUM;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        RegisterParameter(definition, "AudioSrc Attn", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, P_AUDIOSRCATTN, "AudioSource distance attenuation");
        RegisterParameter(definition, "Fixed Volume", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_FIXEDVOLUME, "Fixed volume amount");
        RegisterParameter(definition, "Custom Falloff", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_CUSTOMFALLOFF, "Custom volume falloff amount (logarithmic)");
        RegisterParameter(definition, "Spat Id", "", 0.0f, 100.0f, 0.0f, 1.0f, 1.0f, P_SPATID, "Spat id");
        RegisterParameter(definition, "HS Dir", "", 0.0f, 360, 80.0f, 1.0f, 1.0f, P_H_SOURCEDIRECTIVITY, "Horizontal Source Directivity");
        RegisterParameter(definition, "HF Po Range", "", 0.0f, 360, 40.0f, 1.0f, 1.0f, P_H_FULLPOWERRANGE, "Horizontal Full Power Range");
        RegisterParameter(definition, "VS Dir", "", 0.0f, 360, 0.0f, 1.0f, 1.0f, P_V_SOURCEDIRECTIVITY, "Vertical Source Directivity");
        RegisterParameter(definition, "VF Po Range", "", 0.0f, 360, 0.0f, 1.0f, 1.0f, P_V_FULLPOWERRANGE, "Vertical Full Power Range");
        RegisterParameter(definition, "Dir Damping", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_DIRECTIVITYDAMPING, "Directivity Damping");
        RegisterParameter(definition, "No of rays", "", 0.0f, 6.0f, 0.0f, 1.0f, 1.0f, P_NUMBEROFRAYS, "Number of rays");
        RegisterParameter(definition, "Ref order", "", 0.0f, 10.0f, 0.0f, 1.0f, 1.0f, P_REFLECTIONORDER, "Reflection order");
        RegisterParameter(definition, "Inverse Azi", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_INVERSEAZI, "Inverse Azimuth");
        RegisterParameter(definition, "Inverse Ele", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_INVERSEELE, "Inverse Elevation");
        RegisterParameter(definition, "Bypass", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_BYPASS, "Bypass");
        RegisterParameter(definition, "Lister Only", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_LISTENERORIENTATIONONLY, "Listener orientation only");
        RegisterParameter(definition, "SegSizeEarly", "", 0.0f, 4096.0f, 1024.0f, 1.0f, 1.0f, P_SEGMENTSIZE_EARLYPART, "Segment size early reverb");
        RegisterParameter(definition, "SegSizeLate", "", 0.0f, 4096.0f, 1024.0f, 1.0f, 1.0f, P_SEGMENTSIZE_LATEPART, "Segment size late reverb");

        for(int i = 0; i < VAS_MAXNUMBEROFRAYS; i++)
        {
            for (int j = 0; j < VAS_MAXREFLECTIONORDER; j ++)
            {
                //sprintf(name, "ray%d%dX", i, j);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER*3 +j * 3;
                RegisterParameter(definition, "ray", "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
                //sprintf(name, "ray%d%dY", i, j);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER*3 +j * 3 + 1;
                RegisterParameter(definition, "ray", "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
                //sprintf(name, "ray%d%dZ", i, j);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER*3 +j * 3 + 2;
                RegisterParameter(definition, "ray", "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, "Ray ");
            }
        }
        
        definition.flags |= UnityAudioEffectDefinitionFlags_IsSpatializer;
        return numparams;
    }

    static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK DistanceAttenuationCallback(UnityAudioEffectState* state, float distanceIn, float attenuationIn, float* attenuationOut)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        *attenuationOut = attenuationIn;
           // data->p[P_AUDIOSRCATTN] * attenuationIn +
           // data->p[P_FIXEDVOLUME] +
           // data->p[P_CUSTOMFALLOFF] * (1.0f / FastMax(1.0f, distanceIn));
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
    {
        EffectData* effectdata = new EffectData;
        memset(effectdata, 0, sizeof(EffectData));
        state->effectdata = effectdata;
        
        effectdata->binauralEngine = vas_fir_binaural_new(0);
        effectdata->reverbEngine = vas_fir_binaural_new(0);
        effectdata->numberOfRays = 0;
        effectdata->reflectionOrder = 0;
        effectdata->directivityDamping = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWPASS, 20000, 10);
        currentInstance[instanceCounter++] = effectdata;
        
        if (IsHostCompatible(state))
            ;//  state->spatializerdata->distanceattenuationcallback = DistanceAttenuationCallback;
        
        for(int i = 0;i < VAS_MAXVECTORSIZE; i++)
        {
            effectdata->input[i] = 0.0;
            effectdata->outputL[i] = 0.0;
            effectdata->outputR[i] = 0.0;
            effectdata->lastReflectionInput[i] = 0.0;
            effectdata->tmp[i] = 0.0;
        }
        
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->p);
        
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        
        if(instanceCounter == 1)
        {
#ifdef VAS_DEBUG_TO_UNITY
            if(Debug != NULL)
                Debug("Cleared filter list");
#endif
             vas_fir_list_clear(&IRs);
        }
          
        data->init = 0;
        
        vas_fir_binaural_free(data->binauralEngine);
        vas_fir_binaural_free(data->reverbEngine);
        vas_iir_biquad_free(data->directivityDamping);
 
        if(data->config == VAS_SPAT_CONFIG_AUTO)
        {
            for(int i = 0; i < data->numberOfRays; i++)
            {
                for (int j = 0; j < data->reflectionOrder; j ++)
                    vas_fir_binauralReflection_free(data->reflections[i][j]);
            }
        }
        
        instanceCounter--;
        
        delete data;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
            
        data->p[index] = value;
        
        if(index == P_SPATID)
        {
#ifdef VAS_DEBUG_TO_UNITY
            if(Debug != NULL)
                Debug("Set Spat Id");
#endif
        }
        if(index == P_NUMBEROFRAYS)
            data->numberOfRays = data->p[index];
        if(index == P_REFLECTIONORDER)
            data->reflectionOrder = data->p[index];
#ifdef VAS_DEBUG_TO_UNITY
        if(Debug)
            Debug(data->debugString);
#endif
        
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char *valuestr)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        if (value != NULL)
            *value = data->p[index];
        if (valuestr != NULL)
            valuestr[0] = 0;
         
        return UNITY_AUDIODSP_OK;
    }

    int UNITY_AUDIODSP_CALLBACK GetFloatBufferCallback(UnityAudioEffectState* state, const char* name, float* buffer, int numsamples)
    {
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(UnityAudioEffectState* state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int outchannels)
    {
        if (inchannels != 2 || outchannels != 2 || state->spatializerdata == NULL)
        {
            memset(outbuffer, 0, length * outchannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }
        
        EffectData* data = state->GetEffectData<EffectData>();
        if(!data->init || data->p[P_BYPASS])
        {
            memset(outbuffer, 0, length * outchannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }

        static const float kRad2Deg = 180.0f / kPI;
        float azimuth, elevation;

        float* listener = state->spatializerdata->listenermatrix;
        float* source = state->spatializerdata->sourcematrix;
        float px, py, pz;
        float dir_x, dir_y, dir_z;
     
        if(data->p[P_LISTENERORIENTATIONONLY])
           HeadingAndElevationFromTranformationMatrix(listener, &azimuth, &elevation);
        
        else
        {
            px = source[12];
            py = source[13];
            pz = source[14];
            
            dir_x = listener[0] * px + listener[4] * py + listener[8] * pz + listener[12];
            dir_y = listener[1] * px + listener[5] * py + listener[9] * pz + listener[13];
            dir_z = listener[2] * px + listener[6] * py + listener[10] * pz + listener[14];
            
            azimuth = (fabsf(dir_z) < 0.001f) ? 0.0f : atan2f(dir_x, dir_z);
            if (azimuth < 0.0f)
                azimuth += 2.0f * kPI;
            azimuth = FastClip(azimuth * kRad2Deg, 0.0f, 360.0f);
            elevation = atan2f(dir_y, sqrtf(dir_x * dir_x + dir_z * dir_z) + 0.001f) * kRad2Deg;
        }
  
        
        //float spatialblend = state->spatializerdata->spatialblend;
        //float reverbmix = state->spatializerdata->reverbzonemix;
    
        //float occlusionLowPassFreq = data->p[P_OCCLUSION_FREQ];

        // From the FMOD documentation:
        //   A spread angle of 0 makes the stereo sound mono at the point of the 3D emitter.
        //   A spread angle of 90 makes the left part of the stereo sound place itself at 45 degrees to the left and the right part 45 degrees to the right.
        //   A spread angle of 180 makes the left part of the stero sound place itself at 90 degrees to the left and the right part 90 degrees to the right.
        //   A spread angle of 360 makes the stereo sound mono at the opposite speaker location to where the 3D emitter should be located (by moving the left part 180 degrees left and the right part 180 degrees right). So in this case, behind you when the sound should be in front of you!
        // Note that FMOD performs the spreading and panning in one go. We can't do this here due to the way that impulse-based spatialization works, so we perform the spread calculations on the left/right source signals before they enter the convolution processing.
        // That way we can still use it to control how the source signal downmixing takes place.
       // float spread = cosf(state->spatializerdata->spread * kPI / 360.0f);
       // float spreadmatrix[2] = { 2.0f - spread, spread };
        
        for(unsigned int n = 0; n < length; n++)
        {
            data->input[n] = inbuffer[n * inchannels];
            data->lastReflectionInput[n] = inbuffer[n * inchannels];
        }
     
        if(data->p[P_INVERSEAZI])
            azimuth = 360 - azimuth;
        
        vas_fir_binaural_setAzimuth(data->binauralEngine, azimuth);
        vas_fir_binaural_setElevation(data->binauralEngine, elevation);
        
        float scale = data->p[P_DIRECTIVITYDAMPING];
        
        if(data->config == VAS_SPAT_CONFIG_AUTO)
        {
            vas_iir_biquad_setFrequency(data->directivityDamping, scale * 20000);
            vas_iir_biquad_process(data->directivityDamping, data->input, data->input, length);
        }
        
        vas_fir_binaural_process(data->binauralEngine, data->input, data->outputL, data->outputR, length);
        if(data->initReverbTail)
            vas_fir_binaural_processOutputInPlace(data->reverbEngine, data->lastReflectionInput, data->outputL, data->outputR, length);
        
        if(data->config == VAS_SPAT_CONFIG_AUTO)
        {
            float r_px;
            float r_py;
            float r_pz;
            float distance;
            
            for(int i = 0; i < data->numberOfRays; i++)
            {
                for(int n = 0; n < length; n++)
                {
                    data->lastReflectionInput[n] = inbuffer[n * inchannels];
                }
                
                for (int j = 0; j < data->reflectionOrder; j ++)
                {
                    int paramIndex = reflectionOffset + VAS_MAXNUMBEROFRAYS * i*3 + j*3;
                    
                    r_px = data->p[paramIndex];
                    r_py = data->p[paramIndex+1];
                    r_pz = data->p[paramIndex+2];
                     
                    distance = sqrt(pow(r_px-px, 2) + pow(r_py-py, 2) + pow(r_pz-pz, 2));
                    float delayTime = distance/343.0 * 44100.0;
                    
                    dir_x = listener[0] * r_px + listener[4] * r_py + listener[8] * r_pz + listener[12];
                    dir_y = listener[1] * r_px + listener[5] * r_py + listener[9] * r_pz + listener[13];
                    dir_z = listener[2] * r_px + listener[6] * r_py + listener[10] * r_pz + listener[14];
                     
                    azimuth = (fabsf(dir_z) < 0.001f) ? 0.0f : atan2f(dir_x, dir_z);
                    if (azimuth < 0.0f)
                        azimuth += 2.0f * kPI;
                    azimuth = FastClip(azimuth * kRad2Deg, 0.0f, 360.0f);
                    if(data->p[P_INVERSEAZI])
                        azimuth = 360 - azimuth;
                     
                    elevation = atan2f(dir_y, sqrtf(dir_x * dir_x + dir_z * dir_z) + 0.001f) * kRad2Deg;
                     
                    vas_fir_binauralReflection_setDelayTime(data->reflections[i][j], delayTime);
                    vas_fir_binauralReflection_setAzimuth(data->reflections[i][j], azimuth);
                    vas_fir_binauralReflection_setElevation(data->reflections[i][j], elevation);
                    vas_fir_binauralReflection_process(data->reflections[i][j], data->lastReflectionInput, data->outputL, data->outputR, length);
                }
            }
        }
        
        for(unsigned int n = 0; n < length; n++)
        {
            outbuffer[n * outchannels] = data->outputL[n];
            outbuffer[n * outchannels + 1] = data->outputR[n];
        }
 
        return UNITY_AUDIODSP_OK;
    }
}




