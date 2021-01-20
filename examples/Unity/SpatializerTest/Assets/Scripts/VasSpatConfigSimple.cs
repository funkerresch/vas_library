using UnityEngine;using System;using System.IO;using System.Runtime.InteropServices;
using System.Threading;

public class VasSpatConfigSimple : VasSpat{       void Start()    {
        SetSpatializerInstanceAndConfig(VAS_CONFIG.SIMPLE);

        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_EARLYPART, segmentSizeEarlyReverb);
        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_LATEPART, segmentSizeLateReverb);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_EARLY, scalingEarly);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_LATE, scalingLate);
        InverseAzimuth(inverseAzimuth);        InverseElevation(inverseElevation);        BypassSpat(bypass);        ListenerOrientationOnly(listenerOrientationOnly);                ReadImpulseResponse();        

    }}