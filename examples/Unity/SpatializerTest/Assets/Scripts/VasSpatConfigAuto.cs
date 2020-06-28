using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
using System.IO;
using UnityEditor;

[Serializable]
public struct RayControl
{
    public bool[] onOff;
}

[Serializable]
public struct ReflectionControl
{
    public RayControl[] ray;

    public ReflectionControl(int numberOfRays, int reflectionOrder)
    {
        ray = new RayControl[numberOfRays];
        for (int i = 0; i < numberOfRays; i++) {
            ray[i].onOff = new bool[reflectionOrder];
            for (int j = 0; j < reflectionOrder; j++) {
                ray[i].onOff[j] = false;
            }
        }
    }
}

public class VasSpatConfigAuto : VasSpat
{
    [Range(0.0f, 12.0f)]
    public int numberOfRays = 6;
    [Range(1.0f, 12.0f)]
    public int reflectionOrder = 1;

    public float horizontalSourceDirectivity = 180;
    private float rayDistance = 30;
    public float horizontalFullPowerRange = 120;

    public float verticalSourceDirectivity = 90;
    private float vRayDistance = 30;
    public float verticalFullPowerRange = 60;

    [SerializeField]
    public ReflectionControl reflControl;
    public int maxReflectionCount = 0;

    private float[] spectrum = new float[1024];

    public Material raycastMaterial;
    public Color rayColor;

    private Transform goTransform;
    private LineRenderer[] lineRenderer;
    private Ray ray;
    private RaycastHit hit;
    private Vector3 inDirection;
    private Vector3 lastReflection;
    private float lastDistance;
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

    private float verticalDirectivityOver2;
    private float verticalFullEnergyOver2;
    private float top;
    private float bottom;
    private float topFull;
    private float bottomFull;

    private float hDiff;
    private float vDiff;
    private float s2lAzimuth; // for directivity
    private float s2lElevation;
    private float s2lFullSphericElevation;

    private float hDirectivityScaling;
    private float vDirectivityScaling;

    private float[] listenerMatrix;
    private float[] sourceMatrix;

    void updateRayColors()
    {
        if (!mySource)
            return;

        for (int i = 0; i < numberOfRays; i++)
            lineRenderer[i].material.SetColor("_EmissionColor", rayColor);
    }

    void OnValidate()
    {
        base.OnValidate();
        BypassSpat(bypass);
        ListenerOrientationOnly(listenerOrientationOnly);
        InverseAzimuth(inverseAzimuth);        InverseElevation(inverseElevation);
        updateRayColors();
        Debug.Log("Validate");
    }

    void CalculateHDirectivityScaling()
    {
        horizontalDirectivityOver2 = horizontalSourceDirectivity * 0.5f;
        horizontalFullEnergyOver2 = horizontalFullPowerRange * 0.5f;
        left = 360 - horizontalDirectivityOver2;
        right = horizontalDirectivityOver2;
        leftFull = 360 - horizontalFullEnergyOver2;
        rightFull = horizontalFullEnergyOver2;
        hDiff = leftFull - left;
        //CalculateAzimuth();
        //CalculateElevation();

        if (s2lAzimuth <= rightFull || s2lAzimuth >= leftFull)
            hDirectivityScaling = 1.0f;

        else if (s2lAzimuth > rightFull && s2lAzimuth <= right)
        {
            hDirectivityScaling = (right - s2lAzimuth) / hDiff;
        }

        else if (s2lAzimuth < leftFull && s2lAzimuth >= left)
        {
            hDirectivityScaling = Mathf.Abs(left - s2lAzimuth) / hDiff;
        }
        else
        {
            hDirectivityScaling = 0.0f;
        }

        hDirectivityScaling = ((Mathf.Pow(10f, hDirectivityScaling))-1f) /9f;
    }

