using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Connect2Headtracker : MonoBehaviour
{
    public string headtrackerID = "rwaht00";
    public bool tare = false;
    // Start is called before the first frame update
    void Start()
    {
        VasHeadtrackerConnect.SetHeadtrackerID(headtrackerID);
    }

    void OnValidate()    {        if (tare)
        {
            VasHeadtrackerConnect.Tare();
            tare = false;
        }    }
}
