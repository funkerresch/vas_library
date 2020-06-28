using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Runtime.InteropServices;
using UnityEditor.SceneManagement;
using UnityEngine.SceneManagement;
using System.IO;

[ExecuteInEditMode]
[RequireComponent(typeof(AudioSource))]
abstract public class VasSpat : MonoBehaviour
{
    protected static int spatIdCounter = 0;
    protected int spatId;
    protected IntPtr VAS_Unity_Spatializer = IntPtr.Zero;

    public string IrSet = "";    public string ReverbTail = "";
    public bool inverseAzimuth = false;
    public bool inverseElevation = false;
    public bool bypass = false;
    public bool listenerOrientationOnly = false;
    public int segmentSizeEarlyReverb = 1024;
    public int segmentSizeLateReverb = 4096;

    protected int VAS_MAXREFLECTIONORDER = 12;
    protected int VAS_MAXNUMBEROFRAYS = 12;
    protected int VAS_REFLECTIONPARAMETERS = 7;
    protected AudioSource mySource;
    protected float kPI = Mathf.PI;
    protected float kRad2Deg = 180 / Mathf.PI;

#if UNITY_IPHONE
   
    [DllImport ("__Internal")]
    protected static extern IntPtr GetInstance(int id);

    [DllImport ("__Internal")]
    protected static extern void LoadHRTF(IntPtr x, string str);

    [DllImport ("__Internal")]
    protected static extern void LoadReverbTail(IntPtr x, string str);

    [DllImport ("__Internal")]
    protected static extern void SetConfig(IntPtr x, int config);

    [DllImport ("__Internal")]
    public static extern void SetDebugFunction(IntPtr fp);

