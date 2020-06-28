using System.Collections;using System.Collections.Generic;using UnityEngine;using System.Runtime.InteropServices;using System;using System.IO;
using UnityEngine.UI;public class VasSpatConfigSimple : VasSpat{    //public Toggle bypassButton;                         // TOGGLE BYBASS    void OnValidate()    {        BypassSpat(bypass);        ListenerOrientationOnly(listenerOrientationOnly);        InverseAzimuth(inverseAzimuth);        InverseElevation(inverseElevation);    }    void Awake()    {
        spatIdCounter = 0;

#if UNITY_IPHONE
        ;
#else
        //DebugDelegate callback_delegate = new DebugDelegate(DebugCallback);
        //IntPtr intptr_delegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);
        //SetDebugFunction(intptr_delegate);
#endif        if (Application.isPlaying)
        {
            mySource = GetComponent<AudioSource>();
        }    }    void Start()    {
        if (!Application.isPlaying)
           return;
                spatId = spatIdCounter++;        Debug.Log("SpatId: " + spatId);        mySource.SetSpatializerFloat(3, (float)spatId);
        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_EARLYPART, segmentSizeEarlyReverb);
        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_LATEPART, segmentSizeLateReverb);
        InverseAzimuth(inverseAzimuth);        InverseElevation(inverseElevation);        BypassSpat(bypass);        ListenerOrientationOnly(listenerOrientationOnly);        VAS_Unity_Spatializer = GetInstance(spatId);        SetConfig(VAS_Unity_Spatializer, (int)VAS_CONFIG.SIMPLE);        if (VAS_Unity_Spatializer != IntPtr.Zero)        {            String fullpath = Path.Combine(Application.streamingAssetsPath, IrSet);            //Debug.Log(fullpath);            LoadHRTF(VAS_Unity_Spatializer, fullpath);            fullpath = Path.Combine(Application.streamingAssetsPath, ReverbTail);            //Debug.Log(fullpath);            LoadReverbTail(VAS_Unity_Spatializer, fullpath);            BypassSpat(bypass);        }        else            print("No Renderer Reference");        //bypassButton.onValueChanged.AddListener(delegate { BypassSpat(bypassButton.isOn); }); // TOGGLE BYBASS    }}