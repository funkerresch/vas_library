using System.Collections;
using UnityEngine;
using System;
using System.Runtime.InteropServices;
using UnityEngine.Networking;
using System.IO;
using System.Threading;
using System.Collections.Generic;

public enum SelectLoadingFunction
{
    EarlyIr,
    LateIr
};

[RequireComponent(typeof(AudioSource))]
abstract public class VasSpat : MonoBehaviour
{
    protected class ReadingIrThreadData
    {
        public int threadIndex;
        public string irName;
        public SelectLoadingFunction Read;
    }

    static private List<string> loadedIRs = new List<string>();
    static private int loadIrCounter = 0;
    private bool nothing2Download = false;
    protected string androidPersistantDataPath;

    protected bool earlyIrIsLoading = false;
    protected bool lateIrIsLoading = false;

    protected static int spatIdCounter = 0;
    protected int spatId;
    protected IntPtr VAS_Unity_Spatializer = IntPtr.Zero;

    public string IrSet = "";    public string ReverbTail = "";
    public AudioClip ReverbClip = null;
    [Range(0.0f, 1.0f)]
    public float scalingDirect = 1.0f;
    [Range(0.0f, 1.0f)]
    public float scalingEarly = 1.0f;
    [Range(0.0f, 1.0f)]
    public float scalingLate = 1.0f;

    public bool inverseAzimuth = false;
    public bool inverseElevation = false;
    public bool bypass = false;
    public bool listenerOrientationOnly = false;
    public int segmentSizeEarlyReverb = 1024;
    public int segmentSizeLateReverb = 4096;
    [Range(0.0f, 1000.0f)]
    public int binauralReflectionCount = 100;
    public bool recalculate = false;

    protected int VAS_MAXREFLECTIONORDER = 10;
    protected int VAS_MAXNUMBEROFRAYS = 100;
    protected int VAS_REFLECTIONPARAMETERS = 10;
    protected AudioSource mySource;
    protected float kPI = Mathf.PI;
    protected float kRad2Deg = 180 / Mathf.PI;
    protected AudioListener listener1;
    protected AnimationCurve curve;
    protected float maxDistance;
    protected float[] reflectionParameters = new float[10];

#if UNITY_IPHONE && !UNITY_EDITOR
   
    [DllImport ("__Internal")]
    protected static extern IntPtr GetInstance(int id);

    [DllImport ("__Internal")]
    protected static extern void LoadHRTF(IntPtr x, string str);

    [DllImport ("__Internal")]
    protected static extern void LoadReverbTail(IntPtr x, string str);

    [DllImport ("__Internal")]
    protected static extern void SetReflectionParameter(IntPtr x, int reflectionNumber, int reflectionParameter, float value);

    [DllImport ("__Internal")]
    protected static extern void SetReflectionParameters(IntPtr x, int reflectionNumber, float[] values);

    [DllImport ("__Internal")]
    protected static extern void SetConfig(IntPtr x, int config);

    [DllImport ("__Internal")]
    protected static extern bool EarlyPartIsLoaded(IntPtr x);

    [DllImport ("__Internal")]
    protected static extern bool LatePartIsLoaded(IntPtr x);

    [DllImport("__Internal")]
    protected static extern bool SetReverbTailFromUnityAudioClip(IntPtr x, float[] left, float[] right, int length);

#else
    [DllImport("AudioPlugin_VAS_Binaural", CallingConvention = CallingConvention.Cdecl)]
    protected static extern IntPtr GetInstance(int id);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void LoadHRTF(IntPtr x, string str);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void LoadReverbTail(IntPtr x, string str);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void SetReflectionParameter(IntPtr x, int reflectionNumber, int reflectionParameter, float value);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void SetReflectionParameters(IntPtr x, int reflectionNumber, float[] values);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern void SetConfig(IntPtr x, int config);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool EarlyPartIsLoaded(IntPtr x);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool LatePartIsLoaded(IntPtr x);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    protected static extern bool SetReverbTailFromUnityAudioClip(IntPtr x, float[] left, float[] right, int length);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetDebugFunction(IntPtr fp);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DebugDelegate(string str);

