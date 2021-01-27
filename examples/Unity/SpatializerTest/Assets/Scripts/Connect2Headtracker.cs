using UnityEngine;

public class Connect2Headtracker : MonoBehaviour
{
    public string headtrackerID = "rwaht00";
    public bool tare = false;
    public bool autocConnectHeadtracker = false;
    // Start is called before the first frame update
    void Start()
    {
        VasHeadtrackerConnect.SetHeadtrackerID(headtrackerID);
    }

    private void Update()
    {
#if (!UNITY_ANDROID && !UNITY_EDITOR_WIN)
        if (autocConnectHeadtracker)
        {
            int azi = VasHeadtrackerConnect.GetAzimuth();
            int ele = VasHeadtrackerConnect.GetElevation();
            Vector3 rotation = Camera.main.transform.rotation.eulerAngles;
            rotation.y = azi;
            rotation.x = ele;
            Camera.main.transform.rotation = Quaternion.Euler(rotation);
        }
#endif

    }

    void OnValidate()    {        if (tare)
        {
            VasHeadtrackerConnect.Tare();
            tare = false;
        }    }
}
