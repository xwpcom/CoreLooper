package com.tool;

import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;

public class FileTool {
	static String TAG="FileTool";
	
	public static String getFileExt(String filename)
	{
		String ext="";

		int nc=filename.length();
		for(int i=nc-1;i>0;i--)
		{
			char ch=filename.charAt(i);
			
			if(ch=='.')
			{
				ext=filename.substring(i);
			}
		}
		
		return ext;
	}

	public static long getFileBytes(String filePath)
	{
		try
		{
			File f=new File(filePath);
			if (f.exists() && f.isFile())
			{
				return f.length();
			}
		}
		catch (Exception e)
		{
			return -1;
		}

		return -1;
	}
	public static boolean fileExists(String filePath)
	{
		try
		{
			File f=new File(filePath);
			if(!f.exists())
			{
				return false;
			}

		}
		catch (Exception e)
		{
			return false;
		}

		return true;
	}
	
	/**
	 * 复制文件或文件夹
	 */
	static public boolean copyFolder(String src,String dest)
	{
		try
		{
			// 创建目标文件夹
			(new File(dest)).mkdirs();
			// 获取源文件夹当前下的文件或目录
			File[] file = (new File(src)).listFiles();
			for (int i = 0; i < file.length; i++)
			{
				if (file[i].isFile())
				{
					// 复制文件
					copyFile(file[i], new File(dest + File.separator + file[i].getName()));
				}
				else if (file[i].isDirectory())
				{
					// 复制目录
					String sourceDir = src + File.separator + file[i].getName();
					String targetDir = dest + File.separator + file[i].getName();
					boolean ok = copyFolder(sourceDir, targetDir);
					if(!ok)
					{
						Log.w(TAG, "fail to copyFolder");
						return false;
					}
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
	
	// 复制文件
	public static void copyFile(File sourceFile,File targetFile)throws IOException
	{
		// 新建文件输入流并对它进行缓冲
		FileInputStream input = new FileInputStream(sourceFile);
		BufferedInputStream inBuff=new BufferedInputStream(input);
		
		// 新建文件输出流并对它进行缓冲
		FileOutputStream output = new FileOutputStream(targetFile);
		BufferedOutputStream outBuff=new BufferedOutputStream(output);
		
		// 缓冲数组
		byte[] b = new byte[1024 * 16];
		int len;
		while ((len =inBuff.read(b)) != -1)
		{
			outBuff.write(b, 0, len);
		}
		// 刷新此缓冲的输出流
		outBuff.flush();
		
		//关闭流
		inBuff.close();
		outBuff.close();
		output.close();
		input.close();
	}
	
	static String getParentPath(String path)
	{
		int pos = path.lastIndexOf('/');
		if(pos!=-1)
		{
			return path.substring(0,pos);
		}
		
		return "";
	}
	
	public static boolean moveFolderFile(String oldPath, String newPath)
	{
		try
		{
			File srcFolder = new File(oldPath);
			File destFolder = new File(newPath);
			File newFile = new File(destFolder.getAbsoluteFile() + "/" + srcFolder.getName());
			
			return srcFolder.renameTo(newFile);
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		
		return false;
	}
	
	static public String loadTextFile(String filePath)
	{
		try
		{
			StringBuffer sb = new StringBuffer();
			File file = new File(filePath);
			BufferedReader br = new BufferedReader(new FileReader(file));
			String line = "";
			while ((line = br.readLine()) != null)
			{
				sb.append(line);
				sb.append("\r\n");
			}
			br.close();
			
			return sb.toString();
		}
		catch (Exception e)
		{
			Log.w(TAG,e.toString());
		}
		
		return "";
	}

	public static byte[] loadFile(String filePath)
	{
		try
		{
			File file = new File(filePath);
			InputStream in =  new FileInputStream(file);
			long bytes=FileTool.getFileBytes(filePath);
			if(bytes>0)
			{
				byte []buf=new byte[(int)bytes];
				in.read(buf);
				return buf;
			}
		}
		catch (Exception e)
		{
			Log.w(TAG,e.toString());
		}
		
		return null;
	}
}
