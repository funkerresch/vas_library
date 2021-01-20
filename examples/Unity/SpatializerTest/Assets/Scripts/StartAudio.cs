using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class StartAudio : MonoBehaviour
{
    public AudioSource source1;
    public AudioSource source2;
    public AudioSource source3;
    public AudioSource source4;

    // Start is called before the first frame update
    void Start()
    {
        double startTime = AudioSettings.dspTime + 10.0f;
        source1.PlayScheduled(startTime);
        source2.PlayScheduled(startTime);
        source3.PlayScheduled(startTime);
        source4.PlayScheduled(startTime);
    }
}
