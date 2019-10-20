#include "AudioPluginUtil.h"
#include "vas_fir_binaural.h"
#include "vas_delay_crossfade.h"
#include "vas_iir_biquad.h"
#include "vas_delay_crossfade.h"
#include "vas_fir_binauralReflection.h"
#include <list>

#ifndef DEBUG_UNITY
#define DEBUG_UNITY
typedef void (*FuncPtr)( const char * );
FuncPtr Debug = NULL;
#endif

#define VAS_MAXNUMBEROFRAYS 6
#define VAS_MAXREFLECTIONORDER 6

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
        
        P_NUM
    };
    
    static int reflectionOffset = P_REF_1_1_X;

    struct EffectData
    {
        float p[P_NUM]; // Parameters
        float input[4096];
        float delayTmp[4096];
        float tmp[4096];
        float outputL[4096];
        float outputR[4096];
        float lastReflectionInput[4096];
        float delayOutput[4096];
        
        int reflectionOrder = 1;
        int numberOfRays = 6;
        int config = 1;
        int init = 0;
        int initReverbTail = 0;
        vas_fir_binaural *binauralEngine;
        vas_fir_reverb *reverbEngine;
        vas_iir_biquad *directivityDamping;
        vas_fir_binauralReflection *reflections[VAS_MAXNUMBEROFRAYS][VAS_MAXREFLECTIONORDER];
    };
    
    extern "C"
    {
        static void *currentInstance[100];
        static int instanceCounter = 0;
        
        VAS_EXPORT void SetDebugFunction( FuncPtr fp )
        {
            Debug = fp;
        }
        
        VAS_EXPORT void *GetInstance(int id)
        {
            if(Debug != NULL)
                Debug("Trying to find Renderer");
            int i=0;
            while(i<instanceCounter)
            {
                EffectData *current = static_cast<EffectData *>(currentInstance[i]);
                if( current->p[3] == id)
                {
                    if(Debug != NULL)
                        Debug("Found Renderer");
                    return current;
                    
                }
                i++;
            }
            return NULL;
        }
        
        VAS_EXPORT void SetNumberOfRays(EffectData *effectData, int numberOfRays)
        {
            effectData->numberOfRays = numberOfRays;
        }
        
        VAS_EXPORT void SetReflectionOrder(EffectData *effectData, int reflectionOrder)
        {
            effectData->reflectionOrder = reflectionOrder;
        }
    
        VAS_EXPORT void SetConfig(EffectData *effectData, int config)
        {
            effectData->config = config;
        }
        
        VAS_EXPORT void LoadHRTF(EffectData *effectData, char *fullpath)
        {
            vas_fir *x;
            if(effectData != NULL)
            {
                x = (vas_fir *)effectData->binauralEngine;
                if(Debug != NULL)
                    Debug(fullpath);
                
                const char *fileExtension;
                fileExtension = vas_util_getFileExtension(fullpath);
                if(!strcmp(fileExtension, "sofa"))
                {
#ifdef VAS_USE_LIBMYSOFA
                    void *filter = vas_fir_readSofa_getMetaData(x, fullpath);
                    if(filter)
                    {
                        vas_fir_initFilter1((vas_fir *)x, 1024);
                        vas_fir_readSofa_getFilter(x, filter);
                       // vas_fir_setInitFlag((vas_fir *)x);
                    }
#else
                    if(Debug != NULL)
                        Debug("Sofa not supported for this binary, compile again with VAS_USE_LIBMYSOFA");
#endif
                }
                if(!strcmp(fileExtension, "txt"))
                {
                    if(Debug != NULL)
                        Debug("Read Text");
                    FILE *file = vas_fir_readText_metaData1((vas_fir *)x, fullpath);
                    
                    if(file)
                    {
                        vas_fir_initFilter1((vas_fir *)x, 1024);
                        vas_fir_readText_Ir((vas_fir *)x, file);
                        vas_fir_setInitFlag((vas_fir *)x);
                    }
                    
                    if(effectData->config == VAS_SPAT_CONFIG_AUTO)
                    {
                        for(int i = 0; i < effectData->numberOfRays; i++)
                        {
                            for (int j = 0; j < effectData->reflectionOrder; j ++)
                            {
                                vas_fir_prepareChannelsWithSharedFilter((vas_fir *)x, effectData->reflections[i][j]->binauralEngine->left, effectData->reflections[i][j]->binauralEngine->right);
                                vas_fir_setInitFlag((vas_fir *)effectData->reflections[i][j]->binauralEngine);
                            }
                        }
                    }
                    effectData->init = 1;
                }
            }
            else
            {
                if(Debug != NULL)
                    Debug("Could not find Instance");
                
            }
        }
    
        VAS_EXPORT void LoadReverbTail(EffectData *effectData, char *fullpath)
        {
            vas_fir *x;
            if(effectData != NULL)
            {
                x = (vas_fir *)effectData->reverbEngine;
                if(Debug != NULL)
                    Debug(fullpath);
                
                const char *fileExtension;
                fileExtension = vas_util_getFileExtension(fullpath);
                if(!strcmp(fileExtension, "sofa"))
                {
#ifdef VAS_USE_LIBMYSOFA
                    void *filter = vas_fir_readSofa_getMetaData(x, fullpath);
                    if(filter)
                    {
                        vas_fir_initFilter1((vas_fir *)x, 1024);
                        vas_fir_readSofa_getFilter(x, filter);
                       // vas_fir_setInitFlag((vas_fir *)x);
                    }
#else
                    if(Debug != NULL)
                        Debug("Sofa not supported for this binary, compile again with VAS_USE_LIBMYSOFA");
#endif
                }
                if(!strcmp(fileExtension, "txt"))
                {
                    if(Debug != NULL)
                        Debug("Read Text");
                    FILE *file = vas_fir_readText_metaData1((vas_fir *)x, fullpath);
                    
                    if(file)
                    {
                        vas_fir_initFilter1((vas_fir *)x, 1024);
                        vas_fir_readText_Ir((vas_fir *)x, file);
                        vas_fir_setInitFlag((vas_fir *)x);
                    }
                }
                effectData->initReverbTail = 1;
            }
            else
            {
                if(Debug != NULL)
                    Debug("Could not find Instance");
                
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
        char name[30];
        char description[30];
        int paramNumber;
        int numparams = P_NUM;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        RegisterParameter(definition, "AudioSrc Attn", "", 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, P_AUDIOSRCATTN, "AudioSource distance attenuation");
        RegisterParameter(definition, "Fixed Volume", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_FIXEDVOLUME, "Fixed volume amount");
        RegisterParameter(definition, "Custom Falloff", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_CUSTOMFALLOFF, "Custom volume falloff amount (logarithmic)");
        RegisterParameter(definition, "Spat Id", "", 0.0f, 100.0f, 0.0f, 1.0f, 1.0f, P_SPATID, "Spat id");
        RegisterParameter(definition, "H Source Directivity", "", 0.0f, 360, 80.0f, 1.0f, 1.0f, P_H_SOURCEDIRECTIVITY, "Horizontal Source Directivity");
        RegisterParameter(definition, "H Full Power Range", "", 0.0f, 360, 40.0f, 1.0f, 1.0f, P_H_FULLPOWERRANGE, "Horizontal Full Power Range");
        RegisterParameter(definition, "V Source Directivity", "", 0.0f, 360, 0.0f, 1.0f, 1.0f, P_V_SOURCEDIRECTIVITY, "Vertical Source Directivity");
        RegisterParameter(definition, "V Full Power Range", "", 0.0f, 360, 0.0f, 1.0f, 1.0f, P_V_FULLPOWERRANGE, "Vertical Full Power Range");
        RegisterParameter(definition, "Directivity Damping", "", 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, P_DIRECTIVITYDAMPING, "Directivity Dampint");
        
        for(int i = 0; i < VAS_MAXNUMBEROFRAYS; i++)
        {
            for (int j = 0; j < VAS_MAXREFLECTIONORDER; j ++)
            {
                printf(name, "Ray %d Order %d X", i+1, j+1);
                printf(description, "X position of ray %d, order %d", i+1, j+1);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER*3 +j * 3;
                RegisterParameter(definition, name, "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, description);
                
                printf(name, "Ray %d Order %d Y", i+1, j+1);
                printf(description, "Y position of ray %d, order %d", i+1, j+1);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER*3 +j * 3 + 1;
                RegisterParameter(definition, name, "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, description);
                
                printf(name, "Ray %d Order %d Z", i+1, j+1);
                printf(description, "Z position of ray %d, order %d", i+1, j+1);
                paramNumber = reflectionOffset + i*VAS_MAXREFLECTIONORDER*3 +j * 3 + 2;
                RegisterParameter(definition, name, "", -10000.0f, 10000.0f, 0.0f, 1.0f, 1.0f, paramNumber, description);
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
        
        effectdata->binauralEngine = vas_fir_binaural_new(VAS_VDSP | VAS_BINAURALSETUP_NOELEVATION | VAS_LOCALFILTER, 1024, NULL);
        effectdata->reverbEngine = vas_fir_binaural_new(VAS_VDSP | VAS_BINAURALSETUP_NOELEVATION | VAS_LOCALFILTER, 1024, NULL);
        effectdata->numberOfRays = 6;
        effectdata->reflectionOrder = 4;
        //effectdata->directivityDamping = vas_iir_biquad_new(VAS_IIR_BIQUAD_LOWPASS, 20000, 10);
        currentInstance[instanceCounter++] = effectdata;
    
        state->effectdata = effectdata;
        if (IsHostCompatible(state))
            ;//  state->spatializerdata->distanceattenuationcallback = DistanceAttenuationCallback;
        
        for(int i = 0;i < 4096; i++)
        {
            effectdata->input[i] = 0.0;
            effectdata->outputL[i] = 0.0;
            effectdata->outputR[i] = 0.0;
            effectdata->delayTmp[i] = 0.0;
            effectdata->tmp[i] = 0.0;
        }
        
        if(effectdata->config == VAS_SPAT_CONFIG_AUTO)
        {
            for(int i = 0; i < effectdata->numberOfRays; i++)
            {
                for (int j = 0; j < effectdata->reflectionOrder; j ++)
                {
                    effectdata->reflections[i][j] = vas_fir_binauralReflection_new(effectdata->binauralEngine, 60000);
                }
            }
        }
        
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->p);
        
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        data->init = 0;
        vas_fir_binaural_free(data->binauralEngine);
        vas_fir_binaural_free(data->reverbEngine);
        
        if(data->config == VAS_SPAT_CONFIG_AUTO)
        {
            for(int i = 0; i < data->numberOfRays; i++)
            {
                for (int j = 0; j < data->reflectionOrder; j ++)
                {
                    vas_fir_binauralReflection_free(data->reflections[i][j]);
                }
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
        
        if(index == 3)
        {
            if(Debug != NULL)
                Debug("Set Spat Id");
        }
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
        if (inchannels != 2 || outchannels != 2 ||
            !IsHostCompatible(state) || state->spatializerdata == NULL)
        {
            memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }
        
        EffectData* data = state->GetEffectData<EffectData>();
        if(!data->init)
        {
            memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }

        static const float kRad2Deg = 180.0f / kPI;

        float* m = state->spatializerdata->listenermatrix;
        float* s = state->spatializerdata->sourcematrix;

        // Currently we ignore source orientation and only use the position
        float px = s[12];
        float py = s[13];
        float pz = s[14];
        
        float listenerpos_x = -(m[0] * m[12] + m[ 1] * m[13] + m[ 2] * m[14]);
        float listenerpos_y = -(m[4] * m[12] + m[ 5] * m[13] + m[ 6] * m[14]);
        float listenerpos_z = -(m[8] * m[12] + m[ 9] * m[13] + m[10] * m[14]);
        
        float distance = 0;
        float r_px;
        float r_py;
        float r_pz;
     
        float dir_x = m[0] * px + m[4] * py + m[8] * pz + m[12];
        float dir_y = m[1] * px + m[5] * py + m[9] * pz + m[13];
        float dir_z = m[2] * px + m[6] * py + m[10] * pz + m[14];
        
        float dir_s2l_x = s[0] * listenerpos_x + s[4] * listenerpos_y + s[8] * listenerpos_z + s[12];
        float dir_s2l_y = s[1] * listenerpos_x + s[5] * listenerpos_y + s[9] * listenerpos_z + s[13];
        float dir_s2l_z = s[2] * listenerpos_x + s[6] * listenerpos_y + s[10] * listenerpos_z + s[14];

        float azimuth = (fabsf(dir_z) < 0.001f) ? 0.0f : atan2f(dir_x, dir_z);
        if (azimuth < 0.0f)
            azimuth += 2.0f * kPI;
        azimuth = FastClip(azimuth * kRad2Deg, 0.0f, 360.0f);

        float elevation = atan2f(dir_y, sqrtf(dir_x * dir_x + dir_z * dir_z) + 0.001f) * kRad2Deg;
        
        float s2l_azimuth = (fabsf(dir_s2l_z) < 0.001f) ? 0.0f : atan2f(dir_s2l_x, dir_s2l_z);
        if (s2l_azimuth <  0.0f)
            s2l_azimuth += 2.0f * kPI;
        s2l_azimuth = FastClip(s2l_azimuth * kRad2Deg, 0.0f, 360.0f);
        
        float horizontalDirectivityOver2 = data->p[P_H_SOURCEDIRECTIVITY] * 0.5;
        float horizontalFullEnergyOver2 = data->p[P_H_FULLPOWERRANGE] * 0.5;
        float left = 360 - horizontalDirectivityOver2;
        float right = horizontalDirectivityOver2;
        float leftFull = 360 - horizontalFullEnergyOver2;
        float rightFull = horizontalFullEnergyOver2;
        float diff = leftFull-left;
        
        float s2l_elevation = atan2f(dir_s2l_y, sqrtf(dir_s2l_x * dir_s2l_x + dir_s2l_z * dir_s2l_z) + 0.001f) * kRad2Deg;
        
        float spatialblend = state->spatializerdata->spatialblend;
        float reverbmix = state->spatializerdata->reverbzonemix;
        //float occlusionLowPassFreq = data->p[P_OCCLUSION_FREQ];

        // From the FMOD documentation:
        //   A spread angle of 0 makes the stereo sound mono at the point of the 3D emitter.
        //   A spread angle of 90 makes the left part of the stereo sound place itself at 45 degrees to the left and the right part 45 degrees to the right.
        //   A spread angle of 180 makes the left part of the stero sound place itself at 90 degrees to the left and the right part 90 degrees to the right.
        //   A spread angle of 360 makes the stereo sound mono at the opposite speaker location to where the 3D emitter should be located (by moving the left part 180 degrees left and the right part 180 degrees right). So in this case, behind you when the sound should be in front of you!
        // Note that FMOD performs the spreading and panning in one go. We can't do this here due to the way that impulse-based spatialization works, so we perform the spread calculations on the left/right source signals before they enter the convolution processing.
        // That way we can still use it to control how the source signal downmixing takes place.
        float spread = cosf(state->spatializerdata->spread * kPI / 360.0f);
        float spreadmatrix[2] = { 2.0f - spread, spread };
        
        for(unsigned int n = 0; n < length; n++)
        {
            data->input[n] = inbuffer[n * inchannels];
            data->lastReflectionInput[n] = inbuffer[n * inchannels];
        }
        
        azimuth = 360 - azimuth;
        
        vas_fir_binaural_setAzimuth(data->binauralEngine, azimuth);
        vas_fir_binaural_setElevation(data->binauralEngine, elevation);
       
       /* float scale = 1;
        if(s2l_azimuth <= rightFull)
            ;
        else if(s2l_azimuth >= leftFull)
            ;
        
        else if(s2l_azimuth > rightFull && s2l_azimuth <= right)
        {
            scale = (right - s2l_azimuth ) / diff;
            vas_iir_biquad_setFrequency(data->directivityDamping, scale * 20000);
        }
        
        else if(s2l_azimuth < leftFull && s2l_azimuth >= left)
        {
            scale = fabs(left - s2l_azimuth ) / diff;
            vas_iir_biquad_setFrequency(data->directivityDamping, scale * 20000);
        }
        else
        {
            scale = 0.0;
        }
            
        data->p[P_TEST] = length;
        
        vas_iir_biquad_process(data->directivityDamping, data->input, data->tmp, length);
        vas_util_fscale(data->tmp, scale, length);*/

        vas_fir_binaural_process(data->binauralEngine, data->input, data->outputL, data->outputR, length);
        if(data->initReverbTail)
            vas_fir_binaural_processOutputInPlace(data->reverbEngine, data->input, data->outputL, data->outputR, length);
        
        if(data->config == VAS_SPAT_CONFIG_AUTO)
        {
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
                    
                    dir_x = m[0] * r_px + m[4] * r_py + m[8] * r_pz + m[12];
                    dir_y = m[1] * r_px + m[5] * r_py + m[9] * r_pz + m[13];
                    dir_z = m[2] * r_px + m[6] * r_py + m[10] * r_pz + m[14];
                     
                    azimuth = (fabsf(dir_z) < 0.001f) ? 0.0f : atan2f(dir_x, dir_z);
                    if (azimuth < 0.0f)
                        azimuth += 2.0f * kPI;
                    azimuth = FastClip(azimuth * kRad2Deg, 0.0f, 360.0f);
                     
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




