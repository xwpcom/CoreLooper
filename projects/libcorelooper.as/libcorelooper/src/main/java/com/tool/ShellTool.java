package com.tool;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.os.Process;
import android.os.SystemClock;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class ShellTool
{
	static String TAG="Bear/ShellTool";
	
	public static void sleep(int ms)
	{
		try
		{
			Thread.sleep(ms);
		}
		catch (Exception e)
		{
			Log.w(TAG,e.toString());
		}
	}
	
	public static long getTickCount()
	{
		long ms= SystemClock.uptimeMillis();
		return ms;
	}
	
	public static int getLimitValue(int min,int v,int max)
	{
		if(v<min)
		{
			return min;
		}
		if(v>max)
		{
			return max;
		}
		return v;
	}
	
	public static boolean isJJYMTK6572()
	{
		String str=getLinuxCore_Version();
		if(str!=null)
		{
			return str.indexOf("zxht")!=-1;
		}
		return false;
	}
	
	public static String getLinuxCore_Version()
	{
		java.lang.Process process = null;
		try
		{
			process = Runtime.getRuntime().exec("cat /proc/version");
		}
		catch (IOException e)
		{
			Log.w(TAG,e.toString());
			e.printStackTrace();
		}
		
		// get the output line
		InputStream outs = process.getInputStream();
		InputStreamReader isrout = new InputStreamReader(outs);
		BufferedReader brout = new BufferedReader(isrout, 8 * 1024);
		
		String result = "";
		String line;
		// get the whole standard output string
		try
		{
			while ((line = brout.readLine()) != null)
			{
				//Log.w(TAG,line);
				result += line;
			}
		}
		catch (IOException e)
		{
			Log.w(TAG,e.toString());
			e.printStackTrace();
		}
		
		//Log.w(TAG,"linuxKernel="+result);
		
		return result;
	}
	
	public static int getPid()
	{
		return Process.myPid();
	}
	
	public static int getTid()
	{
		return Process.myTid();
	}

	public static boolean isDebugMode(Context context)
	{
		try {
			ApplicationInfo info= context.getApplicationInfo();
			return (info.flags & ApplicationInfo.FLAG_DEBUGGABLE)!=0;
		} catch (Exception e) {
			
		}
		return false;
	}
	
};
