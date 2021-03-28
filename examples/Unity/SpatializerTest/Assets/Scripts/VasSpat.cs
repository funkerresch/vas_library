using System.Collections;
using UnityEngine;
using System;
using System.Runtime.InteropServices;
using UnityEngine.Networking;
using System.IO;
using System.Threading;
using System.Collections.Generic;

public enum IrType
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
        public IrType Read;
    }

    static private List<string> loadedIRs = new List<string>();
    static private int loadIrCounter = 0;
    protected string androidPersistantDataPath;

    protected bool noEarlyIr2Download = false;
    protected bool noLateIr2Download = false;

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

    // not in use, therefore private for now
    private float horizontalSourceDirectivity = 360;
    private float horizontalFullPowerRange = 360;
    private float verticalSourceDirectivity = 360;
    private float verticalFullPowerRange = 360;

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
    protected static extern bool SetReverbTailFromUnityAudioClip(IntPtr x, string name, float[] left, float[] right, int length);

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
    protected static extern bool SetReverbTailFromUnityAudioClip(IntPtr x, string name, float[] left, float[] right, int length);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetDebugFunction(IntPtr fp);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DebugDelegate(string str);

    public static void DebugCallback(string str)
    {
        Debug.Log(str);
    }
#endif

    public enum SpatParams
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
        P_AZI,
        P_ELE,

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
        P_REF_AZI,
        P_REF_ELE,
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

    protected void CalculateVDirectivityScaling(float s2lFullSphericElevation, out float vDirectivityScaling)
    {
        float verticalDirectivityOver2 = verticalSourceDirectivity * 0.5f;
        float verticalFullEnergyOver2 = verticalFullPowerRange * 0.5f;
        float top = 360 - verticalDirectivityOver2;
        float bottom = verticalDirectivityOver2;
        float topFull = 360 - verticalFullEnergyOver2;
        float bottomFull = verticalFullEnergyOver2;
        float vDiff = topFull - top;

        if (s2lFullSphericElevation <= bottomFull || s2lFullSphericElevation >= topFull)
            vDirectivityScaling = 1.0f;

        else if (s2lFullSphericElevation > bottomFull && s2lFullSphericElevation <= bottom)
        {
            vDirectivityScaling = (bottom - s2lFullSphericElevation) / vDiff;
        }

        else if (s2lFullSphericElevation < topFull && s2lFullSphericElevation >= top)
        {
            vDirectivityScaling = Mathf.Abs(top - s2lFullSphericElevation) / vDiff;
        }
        else
        {
            vDirectivityScaling = 0.0f;
        }

        vDirectivityScaling = ((Mathf.Pow(10f, vDirectivityScaling)) - 1f) / 9f;
    }

    protected void CalculateHDirectivityScaling(float s2lAzimuth, out float hDirectivityScaling)
    {
        float horizontalDirectivityOver2 = horizontalSourceDirectivity * 0.5f;
        float horizontalFullEnergyOver2 = horizontalFullPowerRange * 0.5f;
        float left = 360 - horizontalDirectivityOver2;
        float right = horizontalDirectivityOver2;
        float leftFull = 360 - horizontalFullEnergyOver2;
        float rightFull = horizontalFullEnergyOver2;
        float hDiff = leftFull - left;

        if (s2lAzimuth <= rightFull || s2lAzimuth >= leftFull)
            hDirectivityScaling = 1.0f;

        else if (s2lAzimuth > rightFull && s2lAzimuth <= right)
            hDirectivityScaling = (right - s2lAzimuth) / hDiff;

        else if (s2lAzimuth < leftFull && s2lAzimuth >= left)
            hDirectivityScaling = Mathf.Abs(left - s2lAzimuth) / hDiff;
        else
            hDirectivityScaling = 0.0f;

        hDirectivityScaling = ((Mathf.Pow(10f, hDirectivityScaling)) - 1f) / 9f;
    }

    protected void CalculateSource2ListenerAzimuthAndElevation(out float source2ListenerAzimuth, out float source2ListenerElevation)
    {
        var listenerMatrix = listener1.transform.localToWorldMatrix;
        var sourceMatrix = mySource.transform.localToWorldMatrix;

        float px = listenerMatrix[12];
        float py = listenerMatrix[13];
        float pz = listenerMatrix[14];

        float listenerpos_x = -(sourceMatrix[0] * sourceMatrix[12] + sourceMatrix[1] * sourceMatrix[13] + sourceMatrix[2] * sourceMatrix[14]);
        float listenerpos_y = -(sourceMatrix[4] * sourceMatrix[12] + sourceMatrix[5] * sourceMatrix[13] + sourceMatrix[6] * sourceMatrix[14]);
        float listenerpos_z = -(sourceMatrix[8] * sourceMatrix[12] + sourceMatrix[9] * sourceMatrix[13] + sourceMatrix[10] * sourceMatrix[14]);

        float dir_x = sourceMatrix[0] * px + sourceMatrix[4] * py + sourceMatrix[8] * pz + listenerpos_x;
        float dir_y = sourceMatrix[1] * px + sourceMatrix[5] * py + sourceMatrix[9] * pz + listenerpos_y;
        float dir_z = sourceMatrix[2] * px + sourceMatrix[6] * py + sourceMatrix[10] * pz + listenerpos_z;

        float azimuth = (Mathf.Abs(dir_z) < 0.001f) ? 0.0f : Mathf.Atan2(dir_x, dir_z);
        if (azimuth < 0.0f)
            azimuth += 2.0f * kPI;

        azimuth = Mathf.Clamp(azimuth * kRad2Deg, 0.0f, 360.0f);
        source2ListenerAzimuth = azimuth;

        float elevation = (Mathf.Abs(dir_z) < 0.001f) ? 0.0f : Mathf.Atan2(dir_y, dir_z);
        if (elevation < 0.0f)
            elevation += 2.0f * kPI;

        elevation = Mathf.Clamp(elevation * kRad2Deg, 0.0f, 360.0f);
        source2ListenerElevation = elevation;
    }

    protected void CalculateAziAndElevation(Vector3 sourcePosition, out float azimuth, out float elevation)
    {
        var cameraMatrix = listener1.transform.localToWorldMatrix;
        var listenerMatrix = cameraMatrix.inverse;

        float px = sourcePosition.x;
        float py = sourcePosition.y;
        float pz = sourcePosition.z;

        float dir_x = listenerMatrix[0] * px + listenerMatrix[4] * py + listenerMatrix[8] * pz + listenerMatrix[12];
        float dir_y = listenerMatrix[1] * px + listenerMatrix[5] * py + listenerMatrix[9] * pz + listenerMatrix[13];
        float dir_z = listenerMatrix[2] * px + listenerMatrix[6] * py + listenerMatrix[10] * pz + listenerMatrix[14];

        azimuth = (Mathf.Abs(dir_z) < 0.001f) ? 0.0f : Mathf.Atan2(dir_x, dir_z);
        if (azimuth < 0.0f)
            azimuth += 2.0f * kPI;
        azimuth = Mathf.Clamp(azimuth * kRad2Deg, 0.0f, 360.0f);

        if (inverseAzimuth)
            azimuth = 360 - azimuth;

        elevation = Mathf.Atan2(dir_y, Mathf.Sqrt(dir_x * dir_x + dir_z * dir_z) + 0.001f) * kRad2Deg;
    }

    protected void CalculateAziAndElevationFromListenerOrientationOnly(out float azimuth, out float elevation)
    {
        var cameraMatrix = listener1.transform.localToWorldMatrix;
        var listenerMatrix = cameraMatrix.inverse;
        float aziInRadians;

        if (listenerMatrix[0] == 1.0f)
        {
            aziInRadians = Mathf.Atan2(listenerMatrix[2], listenerMatrix[11]);
            elevation = Mathf.Atan2(-listenerMatrix[6], listenerMatrix[5] + 0.001f) * kRad2Deg;
        }
        else if (listenerMatrix[0] == -1.0f)
        {
            aziInRadians = Mathf.Atan2(listenerMatrix[2], listenerMatrix[11]);
            elevation = Mathf.Atan2(-listenerMatrix[6], listenerMatrix[5] + 0.001f) * kRad2Deg;
        }
        else
        {
            aziInRadians = Mathf.Atan2(listenerMatrix[8], listenerMatrix[0]);
            elevation = Mathf.Atan2(-listenerMatrix[6], listenerMatrix[5] + 0.001f) * kRad2Deg;
        }

        if (aziInRadians < 0)
            aziInRadians += 2*kPI;

        azimuth = aziInRadians * kRad2Deg;

        if (inverseAzimuth)
            azimuth = 360 - azimuth;
    }

    protected void SetSpatializerInstanceAndConfig(VAS_CONFIG configuration)
    {
        spatId = spatIdCounter++;        Debug.Log("SpatId: " + spatId);        mySource.SetSpatializerFloat(3, (float)spatId);
        VAS_Unity_Spatializer = GetInstance(spatId);
        SetConfig(VAS_Unity_Spatializer, (int)configuration);
    }

    protected IEnumerator DownloadIr2PersistantDataPathForAndroid(string fileName)
    {
        String fullpath = Path.Combine(Application.streamingAssetsPath, fileName);
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
            File.WriteAllText(Application.persistentDataPath + "/" + fileName, www.downloadHandler.text);
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
            if (threadData.Read == IrType.EarlyIr)
            {
#if UNITY_ANDROID && !UNITY_EDITOR
                while (loadIrCounter != 0)
                    ;
                Debug.Log("Android Download Early IR Finished");
#endif
            }
            if (threadData.Read == IrType.LateIr)
            {
#if UNITY_ANDROID && !UNITY_EDITOR
                while (loadIrCounter != 0)
                    ;
                Debug.Log("Android Download Late IR Finished");
#endif
            }

            String fullpath = GetFullIrPath(threadData.irName);
            Debug.Log(fullpath);
            if(threadData.Read == IrType.EarlyIr)
                LoadHRTF(VAS_Unity_Spatializer, fullpath);
            if (threadData.Read == IrType.LateIr)
                LoadReverbTail(VAS_Unity_Spatializer, fullpath);
        }
        finally
        {
            if (threadData.Read == IrType.EarlyIr)
                Debug.Log("Early IR is loaded");

            if (threadData.Read == IrType.LateIr)
                Debug.Log("Late IR is loaded");
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
                earlyIR.Read = IrType.EarlyIr;
#if UNITY_ANDROID && !UNITY_EDITOR
                if(noEarlyIr2Download)
                    Debug.Log("No IR to copy!");
                else
                    StartCoroutine(DownloadIr2PersistantDataPathForAndroid(IrSet));
#endif
                ThreadPool.QueueUserWorkItem(new WaitCallback(ReadImpulseResponse_ThreadPool), earlyIR);
            }
            if (ReverbClip == null)
            {
                if (!String.IsNullOrWhiteSpace(ReverbTail))
                {
                    ReadingIrThreadData lateIR = new ReadingIrThreadData();
                    lateIR.irName = ReverbTail;
                    lateIR.Read = IrType.LateIr;
#if UNITY_ANDROID && !UNITY_EDITOR
                    if (noLateIr2Download)
                        Debug.Log("No IR to copy!");
                    else
                        StartCoroutine(DownloadIr2PersistantDataPathForAndroid(ReverbTail));
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
     
                SetReverbTailFromUnityAudioClip(VAS_Unity_Spatializer, ReverbClip.name, left, right, ReverbClip.samples);
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

#if UNITY_ANDROID && !UNITY_EDITOR
    private void AddImpulseResponse2Load(string ir, IrType type )
    {
        foreach (string loadedIr in loadedIRs)
        {
            Debug.Log("CHECKING IR: " + loadedIr);
            if (String.Equals(ir, loadedIr))
            {
                Debug.Log("IR SET EXISTS ALREADY");
                if(type == IrType.EarlyIr)
                    noEarlyIr2Download = true;
                else
                    noLateIr2Download = true;
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
            AddImpulseResponse2Load(IrSet, IrType.EarlyIr);
        if (!String.IsNullOrWhiteSpace(ReverbTail))
            AddImpulseResponse2Load(ReverbTail, IrType.LateIr);
#endif    }

    protected void Update()
    {
        float azimuth, elevation;
        if (!listenerOrientationOnly)
            CalculateAziAndElevation(mySource.transform.position, out azimuth, out elevation);
        else
            CalculateAziAndElevationFromListenerOrientationOnly(out azimuth, out elevation);

        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_EARLY, scalingEarly);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_LATE, scalingLate);
        mySource.SetSpatializerFloat((int)SpatParams.P_AZI, azimuth);
        mySource.SetSpatializerFloat((int)SpatParams.P_ELE, elevation);
    }

    protected void OnValidate()
    {
        BypassSpat(bypass);        ListenerOrientationOnly(listenerOrientationOnly);        InverseAzimuth(inverseAzimuth);        InverseElevation(inverseElevation);
    }
}
