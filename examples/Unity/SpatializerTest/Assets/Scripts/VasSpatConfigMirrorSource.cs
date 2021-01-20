using UnityEngine;
using System.Collections.Generic;

public class MirrorSource
{
    private static VasHeadtrackerConnect _instance;
    public static VasHeadtrackerConnect Instance
    {
        get
        {
            if (_instance == null)
            {
                var obj = new GameObject("VasHeadtrackerConnect");
                _instance = obj.AddComponent<VasHeadtrackerConnect>();
            }
            return _instance;
        }
    }

    public Vector3 position;
    public int reflectionOrder = 0;
    public List<Material> materials;
    public GameObject visualization;

    public MirrorSource(Vector3 position, int reflectionOrder, List<Material> materials)
    {
        this.reflectionOrder = reflectionOrder;
        this.position = new Vector3(position.x, position.y, position.z);
        if (materials != null)
            this.materials = new List<Material>(materials);
        else
            this.materials = new List<Material>();
    }
}

public class VasSpatConfigMirrorSource : VasSpat{
    private Ray ray;
    private RaycastHit hit;
    private Vector3 inDirection;
    private GameObject[] walls;
    private List<MirrorSource> reflections;
    public string wallTag;
    public bool visualizeMirrorSources = false;
    public int reflectionOrder = 4;
    public int numberOfRays = 5;

    private Transform goTransform;
    private LineRenderer[] lineRenderer;
    private Vector3 lastReflection;
    private float lastDistance;
    private GameObject[] lineRenderObject;
    private int nPoints;
    public Material raycastMaterial;
    public Color rayColor;

    void RaytraceReflections(int rayNumber, float azimuth, float elevation)
    {
        int totalReflectionCount = 0;

        azimuth = Mathf.Deg2Rad * azimuth; 
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
        lineRenderer[rayNumber].startWidth = 0.05f;
        lineRenderer[rayNumber].endWidth = 0.05f;
        lastDistance = 0;
        lastReflection = mySource.transform.position;

        for (int i = 0; i < reflectionOrder; i++)
        {
            if (Physics.Raycast(ray.origin, ray.direction, out hit, 1000))
            {
                inDirection = Vector3.Reflect(ray.direction, hit.normal);
                ray = new Ray(hit.point, inDirection);
                lineRenderer[rayNumber].SetPosition(i + 1, hit.point);

                lastDistance += Vector3.Distance(lastReflection, hit.point);
                lastReflection = hit.point;
            }
          
            totalReflectionCount++;
        }
    }

