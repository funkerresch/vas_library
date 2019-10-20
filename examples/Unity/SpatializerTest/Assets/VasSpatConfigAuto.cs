using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.IO;
using uOSC;

public class VasSpatConfigAuto : MonoBehaviour
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

    public enum ReflectionMaterial
    {
        Metal,
        Normal,
        Wood
    }

    enum VAS_CONFIG
    {
        UNDEFINED,
        SIMPLE,
        MANUAL,
        AUTO
    }

    static int spatIdCounter = 0;
    public string IrSet = "";
    public bool autoPositionReflections;
    public float horizontalSourceDirectivity = 180;
    public float rayDistance = 30;
    public float horizontalFullPowerRange = 120;
    public int numberOfRays = 6;
    public int reflectionOrder = 1;
    public Material raycastMaterial;

    private Transform goTransform;
    private LineRenderer[] lineRenderer;
    private int reflectionOffset = (int) (SpatParams.P_REF_1_1_X);
    private int VAS_MAXREFLECTIONORDER = 10;
    private Ray ray;
    private RaycastHit hit;
    private Vector3 inDirection;
    private int spatId;
    private AudioSource mySource;
    private GameObject[] lineRenderObject;
    private int nPoints;
    private AudioListener listener;
    private Transform listenerTransform;
    private float horizontalDirectivityOver2; 
    private float horizontalFullEnergyOver2;
    private float left;
    private float right;
    private float leftFull;
    private float rightFull;
    private float diff;
    private float s2lAzimuth; // for directivity
    private float s2lElevation;
    private float directivityScaling;

#if UNITY_IPHONE
   
    [DllImport ("__Internal")]
    private static extern IntPtr GetInstance(int id);

    [DllImport ("__Internal")]
    private static extern void LoadHRTF(IntPtr x, string str);

    [DllImport ("__Internal")]
    private static extern void LoadReverbTail(IntPtr x, string str);

    [DllImport ("__Internal")]
    private static extern void SetConfig(IntPtr x, int config);

    [DllImport ("__Internal")]
    public static extern void SetDebugFunction(IntPtr fp);

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
        ;
#else
        DebugDelegate callback_delegate = new DebugDelegate(DebugCallback);
        IntPtr intptr_delegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);
        SetDebugFunction(intptr_delegate);
