using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class VasHeadtrackerConnect : MonoBehaviour
{
    #region Declare external C interface    
#if UNITY_IOS && !UNITY_EDITOR

    [DllImport ("__Internal")]
    public static extern int _getAzimuth();

    [DllImport ("__Internal")]
    public static extern int _getElevation();

    [DllImport ("__Internal")]
    public static extern void _setHeadtrackerId(string str);

    [DllImport ("__Internal")]
    public static extern void _tare();

#elif (UNITY_ANDROID && !UNITY_EDITOR_OSX) || UNITY_EDITOR_WIN // Headtracker Connect not implemented for Android

    public static int _getAzimuth() { return 0; }

    public static int _getElevation() { return 0; }

    public static void _setHeadtrackerId(string str) { }

    public static void _tare() { }
#else
    [DllImport("VAS_HeadtrackerConnect_Plugin", CallingConvention = CallingConvention.Cdecl)]
    public static extern int _getAzimuth();

    [DllImport("VAS_HeadtrackerConnect_Plugin", CallingConvention = CallingConvention.Cdecl)]
    public static extern int _getElevation();

    [DllImport("VAS_HeadtrackerConnect_Plugin", CallingConvention = CallingConvention.Cdecl)]
    public static extern void _setHeadtrackerId(string str);

    [DllImport("VAS_HeadtrackerConnect_Plugin", CallingConvention = CallingConvention.Cdecl)]
    public static extern void _tare();

#endif
    #endregion

    #region Wrapped methods and properties
    public static int GetAzimuth()
    {
       return _getAzimuth();
    }

    public static int GetElevation()
    {
        return _getElevation();
    }

    public static void Tare()
    {
        _tare();
    }

    public static void SetHeadtrackerID(string name)
    {
       _setHeadtrackerId(name);
    }
    #endregion
}