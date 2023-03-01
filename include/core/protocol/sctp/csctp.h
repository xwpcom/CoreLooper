//c51不支持#pragma once

#ifndef _CTP_H
#define _CTP_H

#include "cbytebuffer.h"
#include "cbundle.h"

/*
XiongWanPing 2018.12.22
Secure Common Text Protocol,安全的通用文本协议
是在原有ctp协议基础上增加了crc校验，用来支持串口不可靠传输
文档见:sim868.mcu协议.docx
文档位置D:\bear\project\sim868\SDK_1418B01SIM868M32_BT_EAT\doc

要支持如下场景
.sim868和平台之间的tcp连接
.sim868和mcu单片机的串口连接
由于mcu只支持c语言，不支持c++,所以采用c语言来实现
mcu配置很低,不能用alloc类函数,只能用静态buffer

sctp保留字段:
cmd
crc
seq
上层不能使用这些字段
crc计算时采用累积字段和对应的数据,各字段顺序无强制要求，生成和校验crc依次进行即可

*/
typedef struct tagSCTP
{
	tagByteBuffer	mInbox;
	tagByteBuffer	mOutbox;
	tagBundle		mInboxBundle;
	tagBundle		mOutboxBundle;
	unsigned short	mSeq;
	unsigned char	mDisableSeq : 1;
	unsigned char	m485Verbose: 1;//增加模式,过滤485总线上的杂乱数据 //
#ifndef __C51__
	//c51上不支持成员函数指针，会报error C212: indirect call: parameters do not fit within registers
	//所以采用强制转换
	void			*mOnRecvCommandCB;
	void			*mOnErrorCB;
#endif

}tagSCTP;

//common
void SCTP_Create(tagSCTP* obj);
void SCTP_clear(tagSCTP *obj);
void SCTP_Enable485Verbose(tagSCTP* obj);

//receiver专用,parser
int  SCTP_InputString(tagSCTP *obj, char *text);
int  SCTP_InputData(tagSCTP *obj, unsigned char *text, unsigned short bytes);
int  SCTP_CheckCrc(tagBundle *bundle,unsigned short* crcAck);
int SCTP_IsReservedKey(const char* name);

//sender专用,creater
int SCTP_CreateOutboxData(tagSCTP *obj);


#endif