    void CalculateVDirectivityScaling()
    {
        verticalDirectivityOver2 = verticalSourceDirectivity * 0.5f;
        verticalFullEnergyOver2 = verticalFullPowerRange * 0.5f;
        top = 360 - horizontalDirectivityOver2;
        bottom = horizontalDirectivityOver2;
        topFull = 360 - verticalFullEnergyOver2;
        bottomFull = verticalFullEnergyOver2;
        vDiff = topFull - top;
        //CalculateFullSphericElevation();

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

        //Debug.Log("scale: " + vDirectivityScaling);
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

    void CalculateSource2ListenerAziAndEle()
    {
        var listenerMatrix = listenerTransform.localToWorldMatrix;
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

        float azimuth1 = (Mathf.Abs(dir_z) < 0.001f) ? 0.0f : Mathf.Atan2(dir_x, dir_z);
        if (azimuth1 < 0.0f)
            azimuth1 += 2.0f * kPI;

        azimuth1 = Mathf.Clamp(azimuth1 * kRad2Deg, 0.0f, 360.0f);
        s2lAzimuth = azimuth1;

        float ele1 = (Mathf.Abs(dir_z) < 0.001f) ? 0.0f : Mathf.Atan2(dir_y, dir_z);
        if(ele1 < 0.0f)
            ele1 += 2.0f * kPI;

        ele1 = Mathf.Clamp(ele1 * kRad2Deg, 0.0f, 360.0f);
        s2lFullSphericElevation = ele1;                
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
    }

    void MuteReflections(int rayNumber, int reflectionNumber)
    {
        int paramIndex = reflectionOffset + rayNumber * VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS + reflectionNumber * VAS_REFLECTIONPARAMETERS;
        bool value = reflControl.ray[rayNumber].onOff[reflectionNumber];
        float fval = (float)Convert.ToInt32(value);
        mySource.SetSpatializerFloat(paramIndex + 4, fval);
    }

    void RaytraceReflections(int rayNumber, float azimuth, float elevation)
    {
        int totalReflectionCount = 0;

        azimuth = Mathf.Deg2Rad * (azimuth- (horizontalFullPowerRange*0.5f));
        elevation = Mathf.Deg2Rad * elevation;

        float x = Mathf.Sin(azimuth) * Mathf.Cos(elevation);
        float y = Mathf.Cos(azimuth) * Mathf.Cos(elevation);
        float z = Mathf.Sin(elevation);

        Vector3 rayDirection = new Vector3(x, z, y);
        rayDirection = transform.TransformDirection(rayDirection);
        ray = new Ray(goTransform.position, rayDirection);

        nPoints = reflectionOrder + 1;
        // lineRenderer[rayNumber].SetVertexCount(nPoints);
        lineRenderer[rayNumber].positionCount = nPoints;
        lineRenderer[rayNumber].SetPosition(0, goTransform.position);
        lineRenderer[rayNumber].startWidth = 0.01f;
        lineRenderer[rayNumber].endWidth = 0.01f;
        int paramIndex;
        lastDistance = 0;
        lastReflection = mySource.transform.position;
        var curve = mySource.GetCustomCurve(AudioSourceCurveType.CustomRolloff);
        float maxDistance = mySource.maxDistance;
        float sourceListenerDistance = Vector3.Distance(mySource.transform.position, listener.transform.position);
        sourceListenerDistance = 1f / maxDistance * sourceListenerDistance;
        float inverseSourceListenerScale = 1f/curve.Evaluate(sourceListenerDistance);

        for (int i = 0; i < reflectionOrder; i++)
        {   
            paramIndex = reflectionOffset + rayNumber * VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS + i * VAS_REFLECTIONPARAMETERS;
            
            if (Physics.Raycast(ray.origin, ray.direction, out hit, 1000))
            {
                //Debug.Log("NO MUTE");
                MuteReflections(rayNumber, i);
                inDirection = Vector3.Reflect(ray.direction, hit.normal);
                ray = new Ray(hit.point, inDirection);
                lineRenderer[rayNumber].SetPosition(i + 1, hit.point);

                lastDistance += Vector3.Distance(lastReflection, hit.point);
                lastReflection = hit.point;
                
                float dist = 1f / maxDistance * lastDistance;
                float scale = curve.Evaluate(dist) * inverseSourceListenerScale;
                //Debug.Log("Reflection Number: " + i);
                //Debug.Log("Distance: " + lastDistance);
                //Debug.Log("Scale: " + scale);

                mySource.SetSpatializerFloat(paramIndex, hit.point.x);
                mySource.SetSpatializerFloat(paramIndex + 1, hit.point.y);
                mySource.SetSpatializerFloat(paramIndex + 2, hit.point.z);

                if (hit.transform.tag == "CONCRETE")
                    mySource.SetSpatializerFloat(paramIndex + 3, 1f);
                else if (hit.transform.tag == "MIXED")
                    mySource.SetSpatializerFloat(paramIndex + 3, 2f);
                else if (hit.transform.tag == "WOOD")
                    mySource.SetSpatializerFloat(paramIndex + 3, 3f);
                else if (hit.transform.tag == "TEXTILE")
                    mySource.SetSpatializerFloat(paramIndex + 3, 4f);
                else
                    mySource.SetSpatializerFloat(paramIndex + 3, 1f);

                mySource.SetSpatializerFloat(paramIndex + 5, scale);
                

                mySource.SetSpatializerFloat(paramIndex + 6, lastDistance);
                Debug.Log("Scale: " + i + " " + scale);
                Debug.Log("Dist: " + i + " " + lastDistance);
                //Debug.Log(scale);
            }
            else
            {  
                mySource.SetSpatializerFloat(paramIndex + 4, 1f);
            }
            totalReflectionCount++;
            
        }
    }

   

    void Awake()
    {
        spatIdCounter = 0;
#if UNITY_IPHONE
        ;
#else
        //DebugDelegate callback_delegate = new DebugDelegate(DebugCallback);
        //IntPtr intptr_delegate = Marshal.GetFunctionPointerForDelegate(callback_delegate);
        //SetDebugFunction(intptr_delegate);
#endif

        if (Application.isPlaying)
        {   
            reflControl = new ReflectionControl(numberOfRays, reflectionOrder);

            mySource = GetComponent<AudioSource>();
            goTransform = this.GetComponent<Transform>();
            if (lineRenderer == null)
            {
                lineRenderer = new LineRenderer[numberOfRays];
                lineRenderObject = new GameObject[numberOfRays];
                listener = GameObject.FindObjectOfType<AudioListener>();
                listenerTransform = listener.GetComponent<Transform>();

                for (int i = 0; i < numberOfRays; i++)
                {
                    lineRenderObject[i] = new GameObject("MyGameObject");
                    lineRenderObject[i].hideFlags = HideFlags.HideInHierarchy;
                    lineRenderer[i] = lineRenderObject[i].AddComponent<LineRenderer>();
                    lineRenderer[i].sharedMaterial = raycastMaterial;
                    lineRenderer[i].startWidth = 0.3f;
                    lineRenderer[i].endWidth = 0.3f;
                    lineRenderer[i].shadowCastingMode = 0;
                    lineRenderer[i].sharedMaterial.SetColor("_EmissionColor", rayColor);
                }
            }
        }
    }

    void Start()
    {
        if (!Application.isPlaying)
            return;

        spatId = spatIdCounter++;
        Debug.Log("SpatId: " + spatId);

        listenerMatrix = new float[16];
        sourceMatrix = new float[16];

        CalculateSource2ListenerAziAndEle();
        mySource.SetSpatializerFloat(3, (float)spatId);
        rayDistance = horizontalFullPowerRange / numberOfRays;

        mySource.SetSpatializerFloat((int)SpatParams.P_H_SOURCEDIRECTIVITY, horizontalSourceDirectivity);
        mySource.SetSpatializerFloat((int)SpatParams.P_H_FULLPOWERRANGE, horizontalFullPowerRange);
        mySource.SetSpatializerFloat((int)SpatParams.P_NUMBEROFRAYS, numberOfRays);
        mySource.SetSpatializerFloat((int)SpatParams.P_REFLECTIONORDER, reflectionOrder);
        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_EARLYPART, segmentSizeEarlyReverb);
        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_LATEPART, segmentSizeLateReverb);
        InverseAzimuth(inverseAzimuth);
        InverseElevation(inverseElevation);

