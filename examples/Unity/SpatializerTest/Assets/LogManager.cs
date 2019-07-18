using UnityEngine;
using System.Runtime.InteropServices;

public static class LogManager 
{
	public static void Init()
	{
		Application.logMessageReceived += Log;
	}

	static void Log(string condition, string stackTrace, LogType type )
	{
		var message = new System.Text.StringBuilder();
		message.Append(System.Environment.NewLine);
		message.Append(type);
		message.Append( "\t" );
		message.Append(condition);
		message.Append(System.Environment.NewLine);
		message.Append(stackTrace);

		Print(message.ToString());
	}


#if UNITY_IPHONE
	
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