   // [DllImport ("__Internal")]
   // public static extern void SetDebugFunction(IntPtr fp);

#else
    [DllImport("AudioPlugin_VAS_Binaural", CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr GetInstance(int id);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void LoadHRTF(IntPtr x, string str);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void LoadReverbTail(IntPtr x, string str);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void SetConfig(IntPtr x, int config);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetDebugFunction(IntPtr fp);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DebugDelegate(string str);

    static void DebugCallback(string str)
    {
        Debug.Log(str);
    }
#endif

    protected enum SpatParams
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

        P_REF_1_1_X, //9
        P_REF_1_1_Y,
        P_REF_1_1_Z,
        P_REF_1_1_MAT,
        P_REF_1_1_MUTE,
        P_REF_1_1_SCALE,
        P_REF_1_1_DIST,

        P_REF_1_2_X, //13
        P_REF_1_2_Y,
        P_REF_1_2_Z,
        P_REF_1_2_MAT,
        P_REF_1_2_MUTE,
        P_REF_1_2_SCALE,
        P_REF_1_2_DIST,

        P_REF_1_3_X,
        P_REF_1_3_Y,
        P_REF_1_3_Z,
        P_REF_1_3_MAT,
        P_REF_1_3_MUTE,
        P_REF_1_3_SCALE,
        P_REF_1_3_DIST,

        P_REF_1_4_X,
        P_REF_1_4_Y,
        P_REF_1_4_Z,
        P_REF_1_4_MAT,
        P_REF_1_4_MUTE,
        P_REF_1_4_SCALE,
        P_REF_1_4_DIST,

        P_REF_1_5_X,
        P_REF_1_5_Y,
        P_REF_1_5_Z,
        P_REF_1_5_MAT,
        P_REF_1_5_MUTE,
        P_REF_1_5_SCALE,
        P_REF_1_5_DIST,

        P_REF_1_6_X,
        P_REF_1_6_Y,
        P_REF_1_6_Z,
        P_REF_1_6_MAT,
        P_REF_1_6_MUTE,
        P_REF_1_6_SCALE,
        P_REF_1_6_DIST,

        P_REF_1_7_X,
        P_REF_1_7_Y,
        P_REF_1_7_Z,
        P_REF_1_7_MAT,
        P_REF_1_7_MUTE,
        P_REF_1_7_SCALE,
        P_REF_1_7_DIST,

        P_REF_1_8_X,
        P_REF_1_8_Y,
        P_REF_1_8_Z,
        P_REF_1_8_MAT,
        P_REF_1_8_MUTE,
        P_REF_1_8_SCALE,
        P_REF_1_8_DIST,

        P_REF_1_9_X,
        P_REF_1_9_Y,
        P_REF_1_9_Z,
        P_REF_1_9_MAT,
        P_REF_1_9_MUTE,
        P_REF_1_9_SCALE,
        P_REF_1_9_DIST,

        P_REF_1_10_X,
        P_REF_1_10_Y,
        P_REF_1_10_Z,
        P_REF_1_10_MAT,
        P_REF_1_10_MUTE,
        P_REF_1_10_SCALE,
        P_REF_1_10_DIST,

        P_REF_1_11_X,
        P_REF_1_11_Y,
        P_REF_1_11_Z,
        P_REF_1_11_MAT,
        P_REF_1_11_MUTE,
        P_REF_1_11_SCALE,
        P_REF_1_11_DIST,

        P_REF_1_12_X,
        P_REF_1_12_Y,
        P_REF_1_12_Z,
        P_REF_1_12_MAT,
        P_REF_1_12_MUTE,
        P_REF_1_12_SCALE,
        P_REF_1_12_DIST,

        P_REF_2_1_X, //9
        P_REF_2_1_Y,
        P_REF_2_1_Z,
        P_REF_2_1_MAT,
        P_REF_2_1_MUTE,
        P_REF_2_1_SCALE,
        P_REF_2_1_DIST,

        P_REF_2_2_X, //13
        P_REF_2_2_Y,
        P_REF_2_2_Z,
        P_REF_2_2_MAT,
        P_REF_2_2_MUTE,
        P_REF_2_2_SCALE,
        P_REF_2_2_DIST,

        P_REF_2_3_X,
        P_REF_2_3_Y,
        P_REF_2_3_Z,
        P_REF_2_3_MAT,
        P_REF_2_3_MUTE,
        P_REF_2_3_SCALE,
        P_REF_2_3_DIST,

        P_REF_2_4_X,
        P_REF_2_4_Y,
        P_REF_2_4_Z,
        P_REF_2_4_MAT,
        P_REF_2_4_MUTE,
        P_REF_2_4_SCALE,
        P_REF_2_4_DIST,

        P_REF_2_5_X,
        P_REF_2_5_Y,
        P_REF_2_5_Z,
        P_REF_2_5_MAT,
        P_REF_2_5_MUTE,
        P_REF_2_5_SCALE,
        P_REF_2_5_DIST,

        P_REF_2_6_X,
        P_REF_2_6_Y,
        P_REF_2_6_Z,
        P_REF_2_6_MAT,
        P_REF_2_6_MUTE,
        P_REF_2_6_SCALE,
        P_REF_2_6_DIST,

        P_REF_2_7_X,
        P_REF_2_7_Y,
        P_REF_2_7_Z,
        P_REF_2_7_MAT,
        P_REF_2_7_MUTE,
        P_REF_2_7_SCALE,
        P_REF_2_7_DIST,

        P_REF_2_8_X,
        P_REF_2_8_Y,
        P_REF_2_8_Z,
        P_REF_2_8_MAT,
        P_REF_2_8_MUTE,
        P_REF_2_8_SCALE,
        P_REF_2_8_DIST,

        P_REF_2_9_X,
        P_REF_2_9_Y,
        P_REF_2_9_Z,
        P_REF_2_9_MAT,
        P_REF_2_9_MUTE,
        P_REF_2_9_SCALE,
        P_REF_2_9_DIST,

        P_REF_2_10_X,
        P_REF_2_10_Y,
        P_REF_2_10_Z,
        P_REF_2_10_MAT,
        P_REF_2_10_MUTE,
        P_REF_2_10_SCALE,
        P_REF_2_10_DIST,

        P_REF_2_11_X,
        P_REF_2_11_Y,
        P_REF_2_11_Z,
        P_REF_2_11_MAT,
        P_REF_2_11_MUTE,
        P_REF_2_11_SCALE,
        P_REF_2_11_DIST,

        P_REF_2_12_X,
        P_REF_2_12_Y,
        P_REF_2_12_Z,
        P_REF_2_12_MAT,
        P_REF_2_12_MUTE,
        P_REF_2_12_SCALE,
        P_REF_2_12_DIST,

        P_REF_3_1_X, //9
        P_REF_3_1_Y,
        P_REF_3_1_Z,
        P_REF_3_1_MAT,
        P_REF_3_1_MUTE,
        P_REF_3_1_SCALE,
        P_REF_3_1_DIST,

        P_REF_3_2_X, //13
        P_REF_3_2_Y,
        P_REF_3_2_Z,
        P_REF_3_2_MAT,
        P_REF_3_2_MUTE,
        P_REF_3_2_SCALE,
        P_REF_3_2_DIST,

        P_REF_3_3_X,
        P_REF_3_3_Y,
        P_REF_3_3_Z,
        P_REF_3_3_MAT,
        P_REF_3_3_MUTE,
        P_REF_3_3_SCALE,
        P_REF_3_3_DIST,

        P_REF_3_4_X,
        P_REF_3_4_Y,
        P_REF_3_4_Z,
        P_REF_3_4_MAT,
        P_REF_3_4_MUTE,
        P_REF_3_4_SCALE,
        P_REF_3_4_DIST,

        P_REF_3_5_X,
        P_REF_3_5_Y,
        P_REF_3_5_Z,
        P_REF_3_5_MAT,
        P_REF_3_5_MUTE,
        P_REF_3_5_SCALE,
        P_REF_3_5_DIST,

        P_REF_3_6_X,
        P_REF_3_6_Y,
        P_REF_3_6_Z,
        P_REF_3_6_MAT,
        P_REF_3_6_MUTE,
        P_REF_3_6_SCALE,
        P_REF_3_6_DIST,

        P_REF_3_7_X,
        P_REF_3_7_Y,
        P_REF_3_7_Z,
        P_REF_3_7_MAT,
        P_REF_3_7_MUTE,
        P_REF_3_7_SCALE,
        P_REF_3_7_DIST,

        P_REF_3_8_X,
        P_REF_3_8_Y,
        P_REF_3_8_Z,
        P_REF_3_8_MAT,
        P_REF_3_8_MUTE,
        P_REF_3_8_SCALE,
        P_REF_3_8_DIST,

        P_REF_3_9_X,
        P_REF_3_9_Y,
        P_REF_3_9_Z,
        P_REF_3_9_MAT,
        P_REF_3_9_MUTE,
        P_REF_3_9_SCALE,
        P_REF_3_9_DIST,

        P_REF_3_10_X,
        P_REF_3_10_Y,
        P_REF_3_10_Z,
        P_REF_3_10_MAT,
        P_REF_3_10_MUTE,
        P_REF_3_10_SCALE,
        P_REF_3_10_DIST,

        P_REF_3_11_X,
        P_REF_3_11_Y,
        P_REF_3_11_Z,
        P_REF_3_11_MAT,
        P_REF_3_11_MUTE,
        P_REF_3_11_SCALE,
        P_REF_3_11_DIST,

        P_REF_3_12_X,
        P_REF_3_12_Y,
        P_REF_3_12_Z,
        P_REF_3_12_MAT,
        P_REF_3_12_MUTE,
        P_REF_3_12_SCALE,
        P_REF_3_12_DIST,

        P_REF_4_1_X, //9
        P_REF_4_1_Y,
        P_REF_4_1_Z,
        P_REF_4_1_MAT,
        P_REF_4_1_MUTE,
        P_REF_4_1_SCALE,
        P_REF_4_1_DIST,

        P_REF_4_2_X, //13
        P_REF_4_2_Y,
        P_REF_4_2_Z,
        P_REF_4_2_MAT,
        P_REF_4_2_MUTE,
        P_REF_4_2_SCALE,
        P_REF_4_2_DIST,

        P_REF_4_3_X,
        P_REF_4_3_Y,
        P_REF_4_3_Z,
        P_REF_4_3_MAT,
        P_REF_4_3_MUTE,
        P_REF_4_3_SCALE,
        P_REF_4_3_DIST,

        P_REF_4_4_X,
        P_REF_4_4_Y,
        P_REF_4_4_Z,
        P_REF_4_4_MAT,
        P_REF_4_4_MUTE,
        P_REF_4_4_SCALE,
        P_REF_4_4_DIST,

        P_REF_4_5_X,
        P_REF_4_5_Y,
        P_REF_4_5_Z,
        P_REF_4_5_MAT,
        P_REF_4_5_MUTE,
        P_REF_4_5_SCALE,
        P_REF_4_5_DIST,

        P_REF_4_6_X,
        P_REF_4_6_Y,
        P_REF_4_6_Z,
        P_REF_4_6_MAT,
        P_REF_4_6_MUTE,
        P_REF_4_6_SCALE,
        P_REF_4_6_DIST,

        P_REF_4_7_X,
        P_REF_4_7_Y,
        P_REF_4_7_Z,
        P_REF_4_7_MAT,
        P_REF_4_7_MUTE,
        P_REF_4_7_SCALE,
        P_REF_4_7_DIST,

        P_REF_4_8_X,
        P_REF_4_8_Y,
        P_REF_4_8_Z,
        P_REF_4_8_MAT,
        P_REF_4_8_MUTE,
        P_REF_4_8_SCALE,
        P_REF_4_8_DIST,

        P_REF_4_9_X,
        P_REF_4_9_Y,
        P_REF_4_9_Z,
        P_REF_4_9_MAT,
        P_REF_4_9_MUTE,
        P_REF_4_9_SCALE,
        P_REF_4_9_DIST,

        P_REF_4_10_X,
        P_REF_4_10_Y,
        P_REF_4_10_Z,
        P_REF_4_10_MAT,
        P_REF_4_10_MUTE,
        P_REF_4_10_SCALE,
        P_REF_4_10_DIST,

        P_REF_4_11_X,
        P_REF_4_11_Y,
        P_REF_4_11_Z,
        P_REF_4_11_MAT,
        P_REF_4_11_MUTE,
        P_REF_4_11_SCALE,
        P_REF_4_11_DIST,

        P_REF_4_12_X,
        P_REF_4_12_Y,
        P_REF_4_12_Z,
        P_REF_4_12_MAT,
        P_REF_4_12_MUTE,
        P_REF_4_12_SCALE,
        P_REF_4_12_DIST,

        P_REF_5_1_X, //9
        P_REF_5_1_Y,
        P_REF_5_1_Z,
        P_REF_5_1_MAT,
        P_REF_5_1_MUTE,
        P_REF_5_1_SCALE,
        P_REF_5_1_DIST,

        P_REF_5_2_X, //13
        P_REF_5_2_Y,
        P_REF_5_2_Z,
        P_REF_5_2_MAT,
        P_REF_5_2_MUTE,
        P_REF_5_2_SCALE,
        P_REF_5_2_DIST,

        P_REF_5_3_X,
        P_REF_5_3_Y,
        P_REF_5_3_Z,
        P_REF_5_3_MAT,
        P_REF_5_3_MUTE,
        P_REF_5_3_SCALE,
        P_REF_5_3_DIST,

        P_REF_5_4_X,
        P_REF_5_4_Y,
        P_REF_5_4_Z,
        P_REF_5_4_MAT,
        P_REF_5_4_MUTE,
        P_REF_5_4_SCALE,
        P_REF_5_4_DIST,

        P_REF_5_5_X,
        P_REF_5_5_Y,
        P_REF_5_5_Z,
        P_REF_5_5_MAT,
        P_REF_5_5_MUTE,
        P_REF_5_5_SCALE,
        P_REF_5_5_DIST,

        P_REF_5_6_X,
        P_REF_5_6_Y,
        P_REF_5_6_Z,
        P_REF_5_6_MAT,
        P_REF_5_6_MUTE,
        P_REF_5_6_SCALE,
        P_REF_5_6_DIST,

        P_REF_5_7_X,
        P_REF_5_7_Y,
        P_REF_5_7_Z,
        P_REF_5_7_MAT,
        P_REF_5_7_MUTE,
        P_REF_5_7_SCALE,
        P_REF_5_7_DIST,

        P_REF_5_8_X,
        P_REF_5_8_Y,
        P_REF_5_8_Z,
        P_REF_5_8_MAT,
        P_REF_5_8_MUTE,
        P_REF_5_8_SCALE,
        P_REF_5_8_DIST,

        P_REF_5_9_X,
        P_REF_5_9_Y,
        P_REF_5_9_Z,
        P_REF_5_9_MAT,
        P_REF_5_9_MUTE,
        P_REF_5_9_SCALE,
        P_REF_5_9_DIST,

        P_REF_5_10_X,
        P_REF_5_10_Y,
        P_REF_5_10_Z,
        P_REF_5_10_MAT,
        P_REF_5_10_MUTE,
        P_REF_5_10_SCALE,
        P_REF_5_10_DIST,

        P_REF_5_11_X,
        P_REF_5_11_Y,
        P_REF_5_11_Z,
        P_REF_5_11_MAT,
        P_REF_5_11_MUTE,
        P_REF_5_11_SCALE,
        P_REF_5_11_DIST,

        P_REF_5_12_X,
        P_REF_5_12_Y,
        P_REF_5_12_Z,
        P_REF_5_12_MAT,
        P_REF_5_12_MUTE,
        P_REF_5_12_SCALE,
        P_REF_5_12_DIST,

        P_REF_6_1_X, //9
        P_REF_6_1_Y,
        P_REF_6_1_Z,
        P_REF_6_1_MAT,
        P_REF_6_1_MUTE,
        P_REF_6_1_SCALE,
        P_REF_6_1_DIST,

        P_REF_6_2_X, //13
        P_REF_6_2_Y,
        P_REF_6_2_Z,
        P_REF_6_2_MAT,
        P_REF_6_2_MUTE,
        P_REF_6_2_SCALE,
        P_REF_6_2_DIST,

        P_REF_6_3_X,
        P_REF_6_3_Y,
        P_REF_6_3_Z,
        P_REF_6_3_MAT,
        P_REF_6_3_MUTE,
        P_REF_6_3_SCALE,
        P_REF_6_3_DIST,

        P_REF_6_4_X,
        P_REF_6_4_Y,
        P_REF_6_4_Z,
        P_REF_6_4_MAT,
        P_REF_6_4_MUTE,
        P_REF_6_4_SCALE,
        P_REF_6_4_DIST,

        P_REF_6_5_X,
        P_REF_6_5_Y,
        P_REF_6_5_Z,
        P_REF_6_5_MAT,
        P_REF_6_5_MUTE,
        P_REF_6_5_SCALE,
        P_REF_6_5_DIST,

        P_REF_6_6_X,
        P_REF_6_6_Y,
        P_REF_6_6_Z,
        P_REF_6_6_MAT,
        P_REF_6_6_MUTE,
        P_REF_6_6_SCALE,
        P_REF_6_6_DIST,

        P_REF_6_7_X,
        P_REF_6_7_Y,
        P_REF_6_7_Z,
        P_REF_6_7_MAT,
        P_REF_6_7_MUTE,
        P_REF_6_7_SCALE,
        P_REF_6_7_DIST,

        P_REF_6_8_X,
        P_REF_6_8_Y,
        P_REF_6_8_Z,
        P_REF_6_8_MAT,
        P_REF_6_8_MUTE,
        P_REF_6_8_SCALE,
        P_REF_6_8_DIST,

        P_REF_6_9_X,
        P_REF_6_9_Y,
        P_REF_6_9_Z,
        P_REF_6_9_MAT,
        P_REF_6_9_MUTE,
        P_REF_6_9_SCALE,
        P_REF_6_9_DIST,

        P_REF_6_10_X,
        P_REF_6_10_Y,
        P_REF_6_10_Z,
        P_REF_6_10_MAT,
        P_REF_6_10_MUTE,
        P_REF_6_10_SCALE,
        P_REF_6_10_DIST,

        P_REF_6_11_X,
        P_REF_6_11_Y,
        P_REF_6_11_Z,
        P_REF_6_11_MAT,
        P_REF_6_11_MUTE,
        P_REF_6_11_SCALE,
        P_REF_6_11_DIST,

        P_REF_6_12_X,
        P_REF_6_12_Y,
        P_REF_6_12_Z,
        P_REF_6_12_MAT,
        P_REF_6_12_MUTE,
        P_REF_6_12_SCALE,
        P_REF_6_12_DIST,

        P_REF_7_1_X, //9
        P_REF_7_1_Y,
        P_REF_7_1_Z,
        P_REF_7_1_MAT,
        P_REF_7_1_MUTE,
        P_REF_7_1_SCALE,
        P_REF_7_1_DIST,

        P_REF_7_2_X, //13
        P_REF_7_2_Y,
        P_REF_7_2_Z,
        P_REF_7_2_MAT,
        P_REF_7_2_MUTE,
        P_REF_7_2_SCALE,
        P_REF_7_2_DIST,

        P_REF_7_3_X,
        P_REF_7_3_Y,
        P_REF_7_3_Z,
        P_REF_7_3_MAT,
        P_REF_7_3_MUTE,
        P_REF_7_3_SCALE,
        P_REF_7_3_DIST,

        P_REF_7_4_X,
        P_REF_7_4_Y,
        P_REF_7_4_Z,
        P_REF_7_4_MAT,
        P_REF_7_4_MUTE,
        P_REF_7_4_SCALE,
        P_REF_7_4_DIST,

        P_REF_7_5_X,
        P_REF_7_5_Y,
        P_REF_7_5_Z,
        P_REF_7_5_MAT,
        P_REF_7_5_MUTE,
        P_REF_7_5_SCALE,
        P_REF_7_5_DIST,

        P_REF_7_6_X,
        P_REF_7_6_Y,
        P_REF_7_6_Z,
        P_REF_7_6_MAT,
        P_REF_7_6_MUTE,
        P_REF_7_6_SCALE,
        P_REF_7_6_DIST,

        P_REF_7_7_X,
        P_REF_7_7_Y,
        P_REF_7_7_Z,
        P_REF_7_7_MAT,
        P_REF_7_7_MUTE,
        P_REF_7_7_SCALE,
        P_REF_7_7_DIST,

        P_REF_7_8_X,
        P_REF_7_8_Y,
        P_REF_7_8_Z,
        P_REF_7_8_MAT,
        P_REF_7_8_MUTE,
        P_REF_7_8_SCALE,
        P_REF_7_8_DIST,

        P_REF_7_9_X,
        P_REF_7_9_Y,
        P_REF_7_9_Z,
        P_REF_7_9_MAT,
        P_REF_7_9_MUTE,
        P_REF_7_9_SCALE,
        P_REF_7_9_DIST,

        P_REF_7_10_X,
        P_REF_7_10_Y,
        P_REF_7_10_Z,
        P_REF_7_10_MAT,
        P_REF_7_10_MUTE,
        P_REF_7_10_SCALE,
        P_REF_7_10_DIST,

        P_REF_7_11_X,
        P_REF_7_11_Y,
        P_REF_7_11_Z,
        P_REF_7_11_MAT,
        P_REF_7_11_MUTE,
        P_REF_7_11_SCALE,
        P_REF_7_11_DIST,

        P_REF_7_12_X,
        P_REF_7_12_Y,
        P_REF_7_12_Z,
        P_REF_7_12_MAT,
        P_REF_7_12_MUTE,
        P_REF_7_12_SCALE,
        P_REF_7_12_DIST,

        P_REF_8_1_X, //9
        P_REF_8_1_Y,
        P_REF_8_1_Z,
        P_REF_8_1_MAT,
        P_REF_8_1_MUTE,
        P_REF_8_1_SCALE,
        P_REF_8_1_DIST,

        P_REF_8_2_X, //13
        P_REF_8_2_Y,
        P_REF_8_2_Z,
        P_REF_8_2_MAT,
        P_REF_8_2_MUTE,
        P_REF_8_2_SCALE,
        P_REF_8_2_DIST,

        P_REF_8_3_X,
        P_REF_8_3_Y,
        P_REF_8_3_Z,
        P_REF_8_3_MAT,
        P_REF_8_3_MUTE,
        P_REF_8_3_SCALE,
        P_REF_8_3_DIST,

        P_REF_8_4_X,
        P_REF_8_4_Y,
        P_REF_8_4_Z,
        P_REF_8_4_MAT,
        P_REF_8_4_MUTE,
        P_REF_8_4_SCALE,
        P_REF_8_4_DIST,

        P_REF_8_5_X,
        P_REF_8_5_Y,
        P_REF_8_5_Z,
        P_REF_8_5_MAT,
        P_REF_8_5_MUTE,
        P_REF_8_5_SCALE,
        P_REF_8_5_DIST,

        P_REF_8_6_X,
        P_REF_8_6_Y,
        P_REF_8_6_Z,
        P_REF_8_6_MAT,
        P_REF_8_6_MUTE,
        P_REF_8_6_SCALE,
        P_REF_8_6_DIST,

        P_REF_8_7_X,
        P_REF_8_7_Y,
        P_REF_8_7_Z,
        P_REF_8_7_MAT,
        P_REF_8_7_MUTE,
        P_REF_8_7_SCALE,
        P_REF_8_7_DIST,

        P_REF_8_8_X,
        P_REF_8_8_Y,
        P_REF_8_8_Z,
        P_REF_8_8_MAT,
        P_REF_8_8_MUTE,
        P_REF_8_8_SCALE,
        P_REF_8_8_DIST,

        P_REF_8_9_X,
        P_REF_8_9_Y,
        P_REF_8_9_Z,
        P_REF_8_9_MAT,
        P_REF_8_9_MUTE,
        P_REF_8_9_SCALE,
        P_REF_8_9_DIST,

        P_REF_8_10_X,
        P_REF_8_10_Y,
        P_REF_8_10_Z,
        P_REF_8_10_MAT,
        P_REF_8_10_MUTE,
        P_REF_8_10_SCALE,
        P_REF_8_10_DIST,

        P_REF_8_11_X,
        P_REF_8_11_Y,
        P_REF_8_11_Z,
        P_REF_8_11_MAT,
        P_REF_8_11_MUTE,
        P_REF_8_11_SCALE,
        P_REF_8_11_DIST,

        P_REF_8_12_X,
        P_REF_8_12_Y,
        P_REF_8_12_Z,
        P_REF_8_12_MAT,
        P_REF_8_12_MUTE,
        P_REF_8_12_SCALE,
        P_REF_8_12_DIST,

        P_REF_9_1_X, //9
        P_REF_9_1_Y,
        P_REF_9_1_Z,
        P_REF_9_1_MAT,
        P_REF_9_1_MUTE,
        P_REF_9_1_SCALE,
        P_REF_9_1_DIST,

        P_REF_9_2_X, //13
        P_REF_9_2_Y,
        P_REF_9_2_Z,
        P_REF_9_2_MAT,
        P_REF_9_2_MUTE,
        P_REF_9_2_SCALE,
        P_REF_9_2_DIST,

        P_REF_9_3_X,
        P_REF_9_3_Y,
        P_REF_9_3_Z,
        P_REF_9_3_MAT,
        P_REF_9_3_MUTE,
        P_REF_9_3_SCALE,
        P_REF_9_3_DIST,

        P_REF_9_4_X,
        P_REF_9_4_Y,
        P_REF_9_4_Z,
        P_REF_9_4_MAT,
        P_REF_9_4_MUTE,
        P_REF_9_4_SCALE,
        P_REF_9_4_DIST,

        P_REF_9_5_X,
        P_REF_9_5_Y,
        P_REF_9_5_Z,
        P_REF_9_5_MAT,
        P_REF_9_5_MUTE,
        P_REF_9_5_SCALE,
        P_REF_9_5_DIST,

        P_REF_9_6_X,
        P_REF_9_6_Y,
        P_REF_9_6_Z,
        P_REF_9_6_MAT,
        P_REF_9_6_MUTE,
        P_REF_9_6_SCALE,
        P_REF_9_6_DIST,

        P_REF_9_7_X,
        P_REF_9_7_Y,
        P_REF_9_7_Z,
        P_REF_9_7_MAT,
        P_REF_9_7_MUTE,
        P_REF_9_7_SCALE,
        P_REF_9_7_DIST,

        P_REF_9_8_X,
        P_REF_9_8_Y,
        P_REF_9_8_Z,
        P_REF_9_8_MAT,
        P_REF_9_8_MUTE,
        P_REF_9_8_SCALE,
        P_REF_9_8_DIST,

        P_REF_9_9_X,
        P_REF_9_9_Y,
        P_REF_9_9_Z,
        P_REF_9_9_MAT,
        P_REF_9_9_MUTE,
        P_REF_9_9_SCALE,
        P_REF_9_9_DIST,

        P_REF_9_10_X,
        P_REF_9_10_Y,
        P_REF_9_10_Z,
        P_REF_9_10_MAT,
        P_REF_9_10_MUTE,
        P_REF_9_10_SCALE,
        P_REF_9_10_DIST,

        P_REF_9_11_X,
        P_REF_9_11_Y,
        P_REF_9_11_Z,
        P_REF_9_11_MAT,
        P_REF_9_11_MUTE,
        P_REF_9_11_SCALE,
        P_REF_9_11_DIST,

        P_REF_9_12_X,
        P_REF_9_12_Y,
        P_REF_9_12_Z,
        P_REF_9_12_MAT,
        P_REF_9_12_MUTE,
        P_REF_9_12_SCALE,
        P_REF_9_12_DIST,

        P_REF_10_1_X, //9
        P_REF_10_1_Y,
        P_REF_10_1_Z,
        P_REF_10_1_MAT,
        P_REF_10_1_MUTE,
        P_REF_10_1_SCALE,
        P_REF_10_1_DIST,

        P_REF_10_2_X, //13
        P_REF_10_2_Y,
        P_REF_10_2_Z,
        P_REF_10_2_MAT,
        P_REF_10_2_MUTE,
        P_REF_10_2_SCALE,
        P_REF_10_2_DIST,

        P_REF_10_3_X,
        P_REF_10_3_Y,
        P_REF_10_3_Z,
        P_REF_10_3_MAT,
        P_REF_10_3_MUTE,
        P_REF_10_3_SCALE,
        P_REF_10_3_DIST,

        P_REF_10_4_X,
        P_REF_10_4_Y,
        P_REF_10_4_Z,
        P_REF_10_4_MAT,
        P_REF_10_4_MUTE,
        P_REF_10_4_SCALE,
        P_REF_10_4_DIST,

        P_REF_10_5_X,
        P_REF_10_5_Y,
        P_REF_10_5_Z,
        P_REF_10_5_MAT,
        P_REF_10_5_MUTE,
        P_REF_10_5_SCALE,
        P_REF_10_5_DIST,

        P_REF_10_6_X,
        P_REF_10_6_Y,
        P_REF_10_6_Z,
        P_REF_10_6_MAT,
        P_REF_10_6_MUTE,
        P_REF_10_6_SCALE,
        P_REF_10_6_DIST,

        P_REF_10_7_X,
        P_REF_10_7_Y,
        P_REF_10_7_Z,
        P_REF_10_7_MAT,
        P_REF_10_7_MUTE,
        P_REF_10_7_SCALE,
        P_REF_10_7_DIST,

        P_REF_10_8_X,
        P_REF_10_8_Y,
        P_REF_10_8_Z,
        P_REF_10_8_MAT,
        P_REF_10_8_MUTE,
        P_REF_10_8_SCALE,
        P_REF_10_8_DIST,

        P_REF_10_9_X,
        P_REF_10_9_Y,
        P_REF_10_9_Z,
        P_REF_10_9_MAT,
        P_REF_10_9_MUTE,
        P_REF_10_9_SCALE,
        P_REF_10_9_DIST,

        P_REF_10_10_X,
        P_REF_10_10_Y,
        P_REF_10_10_Z,
        P_REF_10_10_MAT,
        P_REF_10_10_MUTE,
        P_REF_10_10_SCALE,
        P_REF_10_10_DIST,

        P_REF_10_11_X,
        P_REF_10_11_Y,
        P_REF_10_11_Z,
        P_REF_10_11_MAT,
        P_REF_10_11_MUTE,
        P_REF_10_11_SCALE,
        P_REF_10_11_DIST,

        P_REF_10_12_X,
        P_REF_10_12_Y,
        P_REF_10_12_Z,
        P_REF_10_12_MAT,
        P_REF_10_12_MUTE,
        P_REF_10_12_SCALE,
        P_REF_10_12_DIST,

        P_REF_11_1_X, //9
        P_REF_11_1_Y,
        P_REF_11_1_Z,
        P_REF_11_1_MAT,
        P_REF_11_1_MUTE,
        P_REF_11_1_SCALE,
        P_REF_11_1_DIST,

        P_REF_11_2_X, //13
        P_REF_11_2_Y,
        P_REF_11_2_Z,
        P_REF_11_2_MAT,
        P_REF_11_2_MUTE,
        P_REF_11_2_SCALE,
        P_REF_11_2_DIST,

        P_REF_11_3_X,
        P_REF_11_3_Y,
        P_REF_11_3_Z,
        P_REF_11_3_MAT,
        P_REF_11_3_MUTE,
        P_REF_11_3_SCALE,
        P_REF_11_3_DIST,

        P_REF_11_4_X,
        P_REF_11_4_Y,
        P_REF_11_4_Z,
        P_REF_11_4_MAT,
        P_REF_11_4_MUTE,
        P_REF_11_4_SCALE,
        P_REF_11_4_DIST,

        P_REF_11_5_X,
        P_REF_11_5_Y,
        P_REF_11_5_Z,
        P_REF_11_5_MAT,
        P_REF_11_5_MUTE,
        P_REF_11_5_SCALE,
        P_REF_11_5_DIST,

        P_REF_11_6_X,
        P_REF_11_6_Y,
        P_REF_11_6_Z,
        P_REF_11_6_MAT,
        P_REF_11_6_MUTE,
        P_REF_11_6_SCALE,
        P_REF_11_6_DIST,

        P_REF_11_7_X,
        P_REF_11_7_Y,
        P_REF_11_7_Z,
        P_REF_11_7_MAT,
        P_REF_11_7_MUTE,
        P_REF_11_7_SCALE,
        P_REF_11_7_DIST,

        P_REF_11_8_X,
        P_REF_11_8_Y,
        P_REF_11_8_Z,
        P_REF_11_8_MAT,
        P_REF_11_8_MUTE,
        P_REF_11_8_SCALE,
        P_REF_11_8_DIST,

        P_REF_11_9_X,
        P_REF_11_9_Y,
        P_REF_11_9_Z,
        P_REF_11_9_MAT,
        P_REF_11_9_MUTE,
        P_REF_11_9_SCALE,
        P_REF_11_9_DIST,

        P_REF_11_10_X,
        P_REF_11_10_Y,
        P_REF_11_10_Z,
        P_REF_11_10_MAT,
        P_REF_11_10_MUTE,
        P_REF_11_10_SCALE,
        P_REF_11_10_DIST,

        P_REF_11_11_X,
        P_REF_11_11_Y,
        P_REF_11_11_Z,
        P_REF_11_11_MAT,
        P_REF_11_11_MUTE,
        P_REF_11_11_SCALE,
        P_REF_11_11_DIST,

        P_REF_11_12_X,
        P_REF_11_12_Y,
        P_REF_11_12_Z,
        P_REF_11_12_MAT,
        P_REF_11_12_MUTE,
        P_REF_11_12_SCALE,
        P_REF_11_12_DIST,

        P_REF_12_1_X, //9
        P_REF_12_1_Y,
        P_REF_12_1_Z,
        P_REF_12_1_MAT,
        P_REF_12_1_MUTE,
        P_REF_12_1_SCALE,
        P_REF_12_1_DIST,

        P_REF_12_2_X, //13
        P_REF_12_2_Y,
        P_REF_12_2_Z,
        P_REF_12_2_MAT,
        P_REF_12_2_MUTE,
        P_REF_12_2_SCALE,
        P_REF_12_2_DIST,

        P_REF_12_3_X,
        P_REF_12_3_Y,
        P_REF_12_3_Z,
        P_REF_12_3_MAT,
        P_REF_12_3_MUTE,
        P_REF_12_3_SCALE,
        P_REF_12_3_DIST,

        P_REF_12_4_X,
        P_REF_12_4_Y,
        P_REF_12_4_Z,
        P_REF_12_4_MAT,
        P_REF_12_4_MUTE,
        P_REF_12_4_SCALE,
        P_REF_12_4_DIST,

        P_REF_12_5_X,
        P_REF_12_5_Y,
        P_REF_12_5_Z,
        P_REF_12_5_MAT,
        P_REF_12_5_MUTE,
        P_REF_12_5_SCALE,
        P_REF_12_5_DIST,

        P_REF_12_6_X,
        P_REF_12_6_Y,
        P_REF_12_6_Z,
        P_REF_12_6_MAT,
        P_REF_12_6_MUTE,
        P_REF_12_6_SCALE,
        P_REF_12_6_DIST,

        P_REF_12_7_X,
        P_REF_12_7_Y,
        P_REF_12_7_Z,
        P_REF_12_7_MAT,
        P_REF_12_7_MUTE,
        P_REF_12_7_SCALE,
        P_REF_12_7_DIST,

        P_REF_12_8_X,
        P_REF_12_8_Y,
        P_REF_12_8_Z,
        P_REF_12_8_MAT,
        P_REF_12_8_MUTE,
        P_REF_12_8_SCALE,
        P_REF_12_8_DIST,

        P_REF_12_9_X,
        P_REF_12_9_Y,
        P_REF_12_9_Z,
        P_REF_12_9_MAT,
        P_REF_12_9_MUTE,
        P_REF_12_9_SCALE,
        P_REF_12_9_DIST,

        P_REF_12_10_X,
        P_REF_12_10_Y,
        P_REF_12_10_Z,
        P_REF_12_10_MAT,
        P_REF_12_10_MUTE,
        P_REF_12_10_SCALE,
        P_REF_12_10_DIST,

        P_REF_12_11_X,
        P_REF_12_11_Y,
        P_REF_12_11_Z,
        P_REF_12_11_MAT,
        P_REF_12_11_MUTE,
        P_REF_12_11_SCALE,
        P_REF_12_11_DIST,

        P_REF_12_12_X,
        P_REF_12_12_Y,
        P_REF_12_12_Z,
        P_REF_12_12_MAT,
        P_REF_12_12_MUTE,
        P_REF_12_12_SCALE,
        P_REF_12_12_DIST,

        P_NUM
    }

    protected int reflectionOffset = (int)(SpatParams.P_REF_1_1_X);

    protected enum ReflectionMaterial
    {
        Metal,
        Normal,
        Wood
    }

    protected enum VAS_CONFIG
    {
        UNDEFINED,
        SIMPLE,
        MANUAL,
        AUTO
    }

    protected void BypassSpat(bool onOff)
    {
        if (!mySource)
            return;

        if (onOff == true)
            mySource.SetSpatializerFloat((int)SpatParams.P_BYPASS, 1f);
        else
            mySource.SetSpatializerFloat((int)SpatParams.P_BYPASS, 0);
    }

    protected void ListenerOrientationOnly(bool onOff)
    {
        if (!mySource)
            return;

        if (onOff == true)
            mySource.SetSpatializerFloat((int)SpatParams.P_LISTENERORIENTATIONONLY, 1f);
        else
            mySource.SetSpatializerFloat((int)SpatParams.P_LISTENERORIENTATIONONLY, 0);
    }

    protected void InverseAzimuth(bool onOff)    {        if (!mySource)
            return;

        if (onOff == true)            mySource.SetSpatializerFloat((int)SpatParams.P_INVERSEAZI, 1f);        else            mySource.SetSpatializerFloat((int)SpatParams.P_INVERSEAZI, 0);    }    protected void InverseElevation(bool onOff)    {        if (!mySource)
            return;

        if (onOff == true)            mySource.SetSpatializerFloat((int)SpatParams.P_INVERSEELE, 1f);        else            mySource.SetSpatializerFloat((int)SpatParams.P_INVERSEELE, 0);    }

    void Reset()
    {
        //Output the message to the Console
        Debug.Log("Reset");
    }

    protected void Awake()
    {
        
    }

    protected void OnValidate()
    {
       /* spatScripts = FindObjectsOfType<VasSpat>();
        if (spatScripts.Length != numberOfSpatScripts)
        {
            Debug.Log("new Vas  Script");
            numberOfSpatScripts = spatScripts.Length;
        }*/
    }

    void OnEnable()
    {      
        //Scene current = EditorSceneManager.GetActiveScene();

        //  spatScripts = FindObjectsOfType<VasSpat>();
        //  if (spatScripts.Length != numberOfSpatScripts && current.isDirty && !Application.isPlaying)
        //  {
        //      numberOfSpatScripts = spatScripts.Length;
        //      bool v = EditorSceneManager.SaveScene(current);
        //      EditorSceneManager.OpenScene(current.path, OpenSceneMode.Single);
        //      Debug.Log("Save Scene");
        //  }
    }
}
