#include "stdafx.h"
#include "include/swd.h"
using namespace Bear::Core;

/*

可用GetId来定时检测当前是否连接上jlink设备,能实时生效
当没插swd设备，或者拔出设备时,GetId马上返回0或-1
当插了设备时,GetId返回样本:GetId=731911287

*/

static const char* TAG = "swd";

int Swd::initApi()
{
	auto file = "JLinkARM.dll";
	auto hModule = LoadLibraryA(file);
	if (!hModule)
	{
		LogW(TAG, "no found:%s",file);
		return -1;
	}

	#define ITEM(fn) JLink.fn  = (JLINK_##fn)GetProcAddress(hModule, "JLINK_"#fn);if(!JLink.fn){LogW(TAG,"no found %s","JLINK_"#fn);return -1;}
	#include "include/swd.api.inl"
	#undef ITEM

	//JLink.SetLogFile((ShellTool::GetAppPath()+"/jlink.log").c_str());

	//auto dllVer = JLink.GetDLLVersion();
	//LogV(TAG, "dllVer=0x%04x", dllVer);

	return 0;
}

/*
C:\Program Files (x86)\SEGGER\JLink下面的
Devices
JLinkARM.dll
JLinkDevices.xml
要放在运行目录中
*/
int Swd::init()
{
	auto ret=initApi();
	if (ret) {
		return ret;
	}

#if 0
	auto ret = JLink.Open();
	LogV(mTag, "open ret=%d,isOpen=%d", ret, JLink.IsOpen());
	//ok=mJLink.Connect();
	//LogV(mTag, "connect ok=%d", ok);
	//auto hwVer = JLink.GetHardwareVersion();
	//LogV(TAG, "hwVer=0x%04x", hwVer);

	/*
	device是JLinkDevices.xml中对应的芯片Name
	比如我们二代地埋网关用的是HC32F460,在JLinkDevices.xml可找到<ChipInfo Vendor="HDSC" Name="HC32F46x" 
	*/
	auto device = "HC32F46x";
	auto cmd = string("device = ") + device;
	JLink.ExecCommand(cmd.c_str(), 0, 0);//这里不选芯片的话，jlink dll会自动弹出芯片选择框

	JLink.TIF_Select(JLINK_TIF_SWD);
	JLink.SetSpeed(4000); //speed,khz
	JLink.Connect();
	if (JLink.IsConnected()) {
		LogV(mTag, "is connected");
	}
	else 
	{
		LogW(mTag, "is NOT connected");
	}

	BYTE buf[64] = { 0 };
	int bytes = sizeof(buf);
	ret = JLink.ReadMem(0, bytes, buf);
	auto hex=ByteTool::ByteToHexChar(buf, bytes,"%02X",16);
	LogV(mTag, "readFlash#1");
	LogV(mTag, "%s",hex.c_str());
	/*
	LogV(mTag, "erase chip#begin");
	JLink.EraseChip();//test ok,速度很快,不占用时间,erase之后用J-Flash可看到所有数据都变成0xFF
	//不需要单独调用擦除,
	//BeginDownload+WriteMem+EndDownload就能写flash,它内部会自动处理擦除
	LogV(mTag, "erase chip#end");
	*/
	if (0)
	{
		for (int i = 0; i < sizeof(buf); i++)
		{
			buf[i] = (i % 0xFF)+0x20;
		}

		JLink.BeginDownload(0);
		LogV(mTag, "writeFlash");
		ret = JLink.WriteMem(0, bytes, buf);//本行只是写数据到dll缓存
		ret=JLink.EndDownload();//这里才是烧录到硬件flash
		LogV(mTag, "writeFlash ret=%d",ret);

		LogV(mTag, "readFlash");
		memset(buf, 0, sizeof(buf));
		ret = JLink.ReadMem(0, bytes, buf);
		hex = ByteTool::ByteToHexChar(buf, bytes, "%02X", 16);
		LogV(mTag, "%s", hex.c_str());
	}
#endif

	return 0;
}
