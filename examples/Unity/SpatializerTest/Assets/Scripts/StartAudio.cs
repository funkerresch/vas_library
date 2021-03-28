using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class StartAudio : MonoBehaviour
{
    public GameObject sources1;
    public GameObject sources2;

    // Start is called before the first frame update
    void Start()
    {
        double startTime = AudioSettings.dspTime + 5.0f;

        AudioSource[] childScripts;
        childScripts = sources1.GetComponentsInChildren<AudioSource>();
        foreach (AudioSource s in childScripts)
            s.PlayScheduled(startTime);

        childScripts = sources2.GetComponentsInChildren<AudioSource>();
        foreach (AudioSource s in childScripts)
            s.PlayScheduled(startTime);
    }
}
