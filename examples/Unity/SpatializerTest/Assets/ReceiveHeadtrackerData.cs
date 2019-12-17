using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using uOSC;

public class ReceiveHeadtrackerData : MonoBehaviour
{

    int[] movingAverage = new int[10000];
    int counter = 0;
    int azimuth = 0;

    float azimuthOffset = 0;
    float elevationOffset = 0;

    public bool calibrate = false;

    void OnValidate()
    {
        azimuthOffset = azimuth;
        calibrate = false;
    }

    void OnDataReceived(Message message)
    {
        if (message.address == "/azi")
        {
            azimuth = (int)message.values.GetValue(0);

            Vector3 newRotation = transform.rotation.eulerAngles;
            newRotation.y =  azimuth-azimuthOffset;
            transform.eulerAngles = newRotation;
            print(azimuth);
           // transform.rotation.
        }

        if (message.address == "/ele")
        {
            message.values.GetValue(0);
        }

        if (message.address == "/time")
        {
           
            int sendTime = (int)message.values.GetValue(0);
           // Debug.Log("SEND TIMESTAMP: " + sendTime);

            System.DateTime epochStart = new System.DateTime(1970, 1, 1, 0, 0, 0, System.DateTimeKind.Utc);
            long cur_time = (long)(System.DateTime.UtcNow - epochStart).TotalMilliseconds;
            cur_time -= 1559393916351;
            //Debug.Log(cur_time);

            int elapsedTime = (int)cur_time - sendTime;
            Debug.Log("elapsed TIMESTAMP: " + elapsedTime);

            counter++;

            if(counter >= 500 && counter < 10500 )
            {
                movingAverage[counter - 500] = elapsedTime;

             
            }
            if(counter >= 10500)
            { 
                
            }




        }
    }

    // Start is called before the first frame update
    void Start()
    {
        var server = GetComponent<uOscServer>();
        server.onDataReceived.AddListener(OnDataReceived);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
