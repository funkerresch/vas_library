
using System.Collections.Generic;
using UnityEngine;

public class Reflect : MonoBehaviour
{
    public int reflectionOrder = 1;
    private Ray ray;
    private RaycastHit hit;
    private Vector3 inDirection;
    private GameObject[] walls;
    private List<GameObject> reflections;
    public string wallTag;

    void CalculateImageSources(GameObject source, GameObject lastWall, int order)
    {
        foreach (GameObject wall in walls)
        {
            if (wall != lastWall)
            {
                if (lastWall != null)
                    lastWall.gameObject.SetActive(false);

                Vector3 rayDirection = wall.transform.position - source.transform.position;
                rayDirection = transform.TransformDirection(rayDirection);
                ray = new Ray(source.transform.position, rayDirection);

                if (Physics.Raycast(ray.origin, ray.direction, out hit, 1000))
                {
                    inDirection = Vector3.Reflect(ray.direction, hit.normal);
                    Vector3 reverseDir = -inDirection * rayDirection.magnitude;
                    Vector3 endPoint = hit.point + (reverseDir.normalized * rayDirection.magnitude);
                    GameObject reflection = GameObject.CreatePrimitive(PrimitiveType.Sphere);
                    reflections.Add(reflection);
                    reflection.hideFlags = HideFlags.HideInHierarchy;
                    reflection.transform.position = endPoint;
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
    }

    void Start()
    {
        reflections = new List<GameObject>();
        walls = GameObject.FindGameObjectsWithTag(wallTag);
        if (reflectionOrder > 0)
            CalculateImageSources(this.gameObject, null, reflectionOrder - 1);
    }

    void Update()
    {
       /* foreach (GameObject g in reflections)
            Destroy(g);

        reflections.Clear();*/

        
    }
}
