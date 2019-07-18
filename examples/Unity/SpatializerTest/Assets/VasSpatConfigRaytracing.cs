using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.IO;
using uOSC;

public class VasSpatConfigRaytracing : MonoBehaviour
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

        P_REF1_X, //9
        P_REF1_Y,
        P_REF1_Z,

        P_REF2_X, //12
        P_REF2_Y,
        P_REF2_Z,

        P_REF3_X,
        P_REF3_Y,
        P_REF3_Z,

        P_REF4_X,
        P_REF4_Y,
        P_REF4_Z,

        P_REF5_X,
        P_REF5_Y,
        P_REF5_Z,

        P_REF6_X,
        P_REF6_Y,
        P_REF6_Z,

        P_OCCLUSION_FREQ,
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
    public bool global;
    public int numberOfRays;
    public float horizontalSourceDirectivity = 180;
    public float horizontalFullPowerRange = 120;
    public float verticalSourceDirectivity = 180;
    public float verticalFullPowerRange = 120;
    private GameObject[] reflection;
    private LineRenderer[] lineRenderer;
    private Transform goTransform;
    private int numberOfReflections;

    //a ray
    private Ray ray;
    //a RaycastHit variable, to gather informartion about the ray's collision
    private RaycastHit hit;

    //reflection direction
    private Vector3 inDirection;

    //the number of reflections
    public int nReflections = 2;

    //the number of points at the line renderer
    private int nPoints;


#if UNITY_IPHONE
   
    [DllImport ("__Internal")]
    private static extern IntPtr GetInstance(int id);

    [DllImport ("__Internal")]
    private static extern void LoadHRTF(IntPtr x, string str);

    // [DllImport ("__Internal")]
    // private static extern void SetNumberOfReflections(IntPtr x, int numberOfReflections);

