#include "AudioPluginUtil.h"
#include "vas_fir_binaural.h"
#include <iostream>
#include <string>

namespace BinauralRIR {
    
    enum Param
    {   
        P_AZI,
        P_IR,
        P_NUM
    };
    
    struct EffectData
    {
        float p[P_NUM]; // Parameters
        int currentIrIndex;
        float input[1024];
        float outputL[1024];
        float outputR[1024];
        char *path;
        vas_fir_binaural *binauralEngine;
    };
 
    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
    {
        int numparams = P_NUM;
        definition.paramdefs = new UnityAudioParameterDefinition [numparams];

        RegisterParameter(definition, "azimuth", "",
                          0.0f, 359.0f, 0.0f,
                          1.0f, 1.0f,
                          P_AZI);
        
        RegisterParameter(definition, "selectir", "",
                          -1.0f, 12.0f, -1.0f,
                          1.0f, 1.0f,
                          P_IR);
        
        return numparams;
    }
    
    void readImpulseResponse(vas_fir *x, char *path, int ir)
    {
        char fullIrPath[512];
        char irName[512];
        sprintf(irName, "fr_pos%d.txt", ir);
        size_t found;
        std::string str((path));
        found=str.find_last_of("/\\");
        if(ir >= 0)
        {
            sprintf(fullIrPath, "%s/%s", path, irName);
            vas_fir_readText_1IrPerLine(x, fullIrPath);
        }
    }
   
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        if (index >= P_NUM)
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        data->p[index] = value;
        
        if(index == P_AZI)
        {
            vas_dynamicFirChannel_setAzimuth(data->binauralEngine->left, value);
            vas_dynamicFirChannel_setAzimuth(data->binauralEngine->right, value);
        }
        
        if(index == P_IR)
        {
            if( (data->currentIrIndex != (int) value))
            {
                data->currentIrIndex = (int) value;
                readImpulseResponse((vas_fir *)data->binauralEngine, data->path, (int) value);
                vas_fir_setInitFlag((vas_fir *)data->binauralEngine);
            }
        }
       
        return UNITY_AUDIODSP_OK;
    }
    
    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char *valuestr)
    {
        EffectData *data = state->GetEffectData<EffectData>();
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

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
    {
        EffectData* effectdata = new EffectData;
        memset(effectdata, 0, sizeof(EffectData));
        
        effectdata->currentIrIndex = -1;
        effectdata->path = (char *) malloc(sizeof(char) * 512);
        
        effectdata->binauralEngine = vas_fir_binaural_new(VAS_VDSP | VAS_BINAURALSETUP_NOELEVATION | VAS_LOCALFILTER, 512, NULL);

        char* path = NULL;
        int length, dirname_length;
        
        length = wai_getModulePath(NULL, 0, &dirname_length);
        if (length > 0)
        {
            path = (char*)malloc(length + 1);
            wai_getModulePath(path, length, &dirname_length);
            path[dirname_length] = '\0';
            strcpy(effectdata->path, path);
            free(path);
        }
        else
            printf("No Dylib Path");
        
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->p);
        
        state->effectdata = effectdata;
      
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        vas_fir_binaural_free(data->binauralEngine);
        delete data;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(UnityAudioEffectState* state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int outchannels)
    {
        EffectData *data = state->GetEffectData<EffectData>();
        
        if( (outchannels != 2) || (inchannels != 2) || (data->p[P_IR] < 0) )
        {
            for(unsigned int n = 0; n < length; n++)
            {
                outbuffer[n * outchannels] = 0.0;
                outbuffer[n * outchannels + 1] = 0.0;
            }
            
            return UNITY_AUDIODSP_OK;
        }
      
        for(unsigned int n = 0; n < length; n++)
        {
            data->input[n] = inbuffer[n * inchannels];
        }
        
        vas_fir_binaural_process(data->binauralEngine, &data->input[0], &data->outputL[0], &data->outputR[0], length);
        
        for(unsigned int n = 0; n < length; n++)
        {
            outbuffer[n * outchannels] = data->outputL[n];
            outbuffer[n * outchannels + 1] = data->outputR[n];
        }
        
        return UNITY_AUDIODSP_OK;
        
    }
}