    void CalculateImageSources(MirrorSource sourcePosition, GameObject lastWall, int order)
    {
        foreach (GameObject wall in walls)
        {
            if (wall != lastWall)
            {
                if (lastWall != null)
                    lastWall.gameObject.SetActive(false);

                Vector3 rayDirection = wall.transform.position - sourcePosition.position;
                rayDirection = transform.TransformDirection(rayDirection);
                ray = new Ray(sourcePosition.position, rayDirection);

                if (Physics.Raycast(ray.origin, ray.direction, out hit, 10000))
                {
                    inDirection = Vector3.Reflect(ray.direction, hit.normal);
                    Vector3 reverseDir = -inDirection * rayDirection.magnitude;
                    Vector3 endPoint = hit.point + (reverseDir.normalized * rayDirection.magnitude);
                    Material reflectionMaterial = hit.transform.gameObject.GetComponent<Renderer>().material;
                    List<Material> materials = new List<Material>(sourcePosition.materials);
                 
                    materials.Add(reflectionMaterial);
                    MirrorSource reflection = new MirrorSource(endPoint, order, materials);

                    if (visualizeMirrorSources)
                    {
                        GameObject visualization = GameObject.CreatePrimitive(PrimitiveType.Sphere);
                        visualization.transform.position = reflection.position;
                        visualization.hideFlags = HideFlags.HideInHierarchy;
                    }

                    reflections.Add(reflection);
                    if (lastWall != null)
                        lastWall.gameObject.SetActive(true);

                    if (order > 0)
                        CalculateImageSources(reflection, wall, order - 1);
                }
                else
                {
                    if (lastWall != null)
                        lastWall.gameObject.SetActive(true);
                }
            }
        }
    }    new void Awake()
    {
        base.Awake();
        goTransform = this.GetComponent<Transform>();
        if (lineRenderer == null)
        {
            lineRenderer = new LineRenderer[numberOfRays];
            lineRenderObject = new GameObject[numberOfRays];

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
    }    void RecalculateImageSources()    {        reflections.Clear();        if (reflectionOrder > 0)
        {
            List<Material> materials = new List<Material>();
            MirrorSource zero = new MirrorSource(this.gameObject.transform.position, 0, materials);
            CalculateImageSources(zero, null, reflectionOrder - 1);
        }    }    void Start()    {
        reflections = new List<MirrorSource>();
        walls = GameObject.FindGameObjectsWithTag(wallTag);
        curve = mySource.GetCustomCurve(AudioSourceCurveType.CustomRolloff);
        maxDistance = mySource.maxDistance;

        if (reflectionOrder > 0)
        {
            List<Material> materials = new List<Material>();
            MirrorSource zero = new MirrorSource(this.gameObject.transform.position, 0, materials);
            CalculateImageSources(zero, null, reflectionOrder - 1);
        }

        //RaytraceReflections(1, 90, 0);

        Debug.Log("Number of calculated Mirror Sources: " + reflections.Count);
        
        SetSpatializerInstanceAndConfig(VAS_CONFIG.AUTO);

        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_EARLYPART, segmentSizeEarlyReverb);
        mySource.SetSpatializerFloat((int)SpatParams.P_SEGMENTSIZE_LATEPART, segmentSizeLateReverb);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_DIRECT, scalingDirect);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_EARLY, scalingEarly);
        mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_LATE, scalingLate);
        mySource.SetSpatializerFloat((int)SpatParams.P_BINAURALREFLECTIONS, binauralReflectionCount);
        mySource.SetSpatializerFloat((int)SpatParams.P_NUMBEROFRAYS, 0);
        mySource.SetSpatializerFloat((int)SpatParams.P_REFLECTIONORDER, reflections.Count);

        InverseAzimuth(inverseAzimuth);        InverseElevation(inverseElevation);        BypassSpat(bypass);        ListenerOrientationOnly(listenerOrientationOnly);
        ReadImpulseResponse();
    }    new void Update()
    {
        int i = 0;
#if (!UNITY_ANDROID && !UNITY_EDITOR_WIN)
        int azi = VasHeadtrackerConnect.GetAzimuth();
        int ele = VasHeadtrackerConnect.GetElevation();
        Vector3 rotation = Camera.main.transform.rotation.eulerAngles;
        rotation.y = azi;
        rotation.x = ele;
        Camera.main.transform.rotation = Quaternion.Euler(rotation);
#endif
        
        foreach (MirrorSource s in reflections)
        {
            float dist = Vector3.Distance(s.position, listener1.transform.position);
            float OneOverScaledDistance = 1f / maxDistance * dist;
            float scale = curve.Evaluate(OneOverScaledDistance);

            mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_DIRECT, scalingDirect);
            mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_EARLY, scalingEarly);
            mySource.SetSpatializerFloat((int)SpatParams.P_SCALING_LATE, scalingLate);
            mySource.SetSpatializerFloat((int)SpatParams.P_BINAURALREFLECTIONS, binauralReflectionCount);

            SetReflectionParameter(VAS_Unity_Spatializer, i, (int)ReflectionParams.P_REF_X, s.position.x);
            SetReflectionParameter(VAS_Unity_Spatializer, i, (int)ReflectionParams.P_REF_Y, s.position.y);
            SetReflectionParameter(VAS_Unity_Spatializer, i, (int)ReflectionParams.P_REF_Z, s.position.z);
            SetReflectionParameter(VAS_Unity_Spatializer, i, (int)ReflectionParams.P_REF_SCALE, scale);
            SetReflectionParameter(VAS_Unity_Spatializer, i, (int)ReflectionParams.P_REF_DIST, dist);
            SetReflectionParameter(VAS_Unity_Spatializer, i, (int)ReflectionParams.P_REF_MAT, 2f);

            i++;
        } 
    }    new void OnValidate()    {        base.OnValidate();        if (recalculate)
        {
            RecalculateImageSources();
            recalculate = false;
        }    }}