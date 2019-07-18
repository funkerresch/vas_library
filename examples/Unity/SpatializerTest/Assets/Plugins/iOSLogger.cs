using System.Runtime.InteropServices;
using AOT;


public class iOSLogger 
{

#if UNITY_IPHONE && !UNITY_EDITOR

	[DllImport ("__Internal")]
	static extern void _log(string log);

	public static void Print(string message)
	{
		_log(message);
	}
#else
	public static void Print(string message)
	{
	}
#endif

}