        VAS_Unity_Spatializer = GetInstance(spatId);
        Debug.Log(spatId);

        if (VAS_Unity_Spatializer != IntPtr.Zero)
        {
            SetConfig(VAS_Unity_Spatializer, (int)VAS_CONFIG.AUTO);
            String fullpath = Path.Combine(Application.streamingAssetsPath, IrSet);
            Debug.Log(fullpath);
            LoadHRTF(VAS_Unity_Spatializer, fullpath);
            BypassSpat(bypass);
        }
        else
            print("No Renderer Reference");
    }

    void Update()
    {
        if (Application.isPlaying)
        {
            CalculateSource2ListenerAziAndEle();
            mySource.SetSpatializerFloat((int)SpatParams.P_H_SOURCEDIRECTIVITY, horizontalSourceDirectivity);
            mySource.SetSpatializerFloat((int)SpatParams.P_H_FULLPOWERRANGE, horizontalFullPowerRange);
            mySource.SetSpatializerFloat((int)SpatParams.P_NUMBEROFRAYS, numberOfRays);
            mySource.SetSpatializerFloat((int)SpatParams.P_REFLECTIONORDER, reflectionOrder);
            //mySource.GetSpectrumData(spectrum, 0, FFTWindow.Rectangular);
            CalculateSource2ListenerAziAndEle();
            CalculateHDirectivityScaling();
            CalculateVDirectivityScaling();
            mySource.SetSpatializerFloat((int)SpatParams.P_H_DIRECTIVITYDAMPING, hDirectivityScaling);
            mySource.SetSpatializerFloat((int)SpatParams.P_V_DIRECTIVITYDAMPING, vDirectivityScaling);
            for (int i = 0; i < numberOfRays; i++)
            {
                RaytraceReflections(i, i * rayDistance, 0);
            }
        }
    }
}
