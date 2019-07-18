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
    public int numberOfRays = 1;
    public int nReflections = 4;
    public Material raycastMaterial;

    GameObject[] lineRenderObject;


    //the number of points at the line renderer
    private int nPoints;
	

	void Awake () 
	{
		//get the attached Transform component
		goTransform = this.GetComponent<Transform>();
        //get the attached LineRenderer component
        lineRenderer = new LineRenderer[6];
        lineRenderObject = new GameObject[6];

        for (int i = 0; i < 6; i++)
        {
            lineRenderObject[i] = new GameObject("MyGameObject");
            lineRenderer[i] = lineRenderObject[i].AddComponent<LineRenderer>();

            lineRenderer[i].SetColors(Color.red, Color.blue);
            lineRenderer[i].material = raycastMaterial;
            
            lineRenderer[i].SetWidth(0.1f, 0.1f);
        }

        for (int i = 0; i < numberOfRays; i++)
        {
            RaytraceReflections(i, i * 20, 0);
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

      //  Debug.DrawRay(goTransform.position, rayDirection * 100, Color.white);

        nPoints = nReflections;
        //make the lineRenderer have nPoints
        lineRenderer[rayNumber].SetVertexCount(nPoints);
        //Set the first point of the line at the current attached game object position
        lineRenderer[rayNumber].SetPosition(0, goTransform.position);

        for (int i = 0; i < nReflections; i++)
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
                  //  Debug.DrawRay(hit.point, hit.normal * 3, Color.white);
                    //represent the ray using a line that can only be viewed at the scene tab
                  //  Debug.DrawRay(hit.point, inDirection * 100, Color.white);

                    //Print the name of the object the cast ray has hit, at the console
                    //Debug.Log("Object name: " + hit.transform.name);

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
                  //  Debug.DrawRay(hit.point, hit.normal * 3, Color.white);
                    //represent the ray using a line that can only be viewed at the scene tab
                  //  Debug.DrawRay(hit.point, inDirection * 100, Color.white);

                    //Print the name of the object the cast ray has hit, at the console
                   // Debug.Log("Object name: " + hit.transform.name);

                    //add a new vertex to the line renderer
                    lineRenderer[rayNumber].SetVertexCount(++nPoints);
                    //set the position of the next vertex at the line renderer to be the same as the hit point
                    lineRenderer[rayNumber].SetPosition(i + 1, hit.point);
                }
            }
        }
    }

    void Update()
    {
        for(int i = 0; i< 4; i++)
        {
            RaytraceReflections(i, i * 20, 0);
        }
    }
}
