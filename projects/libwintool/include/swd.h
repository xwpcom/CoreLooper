#pragma once

//JLINK TIF
#define JLINK_TIF_JTAG          0
#define JLINK_TIF_SWD           1

//RESET TYPE
#define JLINK_RESET_TYPE_NORMAL 0
#define JLINK_RESET_TYPE_CORE   1
#define JLINK_RESET_TYPE_PIN    2

/*
Functions which are described as JLINKARM_* are also implemented and exported as JLINK_*
pdf文档中用的是JLINKARM_前缀,文档:pdfcoffee.com_um08002jlinkdllpdf-4-pdf-free
增加一个接口的流程
.根据pdf中添加函数原型,然后在swd.api.inl中增加映射即可
以JLINK_SetLogFile为例:
添加如下函数原型
typedef void  (WINAPI* JLINK_SetLogFile)(const char* file);
在swd.api.inl中添加ITEM(SetLogFile)即可

*/
typedef void  (WINAPI* JLINK_SetLogFile)(const char* file);
typedef DWORD(WINAPI* JLINK_GetDLLVersion)(void);
typedef DWORD(WINAPI* JLINK_GetHardwareVersion)(void);
typedef DWORD(WINAPI* JLINK_GetFirmwareString)(char* buff, int count);
typedef DWORD(WINAPI* JLINK_GetSN)(void);

typedef BOOL(WINAPI* JLINK_ExecCommand)(const char* cmd, int a, int b);
typedef void  (WINAPI* JLINK_TIF_GetAvailable)(int32_t* mask);
typedef DWORD(WINAPI* JLINK_TIF_Select)(int type);
typedef void  (WINAPI* JLINK_SetSpeed)(int speed);
typedef DWORD(WINAPI* JLINK_GetSpeed)(void);
typedef uint32_t (WINAPI* JLINK_GetId)(void);
typedef DWORD(WINAPI* JLINK_GetDeviceFamily)(void);

typedef void (*JLINKARM_LOG)(const char*);

typedef int32_t(*JLINK_EMU_GetNumDevices)();//Gets the number of emulators which are connected via USB to the PC

typedef const char* (WINAPI* JLINK_OpenEx)(JLINKARM_LOG* pfLog, JLINKARM_LOG* pfErrorOut);
typedef int (WINAPI* JLINK_Open)(void);//网上说返回BOOL是不对的,成功打开时返回的是0
typedef void (WINAPI* JLINK_Close)(void);
typedef char (WINAPI* JLINK_IsOpen)(void);
typedef BOOL(WINAPI* JLINK_Connect)(void);
typedef BOOL(WINAPI* JLINK_IsConnected)(void);
typedef int   (WINAPI* JLINK_Halt)(void);
typedef BOOL(WINAPI* JLINK_IsHalted)(void);
typedef void  (WINAPI* JLINK_ResetNoHalt)(void);
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
typedef int   (WINAPI* JLINK_EraseChip)(void);//>=0 ok,<0 fail
typedef int   (WINAPI* JLINK_DownloadFile)(LPCSTR file, DWORD addr);
typedef void  (WINAPI* JLINK_BeginDownload)(int32_t reservedFlags);
typedef int   (WINAPI* JLINK_EndDownload)();
typedef void  (WINAPI* JLINK_EMU_GetProductName)(char* pBuffer, uint32_t BufferSize);
struct tagJLink
{
	tagJLink()
	{
		memset(this, 0, sizeof(*this));
	}

	#define ITEM(fn) JLINK_ ## fn fn;
	#include "swd.api.inl"
	#undef ITEM
};

/*
XiongWanPing 2024.01.27
文档:pdfcoffee.com_um08002jlinkdllpdf-4-pdf-free

https://blog.csdn.net/carrymen/article/details/133995472
https://blog.csdn.net/qq446252221/article/details/89878996
*/
class WIN_CLASS Swd
{
public:
	int init();

	tagJLink JLink;
protected:
	int initApi();

	string mTag = "swd";
};
