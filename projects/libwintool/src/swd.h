#pragma once


//JLINK TIF
#define JLINK_TIF_JTAG          0
#define JLINK_TIF_SWD           1

//RESET TYPE
#define JLINK_RESET_TYPE_NORMAL 0
#define JLINK_RESET_TYPE_CORE   1
#define JLINK_RESET_TYPE_PIN    2

//REGISTER INDEX
/*
  0 - 15     R0 - R15(SP=R13, PC=R15)
 16          XPSR
 17          MSP
 18          PSP
 19          RAZ
 20          CFBP
 21          APSR
 22          EPSR
 23          IPSR
 24          PRIMASK
 25          BASEPRI
 26          FAULTMASK
 27          CONTROL
 28          BASEPRI_MAX
 29          IAPSR
 30          EAPSR
 31          IEPSR
 */

typedef void  (WINAPI* JLINK_SetLogFile)(char* file);
typedef DWORD(WINAPI* JLINK_GetDLLVersion)(void);
typedef DWORD(WINAPI* JLINK_GetDLLVersion2)(void);
typedef DWORD(WINAPI* JLINK_GetHardwareVersion)(void);
typedef DWORD(WINAPI* JLINK_GetFirmwareString)(char* buff, int count);
typedef DWORD(WINAPI* JLINK_GetSN)(void);

typedef BOOL(WINAPI* JLINK_ExecCommand)(char* cmd, int a, int b);
typedef DWORD(WINAPI* JLINK_TIF_Select)(int type);
typedef void  (WINAPI* JLINK_SetSpeed)(int speed);
typedef DWORD(WINAPI* JLINK_GetSpeed)(void);
typedef DWORD(WINAPI* JLINK_GetId)(void);
typedef DWORD(WINAPI* JLINK_GetDeviceFamily)(void);

typedef BOOL(WINAPI* JLINK_Open)(void);
typedef void  (WINAPI* JLINK_Close)(void);
typedef BOOL(WINAPI* JLINK_IsOpen)(void);

typedef BOOL(WINAPI* JLINK_Connect)(void);
typedef BOOL(WINAPI* JLINK_IsConnected)(void);
typedef int   (WINAPI* JLINK_Halt)(void);
typedef BOOL(WINAPI* JLINK_IsHalted)(void);
typedef void  (WINAPI* JLINK_SetResetType)(int type);
typedef void  (WINAPI* JLINK_Reset)(void);
typedef void  (WINAPI* JLINK_Go)(void);
typedef void  (WINAPI* JLINK_GoIntDis)(void);
typedef DWORD(WINAPI* JLINK_ReadReg)(int index);
typedef int   (WINAPI* JLINK_WriteReg)(int index, DWORD data);

typedef int   (WINAPI* JLINK_ReadMem)(DWORD addr, int len, void* buf);
typedef int   (WINAPI* JLINK_WriteMem)(DWORD addr, int len, void* buf);
typedef int   (WINAPI* JLINK_WriteU8)(DWORD addr, BYTE data);
typedef int   (WINAPI* JLINK_WriteU16)(DWORD addr, WORD data);
typedef int   (WINAPI* JLINK_WriteU32)(DWORD addr, DWORD data);

typedef int   (WINAPI* JLINK_EraseChip)(void);
typedef int   (WINAPI* JLINK_DownloadFile)(LPCSTR file, DWORD addr);
typedef void  (WINAPI* JLINK_BeginDownload)(int index);

class Swd
{
public:
	int init();

	int initApi();
	struct tagApi
	{
		tagApi()
		{
			memset(this, 0, sizeof(*this));
		}

		#include "swd.api.inl"
		JLINK_SetLogFile			setLogFile;
		JLINK_GetDLLVersion			getDLLVersion;
		JLINK_GetHardwareVersion	getHardwareVersion;
		JLINK_GetFirmwareString		getFirmwareString;
		JLINK_GetSN					getSn;

		/*
		typedef BOOL(WINAPI* JLINK_ExecCommand)(char* cmd, int a, int b);
		typedef DWORD(WINAPI* JLINK_TIF_Select)(int type);
		typedef void  (WINAPI* JLINK_SetSpeed)(int speed);
		typedef DWORD(WINAPI* JLINK_GetSpeed)(void);
		typedef DWORD(WINAPI* JLINK_GetId)(void);
		typedef DWORD(WINAPI* JLINK_GetDeviceFamily)(void);

		typedef BOOL(WINAPI* JLINK_Open)(void);
		typedef void  (WINAPI* JLINK_Close)(void);
		typedef BOOL(WINAPI* JLINK_IsOpen)(void);

		typedef BOOL(WINAPI* JLINK_Connect)(void);
		typedef BOOL(WINAPI* JLINK_IsConnected)(void);
		typedef int   (WINAPI* JLINK_Halt)(void);
		typedef BOOL(WINAPI* JLINK_IsHalted)(void);
		typedef void  (WINAPI* JLINK_SetResetType)(int type);
		typedef void  (WINAPI* JLINK_Reset)(void);
		typedef void  (WINAPI* JLINK_Go)(void);
		typedef void  (WINAPI* JLINK_GoIntDis)(void);
		typedef DWORD(WINAPI* JLINK_ReadReg)(int index);
		typedef int   (WINAPI* JLINK_WriteReg)(int index, DWORD data);

		typedef int   (WINAPI* JLINK_ReadMem)(DWORD addr, int len, void* buf);
		typedef int   (WINAPI* JLINK_WriteMem)(DWORD addr, int len, void* buf);
		typedef int   (WINAPI* JLINK_WriteU8)(DWORD addr, BYTE data);
		typedef int   (WINAPI* JLINK_WriteU16)(DWORD addr, WORD data);
		typedef int   (WINAPI* JLINK_WriteU32)(DWORD addr, DWORD data);

		typedef int   (WINAPI* JLINK_EraseChip)(void);
		typedef int   (WINAPI* JLINK_DownloadFile)(LPCSTR file, DWORD addr);
		typedef void  (WINAPI* JLINK_BeginDownload)(int index);
		*/
	}mApi;
};