#endif
        mySource = GetComponent<AudioSource>();
        goTransform = this.GetComponent<Transform>();
        lineRenderer = new LineRenderer[numberOfRays];
        lineRenderObject = new GameObject[numberOfRays];
        listener = GameObject.FindObjectOfType<AudioListener>();
        listenerTransform = listener.GetComponent<Transform>();

        for (int i = 0; i < numberOfRays; i++)
        {
            lineRenderObject[i] = new GameObject("MyGameObject");
            lineRenderer[i] = lineRenderObject[i].AddComponent<LineRenderer>();
            lineRenderer[i].material = raycastMaterial;
            lineRenderer[i].SetWidth(0.15f, 0.15f);
        }
    }

    void CalculateDirectivityScaling()
    {
        horizontalDirectivityOver2 = horizontalSourceDirectivity * 0.5f;
        horizontalFullEnergyOver2 = horizontalFullPowerRange * 0.5f;
        left = 360 - horizontalDirectivityOver2;
        right = horizontalDirectivityOver2;
        leftFull = 360 - horizontalFullEnergyOver2;
        rightFull = horizontalFullEnergyOver2;
        diff = leftFull - left;
        CalculateAzimuth();
        CalculateElevation();

        if (s2lAzimuth <= rightFull || s2lAzimuth >= leftFull)
            directivityScaling = 1.0f;

        else if (s2lAzimuth > rightFull && s2lAzimuth <= right)
        {
            directivityScaling = (right - s2lAzimuth) / diff;
        }

        else if (s2lAzimuth < leftFull && s2lAzimuth >= left)
        {
            directivityScaling = Mathf.Abs(left - s2lAzimuth) / diff;
        }
        else
        {
            directivityScaling = 0.0f;
        }

        directivityScaling = ((Mathf.Pow(10f, directivityScaling))-1f) /9f;
    }

    void CalculateAzimuth()
    {
        var heading = goTransform.position - listenerTransform.position;
        float azimuth = Mathf.Atan2(heading.x, heading.z) * Mathf.Rad2Deg;
        var angle = goTransform.rotation.eulerAngles;
        azimuth += angle.y + 180f;
        azimuth %= 360f;
        s2lAzimuth = azimuth;
       // Debug.Log("Azi: " + azimuth);
    }

    void CalculateElevation()
    {
        var heading = goTransform.position - listenerTransform.position;
        float elevation = Mathf.Atan2(heading.y, Mathf.Sqrt(heading.x*heading.x + heading.z * heading.z)) * Mathf.Rad2Deg;
        var angle = goTransform.rotation.eulerAngles;
        float rotationX = angle.x;
        rotationX %= 360;

        if (rotationX <= 180)
            elevation -= rotationX;
        else
            elevation += ( 360 - rotationX);

        s2lElevation = elevation;
        //Debug.Log("Ele: " + elevation);
    }

    void RaytraceReflections(int rayNumber, float azimuth, float elevation)
    {
       // reflectionOrder = Mathf.Clamp(reflectionOrder, 1, reflectionOrder);
        azimuth = Mathf.Deg2Rad * (azimuth- (horizontalFullPowerRange*0.5f));
        elevation = Mathf.Deg2Rad * elevation;

        float x = Mathf.Sin(azimuth) * Mathf.Cos(elevation);
        float y = Mathf.Cos(azimuth) * Mathf.Cos(elevation);
        float z = Mathf.Sin(elevation);

        Vector3 rayDirection = new Vector3(x, z, y);
        rayDirection = transform.TransformDirection(rayDirection);
        ray = new Ray(goTransform.position, rayDirection);

        nPoints = reflectionOrder + 1;
        lineRenderer[rayNumber].SetVertexCount(nPoints);
        lineRenderer[rayNumber].SetPosition(0, goTransform.position);
        int paramIndex;

        for (int i = 0; i < reflectionOrder; i++)
        {
            paramIndex = reflectionOffset + rayNumber * VAS_MAXREFLECTIONORDER * 3 + i * 3;

            if (Physics.Raycast(ray.origin, ray.direction, out hit, 1000))
            {
                inDirection = Vector3.Reflect(ray.direction, hit.normal);
                ray = new Ray(hit.point, inDirection);
                lineRenderer[rayNumber].SetPosition(i + 1, hit.point);

                mySource.SetSpatializerFloat(paramIndex, hit.point.x);
                mySource.SetSpatializerFloat(paramIndex+1, hit.point.y);
                mySource.SetSpatializerFloat(paramIndex+2, hit.point.z);
            }
        }
    }

    void Start()
    {
        spatId = spatIdCounter++;
        Debug.Log("SpatId: " + spatId);
        mySource.SetSpatializerFloat(3, (float)spatId);
        rayDistance = horizontalFullPowerRange / numberOfRays;

        mySource.SetSpatializerFloat((int)SpatParams.P_H_SOURCEDIRECTIVITY, horizontalSourceDirectivity);
        mySource.SetSpatializerFloat((int)SpatParams.P_H_FULLPOWERRANGE, horizontalFullPowerRange);
        mySource.SetSpatializerFloat((int)SpatParams.P_NUMBEROFRAYS, numberOfRays);
        mySource.SetSpatializerFloat((int)SpatParams.P_REFLECTIONORDER, reflectionOrder);

        IntPtr VAS_Unity_Spatializer = GetInstance(spatId);

        if (VAS_Unity_Spatializer != IntPtr.Zero)
        {
            SetConfig(VAS_Unity_Spatializer, (int)VAS_CONFIG.AUTO);
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

        for (int i = 0; i < numberOfRays; i++)
        {
            RaytraceReflections(i, i * rayDistance, 0);
        }

        CalculateDirectivityScaling();
        mySource.SetSpatializerFloat((int)SpatParams.P_DIRECTIVITYDAMPING, directivityScaling);
    }
}
