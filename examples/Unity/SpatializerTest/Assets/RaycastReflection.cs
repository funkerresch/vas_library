///////////////////////////////////////////////////////////////////////
//                                                   41 Post                                       //
// Created by DimasTheDriver on June/28/2011                                    //
// Part of 'Unity: Raycast Reflection' post.                          		 	 //
// Available at:     http://www.41post.com/?p=4162                              //
/////////////////////////////////////////////////////////////////////

using UnityEngine;
using System.Collections;

public class RaycastReflection : MonoBehaviour 
{
	//this game object's Transform
	private Transform goTransform;
	//the attached line renderer
	private LineRenderer[] lineRenderer;
	
	//a ray
	private Ray ray;
	//a RaycastHit variable, to gather informartion about the ray's collision
	private RaycastHit hit;
	
	//reflection direction
	private Vector3 inDirection;

    //the number of reflections
    public int numberOfRays = 5;
    public int nReflections = 4;
    public Material raycastMaterial;

    GameObject[] lineRenderObject;
    private int nPoints;

	void Awake () 
	{
		goTransform = this.GetComponent<Transform>();
        lineRenderer = new LineRenderer[numberOfRays];
        lineRenderObject = new GameObject[numberOfRays];

        for (int i = 0; i < numberOfRays; i++)
        {
            lineRenderObject[i] = new GameObject("MyGameObject");
            lineRenderer[i] = lineRenderObject[i].AddComponent<LineRenderer>();
            lineRenderer[i].material = raycastMaterial;        
            lineRenderer[i].SetWidth(0.15f, 0.15f);
        }
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

        nPoints = nReflections+1;
        lineRenderer[rayNumber].SetVertexCount(nPoints);
        lineRenderer[rayNumber].SetPosition(0, goTransform.position);

        for (int i = 0; i < nReflections; i++)
        {
            if (i == 0)
            {
                if (Physics.Raycast(ray.origin, ray.direction, out hit, 100))//cast the ray 100 units at the specified direction  
                {
                    inDirection = Vector3.Reflect(ray.direction, hit.normal);
                    ray = new Ray(hit.point, inDirection);
                    lineRenderer[rayNumber].SetPosition(i + 1, hit.point);
                }
            }
            else 
            {
                if (Physics.Raycast(ray.origin, ray.direction, out hit, 100))//cast the ray 100 units at the specified direction  
                {
                    inDirection = Vector3.Reflect(inDirection, hit.normal);
                    ray = new Ray(hit.point, inDirection);
                    lineRenderer[rayNumber].SetPosition(i + 1, hit.point);
                }   
            }
        }
    }

    void Update()
    {
        for(int i = 0; i< numberOfRays; i++)
        {
            RaytraceReflections(i, i * 30, 0);
        }
    }
}
