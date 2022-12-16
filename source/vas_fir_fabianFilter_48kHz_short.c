#include <stdio.h>
#include "vas_fir_fabianFilter_48kHz.h"
#include "vas_fir_binaural.h"

#ifdef __cplusplus
extern "C" {
#endif




vas_fir_binaural *fabianHrtf_48kHz;
int fabianHrtfInit_48kHz;
    
#ifdef __cplusplus
}
#endif

void vas_fir_fabianFilter_48kHz_init(void)
{
    fabianHrtf_48kHz = vas_fir_binaural_new(0);
    vas_fir_setMultiDirection3DegreeGridResoluion((vas_fir *)fabianHrtf_48kHz, 512, 512, 0, 0);
    
    int eleRange = 60;
    int aziRange = 120;
    
    for(int eleCount = 0; eleCount < eleRange; eleCount++)
    {
        for(int aziCount = 0; aziCount < aziRange; aziCount++)
        {
            vas_dynamicFirChannel_prepareFilter(fabianHrtf_48kHz->left, fabian_hrtfLeft_48kHz[eleCount][aziCount],  eleCount, aziCount);
            vas_dynamicFirChannel_prepareFilter(fabianHrtf_48kHz->right, fabian_hrtfRight_48kHz[eleCount][aziCount],  eleCount, aziCount);
        }
    }
}
