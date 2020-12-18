using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.Networking;
using System.Threading;

using System;
using System.IO;
using UnityEditor;

public class VasSpatConfigManual : VasSpat
{
    public bool autoPositionReflections;
    public float horizontalSourceDirectivity = 180;
    public float horizontalFullPowerRange = 120;
    public float verticalSourceDirectivity = 90;
    public float verticalFullPowerRange = 60;

    public GameObject reflection1;
    public GameObject reflection2;
    public GameObject reflection3;
    public GameObject reflection4;
    public GameObject reflection5;
    public GameObject reflection6;
    public GameObject reflection7;
    public GameObject reflection8;

    [Range(0.0f, 12.0f)]
    public int numberOfRays = 6;
    [Range(1.0f, 12.0f)]
    public int reflectionOrder = 1;

    private float[] spectrum = new float[1024];

    public Material raycastMaterial;
    public Color rayColor;

    private Transform goTransform;
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

    private LineRenderer[] lineRenderer;
    private Ray ray;
    private RaycastHit hit;
    private Vector3 inDirection;
    private Vector3 lastReflection;
    private float lastDistance;
    private GameObject[] lineRenderObject;
    private int nPoints;

    void RaytraceReflections(int rayNumber, float azimuth, float elevation)
    {
        int totalReflectionCount = 0;

        azimuth = Mathf.Deg2Rad * (azimuth - (horizontalFullPowerRange * 0.5f));
        elevation = Mathf.Deg2Rad * elevation;

        float x = Mathf.Sin(azimuth) * Mathf.Cos(elevation);
        float y = Mathf.Cos(azimuth) * Mathf.Cos(elevation);
        float z = Mathf.Sin(elevation);

        Vector3 rayDirection = new Vector3(x, z, y);
        rayDirection = transform.TransformDirection(rayDirection);
        ray = new Ray(goTransform.position, rayDirection);

        nPoints = reflectionOrder + 1;
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
        float inverseSourceListenerScale = 1f / curve.Evaluate(sourceListenerDistance);

        for (int i = 0; i < reflectionOrder; i++)
        {
            paramIndex = reflectionOffset + rayNumber * VAS_MAXREFLECTIONORDER * VAS_REFLECTIONPARAMETERS + i * VAS_REFLECTIONPARAMETERS;

            if (Physics.Raycast(ray.origin, ray.direction, out hit, 1000))
            {
                //Debug.Log("NO MUTE");
                inDirection = Vector3.Reflect(ray.direction, hit.normal);
                ray = new Ray(hit.point, inDirection);
                lineRenderer[rayNumber].SetPosition(i + 1, hit.point);

                lastDistance += Vector3.Distance(lastReflection, hit.point);
                lastReflection = hit.point;

                float dist = 1f / maxDistance * lastDistance;
                float scale = curve.Evaluate(dist) * inverseSourceListenerScale;

                //Debug.Log("Scale: " + i + " " + scale);
                //Debug.Log("Dist: " + i + " " + lastDistance);
                //Debug.Log(scale);
            }
            totalReflectionCount++;
        }
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

        hDirectivityScaling = ((Mathf.Pow(10f, hDirectivityScaling)) - 1f) / 9f;
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
        if (ele1 < 0.0f)
            ele1 += 2.0f * kPI;

        ele1 = Mathf.Clamp(ele1 * kRad2Deg, 0.0f, 360.0f);
        s2lFullSphericElevation = ele1;
    }

    void CalculateElevation()
    {
        var heading = goTransform.position - listenerTransform.position;
        float elevation = Mathf.Atan2(heading.y, Mathf.Sqrt(heading.x * heading.x + heading.z * heading.z)) * Mathf.Rad2Deg;
        var angle = goTransform.rotation.eulerAngles;
        float rotationX = angle.x;
        rotationX %= 360;

        if (rotationX <= 180)
            elevation -= rotationX;
        else
            elevation += (360 - rotationX);

        s2lElevation = elevation;
    }

    new void Awake()
    {
        base.Awake();
        goTransform = this.GetComponent<Transform>();
        listener = GameObject.FindObjectOfType<AudioListener>();
        listenerTransform = listener.GetComponent<Transform>();
    }   

