package com.tool;

import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

import android.util.Log;

//XiongWanPing 2013.05.14
public class TextProtocol 
{
	static String TAG="Bear/TextProtocol";
	protected Map<String,String> mMap =new HashMap<String,String>();
	
	public int parse(String param)
	{
		//Log.i(TAG,param);
		if(param==null)
		{
			return 0;
		}
		
		StringTokenizer lineParser=new StringTokenizer(param,"\r\n");
		 while (lineParser.hasMoreTokens())
		 {
		     String line=lineParser.nextToken();
		     int pos=line.indexOf('=',0);
		     if(pos!=-1)
		     {
		    	 String key=line.substring(0,pos);
		    	 String value=line.substring(pos+1);
		    	 mMap.put(key, value);
		     }
		 }
	
		return 0;
	}
	
	public boolean isExists(String key)
	{
		String value=mMap.get(key);
		return value!=null;
	}
	
	public String getString(String key)
	{
		String value=mMap.get(key);
		if(value==null)
		{
			return "";
		}
		
		return value;
	}
	
	public boolean getBool(String key)
	{
		return (getInt(key)==1);
	}
	public int getInt(String key)
	{
		int nValue=0;
		String value=getString(key);
		if(value.length()>0)
		{
			nValue=Integer.parseInt(value);
		}
		
		return nValue;
	}
	
	public static void test() 
	{
		/*
		TextProtocol tp=new TextProtocol();
		tp.Parse("addr=a\r\nport=80\r\n");
		Log.d("", "addr="+tp.GetString("addr"));
		Log.d("", "port="+tp.GetString("port"));
		int x=0;
		x=1;
		int y=x;
		//*/
	}
	

}