    public static void DebugCallback(string str)
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
        P_SCALING_EARLY,
        P_SCALING_LATE,
        P_SCALING_DIRECT,
        P_SCALING_STEREO,
        P_BINAURALREFLECTIONS,

        P_NUM
    }

    protected enum ReflectionParams
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

    protected void SetSpatializerInstanceAndConfig(VAS_CONFIG configuration)
    {
        spatId = spatIdCounter++;        Debug.Log("SpatId: " + spatId);        mySource.SetSpatializerFloat(3, (float)spatId);
        VAS_Unity_Spatializer = GetInstance(spatId);
        SetConfig(VAS_Unity_Spatializer, (int)configuration);
    }

    protected IEnumerator DownloadEarlyIr2PersistantDataPathForAndroid()
    {
        String fullpath = Path.Combine(Application.streamingAssetsPath, IrSet);
        UnityWebRequest www = UnityWebRequest.Get(fullpath);
        yield return www.SendWebRequest();

        if (www.isNetworkError || www.isHttpError)
        {
            Debug.Log("ERROR");
            Debug.Log(www.error);
        }
        else
        {
            Debug.Log("NO ERROR");
            Debug.Log(www.downloadHandler.text.Length);
            File.WriteAllText(Application.persistentDataPath + "/" + IrSet, www.downloadHandler.text);
            loadIrCounter--;
        }
    }

    protected IEnumerator DownloadLateIr2PersistantDataPathForAndroid()
    {
        String fullpath = Path.Combine(Application.streamingAssetsPath, ReverbTail);
        UnityWebRequest www = UnityWebRequest.Get(fullpath);
        yield return www.SendWebRequest();

        if (www.isNetworkError || www.isHttpError)
        {
            Debug.Log("ERROR");
            Debug.Log(www.error);
        }
        else
        {
            Debug.Log("NO ERROR");
            Debug.Log(www.downloadHandler.text.Length);
            File.WriteAllText(Application.persistentDataPath + "/" + ReverbTail, www.downloadHandler.text);
            loadIrCounter--;
        }
    }

    protected String GetFullIrPath(String name)
    {
#if UNITY_ANDROID && !UNITY_EDITOR
        String fullpath = Path.Combine(androidPersistantDataPath + "/", name);
#else
        String fullpath = Path.Combine(Application.streamingAssetsPath, name);
#endif
        return fullpath;
    }

    protected void ReadImpulseResponse_ThreadPool(System.Object a)
    {
        ReadingIrThreadData threadData = a as ReadingIrThreadData;
        try
        {
            if (threadData.Read == SelectLoadingFunction.EarlyIr)
            {
                earlyIrIsLoading = true;
#if UNITY_ANDROID && !UNITY_EDITOR
                while (loadIrCounter != 0)
                    ;
                Debug.Log("Android Download Early IR Finished");
#endif
            }
            if (threadData.Read == SelectLoadingFunction.LateIr)
            {
                lateIrIsLoading = true;
#if UNITY_ANDROID && !UNITY_EDITOR
                while (loadIrCounter != 0)
                    ;
                Debug.Log("Android Download Late IR Finished");
#endif
            }

            String fullpath = GetFullIrPath(threadData.irName);
            Debug.Log(fullpath);
            if(threadData.Read == SelectLoadingFunction.EarlyIr)
                LoadHRTF(VAS_Unity_Spatializer, fullpath);
            if (threadData.Read == SelectLoadingFunction.LateIr)
                LoadReverbTail(VAS_Unity_Spatializer, fullpath);
        }
        finally
        {
            if (threadData.Read == SelectLoadingFunction.EarlyIr)
            {
                earlyIrIsLoading = false;
                Debug.Log("Early IR is loaded");
            }

            if (threadData.Read == SelectLoadingFunction.LateIr)
            {
                lateIrIsLoading = false;
                Debug.Log("Late IR is loaded");
            }
        }
    }

    protected void ReadImpulseResponse()
    {
        if (VAS_Unity_Spatializer != IntPtr.Zero)
        {
            if (!String.IsNullOrWhiteSpace(IrSet))
            {
                ReadingIrThreadData earlyIR = new ReadingIrThreadData();
                earlyIR.irName = IrSet;
                earlyIR.Read = SelectLoadingFunction.EarlyIr;
#if UNITY_ANDROID && !UNITY_EDITOR
                if(nothing2Download)
                    Debug.Log("No IR to copy!");
                else
                    StartCoroutine(DownloadEarlyIr2PersistantDataPathForAndroid());
#endif
                ThreadPool.QueueUserWorkItem(new WaitCallback(ReadImpulseResponse_ThreadPool), earlyIR);
            }
            if (ReverbClip == null)
            {
                if (!String.IsNullOrWhiteSpace(ReverbTail))
                {
                    ReadingIrThreadData lateIR = new ReadingIrThreadData();
                    lateIR.irName = ReverbTail;
                    lateIR.Read = SelectLoadingFunction.LateIr;
#if UNITY_ANDROID && !UNITY_EDITOR
                    if (nothing2Download)
                        Debug.Log("No IR to copy!");
                    else
                        StartCoroutine(DownloadLateIr2PersistantDataPathForAndroid());
#endif
                    ThreadPool.QueueUserWorkItem(new WaitCallback(ReadImpulseResponse_ThreadPool), lateIR);   
                }
            }
            else
            {
                float[] all = new float[ReverbClip.samples * ReverbClip.channels];
                float[] left = new float[ReverbClip.samples];
                float[] right = new float[ReverbClip.samples];
                ReverbClip.GetData(all, 0);
        
                for (int i = 0; i < ReverbClip.samples; ++i)
                {
                    left[i] = all[i * 2];
                    right[i] = all[i * 2 + 1];
                }
     
                SetReverbTailFromUnityAudioClip(VAS_Unity_Spatializer, left, right, ReverbClip.samples);
                Debug.Log("Loaded Tail from Wav");
            }
        }
        else
            print("No Renderer Reference");
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

    protected void Update()
    {
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_EARLY, scalingEarly);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_LATE, scalingLate);
    }