    void Start()
    {
        SetSpatializerInstanceAndConfig(VAS_CONFIG.AUTO);

        listenerMatrix = new float[16];
        sourceMatrix = new float[16];

        mySource.SetSpatializerFloat((int)SpatParams.P_H_SOURCEDIRECTIVITY, horizontalSourceDirectivity);
        mySource.SetSpatializerFloat((int)SpatParams.P_H_FULLPOWERRANGE, horizontalFullPowerRange);
        mySource.SetSpatializerFloat((int)SpatParams.P_NUMBEROFRAYS, numberOfRays);
        mySource.SetSpatializerFloat((int)SpatParams.P_REFLECTIONORDER, reflectionOrder);
        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_EARLYPART, segmentSizeEarlyReverb);
        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_LATEPART, segmentSizeLateReverb);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_EARLY, scalingEarly);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_LATE, scalingLate);
        InverseAzimuth(inverseAzimuth);        InverseElevation(inverseElevation);        BypassSpat(bypass);        ListenerOrientationOnly(listenerOrientationOnly);        ReadImpulseResponse();
    }

    new void Update()
    {
        base.Update();

        CalculateSource2ListenerAziAndEle();
        mySource.SetSpatializerFloat((int)SpatParams.P_H_SOURCEDIRECTIVITY, horizontalSourceDirectivity);
        mySource.SetSpatializerFloat((int)SpatParams.P_H_FULLPOWERRANGE, horizontalFullPowerRange);
        mySource.SetSpatializerFloat((int)SpatParams.P_NUMBEROFRAYS, numberOfRays);
        mySource.SetSpatializerFloat((int)SpatParams.P_REFLECTIONORDER, reflectionOrder);
        //mySource.GetSpectrumData(spectrum, 0, FFTWindow.Rectangular);
        CalculateHDirectivityScaling();
        CalculateVDirectivityScaling();
        mySource.SetSpatializerFloat((int)SpatParams.P_H_DIRECTIVITYDAMPING, hDirectivityScaling);
        mySource.SetSpatializerFloat((int)SpatParams.P_V_DIRECTIVITYDAMPING, vDirectivityScaling);

        float sourceListenerDistance1 = Vector3.Distance(reflection1.transform.position, listener.transform.position);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_1_1_X, (float)reflection1.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_1_1_Y, (float)reflection1.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_1_1_Z, (float)reflection1.transform.position.z);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_1_1_MAT, (float)1f);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_1_1_SCALE, (float)0.9f);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_1_1_DIST, (float)sourceListenerDistance1);

        float sourceListenerDistance2 = Vector3.Distance(reflection2.transform.position, listener.transform.position);
        //print(reflection2.transform.position);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_2_1_X, (float)reflection2.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_2_1_Y, (float)reflection2.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_2_1_Z, (float)reflection2.transform.position.z);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_2_1_MAT, (float)1f);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_2_1_SCALE, (float)0.9f);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_2_1_DIST, (float)sourceListenerDistance2);

        float sourceListenerDistance3 = Vector3.Distance(reflection3.transform.position, listener.transform.position);
        //print(reflection3.transform.position);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_3_1_X, (float)reflection3.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_3_1_Y, (float)reflection3.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_3_1_Z, (float)reflection3.transform.position.z);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_3_1_MAT, (float)1f);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_3_1_SCALE, (float)0.9f);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_3_1_DIST, (float)sourceListenerDistance3);

        // Debug.Log(sourceListenerDistance1 + " " + sourceListenerDistance2);

        float sourceListenerDistance4 = Vector3.Distance(reflection4.transform.position, listener.transform.position);
        //print(reflection3.transform.position);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_4_1_X, (float)reflection4.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_4_1_Y, (float)reflection4.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_4_1_Z, (float)reflection4.transform.position.z);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_4_1_MAT, (float)1f);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_4_1_SCALE, (float)0.8f);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_4_1_DIST, (float)sourceListenerDistance4);

        float sourceListenerDistance5 = Vector3.Distance(reflection5.transform.position, listener.transform.position);
        //print(reflection3.transform.position);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_5_1_X, (float)reflection5.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_5_1_Y, (float)reflection5.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_5_1_Z, (float)reflection5.transform.position.z);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_5_1_MAT, (float)1f);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_5_1_SCALE, (float)0.8f);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_5_1_DIST, (float)sourceListenerDistance5);

        float sourceListenerDistance6 = Vector3.Distance(reflection6.transform.position, listener.transform.position);
        //print(reflection3.transform.position);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_6_1_X, (float)reflection6.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_6_1_Y, (float)reflection6.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_6_1_Z, (float)reflection6.transform.position.z);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_6_1_MAT, (float)1f);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_6_1_SCALE, (float)0.8f);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_6_1_DIST, (float)sourceListenerDistance6);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_4_1_DIST, (float)sourceListenerDistance4);

        float sourceListenerDistance7 = Vector3.Distance(reflection7.transform.position, listener.transform.position);
        //print(reflection3.transform.position);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_7_1_X, (float)reflection7.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_7_1_Y, (float)reflection7.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_7_1_Z, (float)reflection7.transform.position.z);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_7_1_MAT, (float)1f);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_7_1_SCALE, (float)0.8f);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_7_1_DIST, (float)sourceListenerDistance7);

        float sourceListenerDistance8 = Vector3.Distance(reflection8.transform.position, listener.transform.position);
        //print(reflection3.transform.position);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_8_1_X, (float)reflection8.transform.position.x);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_8_1_Y, (float)reflection8.transform.position.y);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_8_1_Z, (float)reflection8.transform.position.z);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_8_1_MAT, (float)1f);
        mySource.SetSpatializerFloat((int)SpatParams.P_REF_8_1_SCALE, (float)0.8f);

        mySource.SetSpatializerFloat((int)SpatParams.P_REF_8_1_DIST, (float)sourceListenerDistance8);    
    }
}
