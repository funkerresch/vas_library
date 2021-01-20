/**
* @file VasUnitySpatializer.h
* @author Thomas Resch
* @date 10 Sep 2018
* @brief C - Binaural filter plugin <br>
*
* The plugin calculates dynamic binaural synthesis based on<br>
 the source-listener rotation matrix.<br>
 For BRIR-based calculations it is possible to use<br>
 listener orientation only. In VAS_SPAT_CONFIG_AUTO-mode an experimental<br>
 raytracing-based synthesis is calculated.<br>
 <br>
 In order to save resources it is possible to load <br>
 a static reverb tail with a larger vector size. The tail IR must
 contain "BRIR.lengh"-zeros in the beginning.<br>
*
*/

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

#define VAS_SPAT_MAXNUMBEROFRAYS 100
#define VAS_SPAT_MAXREFLECTIONORDER 10
#define VAS_DEBUG_TO_UNITY

#define VAS_SPAT_CONFIG_SIMPLE 1
#define VAS_SPAT_CONFIG_MANUAL 2
#define VAS_SPAT_CONFIG_AUTO 3

using namespace std;

namespace Spatializer
{
    enum
    {
        P_REF_X,
        P_REF_Y,
        P_REF_Z,
        P_REF_MAT,
        P_REF_MUTE,
        P_REF_SCALE,
        P_REF_DIST,
        P_REF_DUMMY1,
        P_REF_DUMMY2,
        P_REF_DUMMY3,
        P_REF_SIZE
    };

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
        P_H_DIRECTIVITYDAMPING,
        P_V_DIRECTIVITYDAMPING,
        P_NUMBEROFRAYS,
        P_REFLECTIONORDER,
        P_INVERSEAZI,
        P_INVERSEELE,
        P_BYPASS,
        P_LISTENERORIENTATIONONLY,
        P_SEGMENTSIZE_EARLYPART,
        P_SEGMENTSIZE_LATEPART,
        P_DISTANCE_SCALING,
        P_OCCLUSION,
        P_SCALING_EARLY,
        P_SCALING_LATE,
        
