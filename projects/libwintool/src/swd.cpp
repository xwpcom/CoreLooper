#include "stdafx.h"
#include "swd.h"
using namespace Bear::Core;

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

	#define ITEM(fn,api) fn  = (api)GetProcAddress(hModule, # api);if(!fn){LogW(TAG,"no found %s",# api);}
	#include "swd.api.inl"
	#undef ITEM

	//ITEM(mApi.getDLLVersion, JLINK_GetDLLVersion);
	//ITEM(mApi.getHardwareVersion, JLINK_GetHardwareVersion);

	if (mApi.getDLLVersion)
	{
		auto dllVer = mApi.getDLLVersion();
		LogV(TAG, "dllVer=0x%04x", dllVer);
	}

	if (mApi.getHardwareVersion)
	{
		auto hwVer = mApi.getHardwareVersion();
		LogV(TAG, "hwVer=0x%04x", hwVer);
	}
	
	int x = 0;
	/*
	JLINK_GetHardwareVersion = (void*)GetProcAddress(hModule, "JLINK_GetHardwareVersion");
	JLINK_GetFirmwareString = (void*)GetProcAddress(hModule, "JLINK_GetFirmwareString");
	JLINK_GetSN = (void*)GetProcAddress(hModule, "JLINK_GetSN");
	JLINK_GetId = (void*)GetProcAddress(hModule, "JLINK_GetId");
	JLINK_GetDeviceFamily = (void*)GetProcAddress(hModule, "JLINK_GetDeviceFamily");

	JLINK_SetLogFile = (void*)GetProcAddress(hModule, "JLINK_SetLogFile");
	JLINK_ExecCommand = (void*)GetProcAddress(hModule, "JLINK_ExecCommand");

	JLINK_GetSpeed = (void*)GetProcAddress(hModule, "JLINK_GetSpeed");
	JLINK_SetSpeed = (void*)GetProcAddress(hModule, "JLINK_SetSpeed");
	JLINK_TIF_Select = (void*)GetProcAddress(hModule, "JLINK_TIF_Select");

	JLINK_Open = (void*)GetProcAddress(hModule, "JLINK_Open");
	JLINK_Close = (void*)GetProcAddress(hModule, "JLINK_Close");
	JLINK_IsOpen = (void*)GetProcAddress(hModule, "JLINK_IsOpen");

	JLINK_Connect = (void*)GetProcAddress(hModule, "JLINK_Connect");
	JLINK_IsConnected = (void*)GetProcAddress(hModule, "JLINK_IsConnected");
	JLINK_SetResetType = (void*)GetProcAddress(hModule, "JLINK_SetResetType");
	JLINK_Reset = (void*)GetProcAddress(hModule, "JLINK_Reset");
	JLINK_Halt = (void*)GetProcAddress(hModule, "JLINK_Halt");
	JLINK_IsHalted = (void*)GetProcAddress(hModule, "JLINK_IsHalted");
	JLINK_Go = (void*)GetProcAddress(hModule, "JLINK_Go");
	JLINK_GoIntDis = (void*)GetProcAddress(hModule, "JLINK_GoIntDis");

	JLINK_ReadReg = (void*)GetProcAddress(hModule, "JLINK_ReadReg");
	JLINK_WriteReg = (void*)GetProcAddress(hModule, "JLINK_WriteReg");

	JLINK_ReadMem = (void*)GetProcAddress(hModule, "JLINK_ReadMem");
	JLINK_WriteMem = (void*)GetProcAddress(hModule, "JLINK_WriteMem");
	JLINK_WriteU8 = (void*)GetProcAddress(hModule, "JLINK_WriteU8");
	JLINK_WriteU16 = (void*)GetProcAddress(hModule, "JLINK_WriteU16");
	JLINK_WriteU32 = (void*)GetProcAddress(hModule, "JLINK_WriteU32");

	JLINK_EraseChip = (void*)GetProcAddress(hModule, "JLINK_EraseChip");
	JLINK_DownloadFile = (void*)GetProcAddress(hModule, "JLINK_DownloadFile");
	JLINK_BeginDownload = (void*)GetProcAddress(hModule, "JLINK_BeginDownload");
	JLINK_EndDownload = (void*)GetProcAddress(hModule, "JLINK_EndDownload");
	*/

	return 0;
}

int Swd::init()
{
	return initApi();
}