#else
    [DllImport("AudioPlugin_VAS_Binaural", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr GetInstance(int id);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    private static extern void LoadHRTF(IntPtr x, string str);

    [DllImport("AudioPlugin_VAS_Binaural", BestFitMapping = true, CallingConvention = CallingConvention.Cdecl)]
    private static extern void SetNumberOfReflections(IntPtr x, int numberOfReflections);

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

        lineRenderer = new LineRenderer[24];
        reflection = new GameObject[24];
        goTransform = this.GetComponent<Transform>();
        for (int i = 0; i < 24; i++)
        {
            reflection[i] = GameObject.CreatePrimitive(PrimitiveType.Sphere);
            reflection[i].transform.localScale = new Vector3(0.5f, 0.5f, 0.5f);
            lineRenderer[i] = reflection[i].AddComponent<LineRenderer>();
            lineRenderer[i].SetColors(Color.red, Color.blue);
            // lineRenderer.material = new Material(Shader.Find("Particles/Additive"));
            lineRenderer[i].SetWidth(0.1f, 0.1f);
        }
    }

    void updateReflections()
    {
        int offsetReflections = (int)SpatParams.P_REF1_X;

        for (int i = 0; i < numberOfReflections; i++)
            mySource.SetSpatializerFloat(offsetReflections + i, (float)reflection[i].transform.position.x);
    }

    void RaytraceReflections(int rayNumber, float azimuth, float elevation)
    {
        nReflections = Mathf.Clamp(nReflections, 1, nReflections);
        azimuth = Mathf.Deg2Rad * azimuth;
        elevation = Mathf.Deg2Rad * elevation;

        float x = Mathf.Sin(azimuth) * Mathf.Cos(elevation);
        float y = Mathf.Cos(azimuth) * Mathf.Cos(elevation);
        float z = Mathf.Sin(elevation);

        Vector3 rayDirection = new Vector3(x, z, y);
        rayDirection = transform.TransformDirection(rayDirection);
        ray = new Ray(goTransform.position, rayDirection);

        Debug.DrawRay(goTransform.position, rayDirection * 100, Color.magenta);

        nPoints = nReflections;
        lineRenderer[rayNumber].SetVertexCount(nPoints);
        lineRenderer[rayNumber].SetPosition(0, goTransform.position);

        for (int i = 0; i <= nReflections; i++)
        {
            //If the ray hasn't reflected yet
            if (i == 0)
            {
                //Check if the ray has hit something
                if (Physics.Raycast(ray.origin, ray.direction, out hit, 100))//cast the ray 100 units at the specified direction  
                {
                    //the reflection direction is the reflection of the current ray direction flipped at the hit normal
                    inDirection = Vector3.Reflect(ray.direction, hit.normal);
                    //cast the reflected ray, using the hit point as the origin and the reflected direction as the direction
                    ray = new Ray(hit.point, inDirection);

                    //Draw the normal - only can be seen at the Scene tab for debugging purposes
                    Debug.DrawRay(hit.point, hit.normal * 3, Color.blue);
                    //represent the ray using a line that can only be viewed at the scene tab
                    Debug.DrawRay(hit.point, inDirection * 100, Color.magenta);

                    //Print the name of the object the cast ray has hit, at the console
                    Debug.Log("Object name: " + hit.transform.name);

                    //if the number of reflections is set to 1
                    if (nReflections == 1)
                    {
                        //add a new vertex to the line renderer
                        lineRenderer[rayNumber].SetVertexCount(++nPoints);
                    }

                    //set the position of the next vertex at the line renderer to be the same as the hit point
                    lineRenderer[rayNumber].SetPosition(i + 1, hit.point);
                }
            }
            else // the ray has reflected at least once
            {
                //Check if the ray has hit something
                if (Physics.Raycast(ray.origin, ray.direction, out hit, 100))//cast the ray 100 units at the specified direction  
                {
                    //the refletion direction is the reflection of the ray's direction at the hit normal
                    inDirection = Vector3.Reflect(inDirection, hit.normal);
                    //cast the reflected ray, using the hit point as the origin and the reflected direction as the direction
                    ray = new Ray(hit.point, inDirection);

                    //Draw the normal - only can be seen at the Scene tab for debugging purposes
                    Debug.DrawRay(hit.point, hit.normal * 3, Color.blue);
                    //represent the ray using a line that can only be viewed at the scene tab
                    Debug.DrawRay(hit.point, inDirection * 100, Color.magenta);

                    //Print the name of the object the cast ray has hit, at the console
                    Debug.Log("Object name: " + hit.transform.name);

                    //add a new vertex to the line renderer
                    lineRenderer[rayNumber].SetVertexCount(++nPoints);
                    //set the position of the next vertex at the line renderer to be the same as the hit point
                    lineRenderer[rayNumber].SetPosition(i + 1, hit.point);
                }
            }
        }
    }

    void Start()
    {
        for (int i = 0 ;i < 24; i++)
             reflection[i] = GameObject.CreatePrimitive(PrimitiveType.Sphere);

        spatId = spatIdCounter++;
        Debug.Log("SpatId: " + spatId);
        mySource.SetSpatializerFloat(3, (float)spatId);

        mySource.SetSpatializerFloat((int)SpatParams.P_H_SOURCEDIRECTIVITY, horizontalSourceDirectivity);
        mySource.SetSpatializerFloat((int)SpatParams.P_H_FULLPOWERRANGE, horizontalFullPowerRange);

        for (int i = 0; i < numberOfRays; i++)
        {
            RaytraceReflections(i, i * 20, 0);
        }

        updateReflections();

        IntPtr VAS_Unity_Spatializer = GetInstance(spatId);

        if (VAS_Unity_Spatializer != IntPtr.Zero)
        {
            String fullpath = Path.Combine(Application.streamingAssetsPath, IrSet);
            Debug.Log(fullpath);
          //  SetNumberOfReflections(VAS_Unity_Spatializer, 24);
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
            RaytraceReflections(i, i * 20, 0);
        }

        updateReflections();



        float test;
        mySource.GetSpatializerFloat((int)SpatParams.P_TEST, out test);
        Debug.Log(test);

       //RaycastHit hit;

        // Does the ray intersect any objects excluding the player layer
      /*  if (autoPositionReflections)
        {
            if (Physics.Raycast(transform.position, transform.TransformDirection(new Vector3(0, -0.3f, -1)), out hit, Mathf.Infinity))
            {

                Debug.Log("Did Hit " + hit.point.x);
                reflection1.transform.position = hit.point;


            }
            else
            {
                Debug.Log("Did not Hit");
            }
        }*/
    }

    private void OnValidate()
    {
        if (global && globalIrSet == "" && IrSet != "")
            globalIrSet = IrSet;

        Debug.Log(globalIrSet);
    }

}