        P_NUM
    };

    struct EffectData
    {
        float p[P_NUM]; // Parameters
        float input[VAS_MAXVECTORSIZE];
        float outputL[VAS_MAXVECTORSIZE];
        float outputR[VAS_MAXVECTORSIZE];
        float lastReflectionInput[VAS_MAXVECTORSIZE];
        float tmp[VAS_MAXVECTORSIZE];
        float reflectionParameters[10000]; // for 100 Rays with max Reflectionorder 10 a 10 parameters
        
        int reflectionOrder = 1;
        int numberOfRays = 6;
        int config = 1;
        int init = 0;
        int initReverbTail = 0;
        
        vas_fir_binaural *binauralEngine;
        vas_fir_reverb *reverbEngine;
        vas_iir_biquad *directivityDamping;
        vas_fir_binauralReflection *reflections[VAS_SPAT_MAXNUMBEROFRAYS][VAS_SPAT_MAXREFLECTIONORDER];
        
        char debugString[512];
    };
    
    extern "C"
    {
        static void *currentInstance[100];
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
            while(i<100)
            {
                if(currentInstance[i])
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
                }
                i++;
            }
            return NULL;
        }
    
		VAS_EXPORT void SetConfig(EffectData *effectData, int config)
        {
            effectData->config = config;
        }
    
        VAS_EXPORT bool EarlyPartIsLoaded(EffectData *effectData)
        {

            if(effectData->init)
            {
#ifdef VAS_DEBUG_TO_UNITY
                        if(Debug != NULL)
                            Debug("LOADED");
#endif
                return true;
            }
            
            else
            {
#ifdef VAS_DEBUG_TO_UNITY
                        if(Debug != NULL)
                            Debug("NOT LOADED");
#endif
                return false;
            }
        }
    
        VAS_EXPORT bool LatePartIsLoaded(EffectData *effectData)
        {
            if(effectData->initReverbTail)
                return true;
            else
                return false;
        }
    
        void readIR(vas_fir *x, char *fullpath, int segmentSize, int offset)
        {
            const char *fileExtension;
            int end = 0;
            fileExtension = vas_util_getFileExtension(fullpath);
            if(!strcmp(fileExtension, "sofa"))
            {
#ifdef VAS_USE_LIBMYSOFA
                void *filter = vas_fir_readSofa_getMetaData(x, fullpath);
                if(filter)
                {
                    vas_fir_initFilter1((vas_fir *)x, segmentSize);
                    vas_fir_readSofa_getFilter(x, filter);
                    vas_fir_setInitFlag((vas_fir *)x);
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
                //vas_fir *existingFilter = vas_fir_list_find1(&IRs, fullpath, segmentSize, offset, end);
                 
                if(existingFilter != NULL)
                {
#ifdef VAS_DEBUG_TO_UNITY
                    if(Debug != NULL)
                    {
                        Debug("Use existing filter");
                        Debug(existingFilter->metaData.fullPath);
                        
                        char tmp[64];
                        sprintf(tmp, "search for: %d %d %d", segmentSize, offset, end);
                        Debug(tmp);
                        
                        sprintf(tmp, "existing is: %d %d %d", existingFilter->metaData.segmentSize, existingFilter->metaData.filterOffset, existingFilter->metaData.filterEnd);
                        Debug(tmp);
                    }
#endif
                    size_t size = strlen(existingFilter->metaData.fullPath);
                    x->metaData.fullPath = (char *)vas_mem_alloc(sizeof(char) * size);
                    strcpy(x->metaData.fullPath, existingFilter->metaData.fullPath);
                    vas_fir_prepareChannelsWithSharedFilter((vas_fir *)existingFilter, x->left, x->right);
                    vas_fir_setInitFlag((vas_fir *)x);
                    return;
                }
                
                else
                {
#ifdef VAS_DEBUG_TO_UNITY
                    if(Debug != NULL)
                        Debug("Read text file");
#endif
                    FILE *file = vas_fir_readText_metaData1((vas_fir *)x, fullpath);
                    if(file)
                    {
                        ((vas_fir *)x)->metaData.filterOffset = offset;
                        ((vas_fir *)x)->metaData.segmentSize = segmentSize;
                        if(end > 0 && (end < ((vas_fir *)x)->metaData.filterLength) )
                            ((vas_fir *)x)->metaData.filterLength = end;
                   
                        vas_fir_initFilter2((vas_fir *)x, segmentSize);
                        vas_fir_readText_Ir1((vas_fir *)x, file, offset);
                        vas_fir_list_addNode(&IRs, vas_fir_listNode_new(x));
                        vas_fir_setInitFlag((vas_fir *)x);
                    }
#ifdef VAS_DEBUG_TO_UNITY
                    else
                    {
                        if(Debug != NULL)
                            Debug("Error: Could not read file");
                    }
#endif
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
                    
                if(effectData->config != VAS_SPAT_CONFIG_SIMPLE)
                {
                    for(int i = 0; i < effectData->numberOfRays; i++)
                    {
                        for (int j = 0; j < effectData->reflectionOrder; j ++)
                        {
                            effectData->reflections[i][j] = vas_fir_binauralReflection_new(effectData->binauralEngine, 30000);
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
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                    Debug(fullpath);
#endif
                
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
    
        VAS_EXPORT void SetReflectionParameter(EffectData *effectData, int rayNumber, int reflectionNumber, int reflectionParameter, float value)
        {
            int paramIndex = rayNumber * VAS_SPAT_MAXREFLECTIONORDER * P_REF_SIZE + reflectionNumber * P_REF_SIZE + reflectionParameter;
            effectData->reflectionParameters[paramIndex] = value;
#ifdef VAS_DEBUG_TO_UNITY
                if(Debug != NULL)
                {
                    sprintf(effectData->debugString, "value: %f", value);
                    Debug(effectData->debugString);
                }
#endif
        }
    
        VAS_EXPORT float GetReflectionParameter(EffectData *effectData, int rayNumber, int reflectionNumber, int reflectionParameter)
        {
            int paramIndex = rayNumber * VAS_SPAT_MAXREFLECTIONORDER * P_REF_SIZE + reflectionNumber * P_REF_SIZE + reflectionParameter;
            return effectData->reflectionParameters[paramIndex];
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
        RegisterParameter(definition, "Spat Id", "", -1.0f, 100.0f, -1.0f, 1.0f, 1.0f, P_SPATID, "Spat id");
        RegisterParameter(definition, "HS Dir", "", 0.0f, 360, 80.0f, 1.0f, 1.0f, P_H_SOURCEDIRECTIVITY, "Horizontal Source Directivity");
        RegisterParameter(definition, "HF Po Range", "", 0.0f, 360, 40.0f, 1.0f, 1.0f, P_H_FULLPOWERRANGE, "Horizontal Full Power Range");
        RegisterParameter(definition, "VS Dir", "", 0.0f, 360, 0.0f, 1.0f, 1.0f, P_V_SOURCEDIRECTIVITY, "Vertical Source Directivity");
        RegisterParameter(definition, "VF Po Range", "", 0.0f, 360, 0.0f, 1.0f, 1.0f, P_V_FULLPOWERRANGE, "Vertical Full Power Range");
        RegisterParameter(definition, "HDir Damping", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_H_DIRECTIVITYDAMPING, "Horizontal Directivity Damping");
        RegisterParameter(definition, "VDir Damping", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_V_DIRECTIVITYDAMPING, "Vertical Directivity Damping");
        RegisterParameter(definition, "No of rays", "", 0.0f, 100.0f, 0.0f, 1.0f, 1.0f, P_NUMBEROFRAYS, "Number of rays");
        RegisterParameter(definition, "Ref order", "", 0.0f, 10.0f, 0.0f, 1.0f, 1.0f, P_REFLECTIONORDER, "Reflection order");
        RegisterParameter(definition, "Inverse Azi", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_INVERSEAZI, "Inverse Azimuth");
        RegisterParameter(definition, "Inverse Ele", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_INVERSEELE, "Inverse Elevation");
        RegisterParameter(definition, "Bypass", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_BYPASS, "Bypass");
        RegisterParameter(definition, "Lister Only", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_LISTENERORIENTATIONONLY, "Listener orientation only");
        RegisterParameter(definition, "SegSizeEarly", "", 0.0f, 4096.0f, 1024.0f, 1.0f, 1.0f, P_SEGMENTSIZE_EARLYPART, "Segment size early reverb");
        RegisterParameter(definition, "SegSizeLate", "", 0.0f, 4096.0f, 1024.0f, 1.0f, 1.0f, P_SEGMENTSIZE_LATEPART, "Segment size late reverb");
        RegisterParameter(definition, "Dist Scaling", "", 0.0f, 10000.0f, 1.0f, 1.0f, 1.0f, P_DISTANCE_SCALING, "Scale Roomsize");
        RegisterParameter(definition, "Occlusion", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_OCCLUSION, "Occlusion");
        RegisterParameter(definition, "Scaling Early", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, P_SCALING_EARLY, "Scaling early part");
        RegisterParameter(definition, "Scaling Late", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, P_SCALING_LATE, "Scaling late part");
        
        definition.flags |= UnityAudioEffectDefinitionFlags_IsSpatializer;
        return numparams;
    }

    static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK DistanceAttenuationCallback(UnityAudioEffectState* state, float distanceIn, float attenuationIn, float* attenuationOut)
    {
        EffectData* data = state->GetEffectData<EffectData>();
        //*attenuationOut = attenuationIn;
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
        
        //memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
        //return UNITY_AUDIODSP_OK;

        static const float kRad2Deg = 180.0f / kPI;
        float azimuth, elevation;

        float* listener = state->spatializerdata->listenermatrix;
        float* source = state->spatializerdata->sourcematrix;
        float px, py, pz;
        float dir_x, dir_y, dir_z;
     
        if(data->p[P_LISTENERORIENTATIONONLY])
        {
           HeadingAndElevationFromTranformationMatrix(listener, &azimuth, &elevation);
//#ifdef VAS_DEBUG_TO_UNITY
//            if(Debug)
//            {
//                sprintf(data->debugString, "Azimuth: %f Elevation :%f", azimuth, elevation);
//                Debug(data->debugString);
//            }
//#endif
        }
        
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
        
        for(unsigned int n = 0; n < length; n++)
        {
            data->input[n] = inbuffer[n * inchannels]; // last reflectionInput deleted
            data->lastReflectionInput[n] = inbuffer[n * inchannels]; // last reflectionInput deleted
        }
     
        if(data->p[P_INVERSEAZI])
            azimuth = 360 - azimuth;
        
        vas_fir_binaural_setAzimuth(data->binauralEngine, azimuth);
        vas_fir_binaural_setElevation(data->binauralEngine, elevation);
        
        float r_hDirectivityDamping = data->p[P_H_DIRECTIVITYDAMPING];
        float r_vDirectivityDamping = data->p[P_V_DIRECTIVITYDAMPING];
        
        if(data->config != VAS_SPAT_CONFIG_SIMPLE)
        {
            vas_iir_biquad_setFrequency(data->directivityDamping, r_hDirectivityDamping * r_vDirectivityDamping * 20000);
            vas_iir_biquad_process(data->directivityDamping, data->input, data->input, length);
        }
        
        vas_util_fscale(data->input, data->p[P_SCALING_EARLY], length);
        vas_fir_binaural_process(data->binauralEngine, data->input, data->outputL, data->outputR, length);

        if(data->initReverbTail)
        {
            vas_util_fscale(data->lastReflectionInput, data->p[P_SCALING_LATE], length);
            vas_fir_binaural_processOutputInPlace(data->reverbEngine, data->lastReflectionInput, data->outputL, data->outputR, length); // input was lastReflectionInput
        }
        
        if(data->config != VAS_SPAT_CONFIG_SIMPLE)
        {
            float r_px;
            float r_py;
            float r_pz;
            float r_scaling;
            float r_distance;
            int material;
            int mute;
            int paramIndex;
            
            for(int i = 0; i < data->numberOfRays; i++)
            {
                for(int n = 0; n < length; n++)
                    data->lastReflectionInput[n] = inbuffer[n * inchannels];
                
                vas_util_fscale(data->lastReflectionInput, data->p[P_SCALING_EARLY], length);
                
                for (int j = 0; j < data->reflectionOrder; j ++)
                {
                    paramIndex = i * VAS_SPAT_MAXREFLECTIONORDER * P_REF_SIZE + j * P_REF_SIZE;
                    
                    r_px = data->reflectionParameters[paramIndex + P_REF_X];
                    r_py = data->reflectionParameters[paramIndex + P_REF_Y];
                    r_pz = data->reflectionParameters[paramIndex + P_REF_Z];
                    material = (int)data->reflectionParameters[paramIndex + P_REF_MAT];
                    mute =  data->reflectionParameters[paramIndex + P_REF_MUTE];
                    r_scaling = data->reflectionParameters[paramIndex + P_REF_SCALE];;
                    r_distance = data->reflectionParameters[paramIndex + P_REF_DIST];
                     
                    //distance = sqrt(pow(r_px-px, 2) + pow(r_py-py, 2) + pow(r_pz-pz, 2)); // bullshit... need to set distance as parameter
                    float delayTime = r_distance/343.0 * 44100.0;
                      
                    vas_fir_binauralReflection_setScaling(data->reflections[i][j], r_scaling);
                    vas_fir_binauralReflection_setMaterial(data->reflections[i][j], material); // for a very simplyfied material characteristics
                    vas_fir_binauralReflection_setDelayTime(data->reflections[i][j], delayTime); // these should be called from game loop, not within the dsp loop
                    
//                    if(Debug)
//                    {
//                        sprintf(data->debugString, "%d %d %f",i, j, r_distance);
//                        Debug(data->debugString);
//                    }
                     
                    if(!mute)
                    {
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
                        
                        vas_fir_binauralReflection_setAzimuth(data->reflections[i][j], azimuth);
                        vas_fir_binauralReflection_setElevation(data->reflections[i][j], elevation);
                        vas_fir_binauralReflection_process(data->reflections[i][j], data->lastReflectionInput, data->outputL, data->outputR, length);
                    }
                    else
                    {
                        vas_fir_binauralReflection_process_mute(data->reflections[i][j], data->lastReflectionInput, length);
                    }
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