#if UNITY_ANDROID && !UNITY_EDITOR
    private void AddImpulseResponse2Load(string ir)
    {
        foreach (string loadedIr in loadedIRs)
        {
            Debug.Log("CHECKING IR: " + loadedIr);
            if (String.Equals(ir, loadedIr))
            {
                Debug.Log("IR SET EXISTS ALREADY");
                nothing2Download = true;
                return;
            } 
        }

        loadedIRs.Add(ir);
        loadIrCounter++;
        Debug.Log("NEW IR SET: " + ir);    
    }
#endif

    protected void Awake()    {
        spatIdCounter = 0;
        mySource = GetComponent<AudioSource>();
        androidPersistantDataPath = Application.persistentDataPath;
        listener1 = GameObject.FindObjectOfType<AudioListener>();

#if UNITY_EDITOR && !UNITY_IOS
        DebugDelegate callback_delegate = new DebugDelegate(DebugCallback);
        IntPtr intptr_delegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);
        SetDebugFunction(intptr_delegate);
#endif

#if UNITY_ANDROID && !UNITY_EDITOR        if (!String.IsNullOrWhiteSpace(IrSet))
            AddImpulseResponse2Load(IrSet);
        if (!String.IsNullOrWhiteSpace(ReverbTail))
            AddImpulseResponse2Load(ReverbTail);
#endif    }

    protected void OnValidate()
    {
        BypassSpat(bypass);        ListenerOrientationOnly(listenerOrientationOnly);        InverseAzimuth(inverseAzimuth);        InverseElevation(inverseElevation);
        
        Debug.Log("Validate");
    }
}
