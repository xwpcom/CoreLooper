package com.tool;

import java.io.FileOutputStream;
import java.io.IOException;

import android.util.Log;

public class FileDump 
{
	public	int		mMaxBytes=0;
	public	String  mFileName;
	
	private FileOutputStream mOutput=null;
	private int mFileSize=0;
	static String TAG="Bear/FileDump";
	
	public FileDump(String filename,int maxBytes)
	{
		mMaxBytes=maxBytes;
		mFileName=filename;
		
		try
		{
			//Log.w(TAG,"start write file: "+mFileName);
			mOutput=new FileOutputStream(mFileName);
		} 
		catch (IOException e)
		{  
			e.printStackTrace();  
		}  				
	}
	
	public int write(byte[] datum)
	{
		return write(datum,datum.length);
	}

	public int write(byte[] datum,int bytes)
	{
		if(mOutput==null)
		{
			return 0;
		}
		
		try
		{
			int ret=bytes;
			mOutput.write(datum, 0, ret);
			mFileSize+=ret;
			
			if(mFileSize>=mMaxBytes)
			{
				mOutput.close();
				mOutput=null;
				//Log.w(TAG,"stop  write file: "+mFileName);
			}
		}
		catch(IOException e)
		{
			
		}
		
		return 0;
	}

	public void close()
	{
		try
		{
			if (mOutput != null)
			{
				mOutput.close();
				mOutput = null;
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			Log.w(TAG,e.toString());
		}
	}
}
