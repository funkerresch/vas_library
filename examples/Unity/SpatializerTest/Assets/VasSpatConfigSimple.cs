
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.IO;
using uOSC;

public class VasSpatConfigSimple : MonoBehaviour
{
    enum SpatParams
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
    }

    static int spatIdCounter;
    public string IrSet = "";
    public string ReverbTail = "";
    private int spatId;
    private AudioSource mySource;
    private IntPtr VAS_Unity_Spatializer;

    enum VAS_CONFIG { 
        UNDEFINED,
        SIMPLE,
        MANUAL,
        AUTO
    }

#if UNITY_IPHONE
   
    [DllImport ("__Internal")]
    private static extern IntPtr GetInstance(int id);

    [DllImport ("__Internal")]
    private static extern void LoadHRTF(IntPtr x, string str);

   // [DllImport ("__Internal")]
   // public static extern void SetDebugFunction(IntPtr fp);

#else
    [DllImport("AudioPlugin_VAS_Binaural", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr GetInstance(int id);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    private static extern void LoadHRTF(IntPtr x, string str);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    private static extern void LoadReverbTail(IntPtr x, string str);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    private static extern void SetConfig(IntPtr x, int config);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetDebugFunction(IntPtr fp);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DebugDelegate(string str);

    static void DebugCallback(string str)
    {
        Debug.Log(str);
    }
#endif

    void Awake()
    {
#if UNITY_IPHONE
        ;// Debug is not working under IOS
#else
        DebugDelegate callback_delegate = new DebugDelegate(DebugCallback);
        IntPtr intptr_delegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);
        SetDebugFunction(intptr_delegate);
#endif
        mySource = GetComponent<AudioSource>();
        spatId = spatIdCounter++;
        Debug.Log("SpatId: " + spatId);
        mySource.SetSpatializerFloat((int)SpatParams.P_SPATID, (float)spatId);
        mySource.SetSpatializerFloat((int)SpatParams.P_DIRECTIVITYDAMPING, 1f);

    }

    void Start()
    {
        VAS_Unity_Spatializer = GetInstance(spatId);
        SetConfig(VAS_Unity_Spatializer, (int)VAS_CONFIG.SIMPLE);
        if (VAS_Unity_Spatializer != IntPtr.Zero)
        {
            String fullpath = Path.Combine(Application.streamingAssetsPath, IrSet);
            //Debug.Log(fullpath);
            LoadHRTF(VAS_Unity_Spatializer, fullpath);
            fullpath = Path.Combine(Application.streamingAssetsPath, ReverbTail);
            //Debug.Log(fullpath);
            LoadReverbTail(VAS_Unity_Spatializer, fullpath);
        }
        else
            print("No Renderer Reference");

    }
}

