using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.IO;
using uOSC;

public class VasSpatializerConfiguration : MonoBehaviour
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

        P_REF1_X, //4
        P_REF1_Y,
        P_REF1_Z,

        P_REF2_X, //7
        P_REF2_Y,
        P_REF2_Z,

        P_REF3_X, //10
        P_REF3_Y,
        P_REF3_Z,

        P_REF4_X, //13
        P_REF4_Y,
        P_REF4_Z,

        P_REF5_X, //13
        P_REF5_Y,
        P_REF5_Z,

        P_REF6_X, //13
        P_REF6_Y,
        P_REF6_Z,

        P_T,
        P_TEST,

        P_NUM
    }

    public enum ReflectionMaterial
    { 
        Metal,
        Normal,
        Wood
    }

    static int spatIdCounter = 0;
    public string IrSet = "";
    static string globalIrSet = "";
    private int spatId;
    private AudioSource mySource;
    public float horizontalSourceDirectivity = 180;
    public float horizontalFullPowerRange = 120;
   

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
        ;
#else
        DebugDelegate callback_delegate = new DebugDelegate(DebugCallback);
        IntPtr intptr_delegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);
        SetDebugFunction(intptr_delegate);
#endif
        mySource = GetComponent<AudioSource>();
    }

    void Start()
    {
        spatId = spatIdCounter++;
        Debug.Log("SpatId: " + spatId);
        mySource.SetSpatializerFloat(3, (float)spatId);

        mySource.SetSpatializerFloat((int)SpatParams.P_H_SOURCEDIRECTIVITY, horizontalSourceDirectivity);
        mySource.SetSpatializerFloat((int)SpatParams.P_H_FULLPOWERRANGE, horizontalFullPowerRange);

        IntPtr VAS_Unity_Spatializer = GetInstance(spatId);

        if (VAS_Unity_Spatializer != IntPtr.Zero)
        {
            String fullpath = Path.Combine(Application.streamingAssetsPath, IrSet);
            Debug.Log(fullpath);
            LoadHRTF(VAS_Unity_Spatializer, fullpath);
        }
        else
            print("No Renderer Reference");
    }

    private void Update()
    {
        mySource.SetSpatializerFloat((int)SpatParams.P_H_SOURCEDIRECTIVITY, horizontalSourceDirectivity);
        mySource.SetSpatializerFloat((int)SpatParams.P_H_FULLPOWERRANGE, horizontalFullPowerRange);
        float test;
        mySource.GetSpatializerFloat((int)SpatParams.P_TEST, out test);

    }
}