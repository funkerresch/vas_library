using UnityEngine.Audio;
using UnityEngine;
using System.Collections;

public class TriggerZoneReceiver : MonoBehaviour
{
    public AudioMixer zoneMixer;

    public AudioSource quartett1;
    public AudioSource quartett2;
    public AudioSource quartett3;
    public AudioSource quartett4;

    public AudioSource cicero;

    IEnumerator WaitAndMuteCicero(float seconds)
    {
        yield return new WaitForSeconds(seconds);
        cicero.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 1f);
    }

    IEnumerator WaitAndMuteQuartett(float seconds)
    {
        yield return new WaitForSeconds(seconds);
        quartett1.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 1f);
        quartett2.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 1f);
        quartett3.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 1f);
        quartett4.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 1f);
    }

    public void CiceroOnly()
    {
        zoneMixer.FindSnapshot("ciceroonly").TransitionTo(5f);
        StartCoroutine(WaitAndMuteQuartett(5f));
        Debug.Log("Cicero");
    }

    public void QuartettOnly()
    {
        zoneMixer.FindSnapshot("quartettonly").TransitionTo(5f);
        StartCoroutine(WaitAndMuteCicero(5f));
        Debug.Log("Quartett");
    }

    public void QuartettAndCicero()
    {
        zoneMixer.FindSnapshot("quartettandcicero").TransitionTo(3f);
        quartett1.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 0f);
        quartett2.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 0f);
        quartett3.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 0f);
        quartett4.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 0f);
        cicero.SetSpatializerFloat((int)VasSpat.SpatParams.P_BYPASS, 0f);
        Debug.Log("Both");
    }
}

