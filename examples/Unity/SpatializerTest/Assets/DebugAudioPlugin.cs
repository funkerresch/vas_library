using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.IO;
using uOSC;

enum SpatParams{
    P_AUDIOSRCATTN,
    P_FIXEDVOLUME,
    P_CUSTOMFALLOFF,
    P_SPATID,

    P_REF1_X,
    P_REF1_Y,
    P_REF1_Z,

    P_NUM


}

public class DebugAudioPlugin : MonoBehaviour
{
    static int spatIdCounter = 0;
    public string IrSet = "";
    private int spatId;
    private AudioSource mySource;
    public GameObject reflection1;
    public GameObject reflection2;
    public GameObject reflection3;
    public GameObject reflection4;

#if UNITY_IPHONE
   
    [DllImport ("__Internal")]
    private static extern IntPtr GetInstance(int id);

    [DllImport ("__Internal")]
    private static extern void LoadHRTF(IntPtr x, string str);

    [DllImport ("__Internal")]
    public static extern void SetDebugFunction(IntPtr fp);

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

        mySource.SetSpatializerFloat((int)SpatParams.P_REF1_X, (float)reflection1.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF1_Y, (float)reflection1.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF1_Z, (float)reflection1.transform.position.z);

        IntPtr vas_binaural = GetInstance(spatId);

        if (vas_binaural != IntPtr.Zero)
        {
            String fullpath = Path.Combine(Application.streamingAssetsPath, IrSet);
            Debug.Log(fullpath);
            LoadHRTF(vas_binaural, fullpath);
        }
        else
            print("No Renderer Reference");
    }

    private void Update()
    {
        mySource.SetSpatializerFloat((int)SpatParams.P_REF1_X, (float)reflection1.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF1_Y, (float)reflection1.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF1_Z, (float)reflection1.transform.position.z);
        //Debug.Log(reflection1.transform.position.x);
    }

    private void OnValidate()
    {

    }

}
