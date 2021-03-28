using System.Collections;
using System.Collections.Generic;
using UnityEngine.Events;
using UnityEngine;

public class TriggerZones : MonoBehaviour
{
    UnityEvent ciceroOnly;
    UnityEvent quartettOnly;
    UnityEvent quartettAndCicero;
    public TriggerZoneReceiver triggerZoneReceiver;

    void Start()
    {
        if (ciceroOnly == null)
            ciceroOnly = new UnityEvent();
        if (quartettOnly == null)
            quartettOnly = new UnityEvent();
        if (quartettAndCicero == null)
            quartettAndCicero = new UnityEvent();

        ciceroOnly.AddListener(triggerZoneReceiver.CiceroOnly);
        quartettOnly.AddListener(triggerZoneReceiver.QuartettOnly);
        quartettAndCicero.AddListener(triggerZoneReceiver.QuartettAndCicero);
    }

    private void OnTriggerEnter(Collider other)
    {
        if (other.gameObject.tag == "ciceroonly")
        {
            ciceroOnly.Invoke();
        }

        if (other.gameObject.tag == "quartettonly")
        {
            quartettOnly.Invoke();
        }

        if (other.gameObject.tag == "quartettandcicero")
        {
            quartettAndCicero.Invoke();
        }
    }
}
