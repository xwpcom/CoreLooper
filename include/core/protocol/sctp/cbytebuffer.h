#ifndef _BYTE_BUFFER_H
#define _BYTE_BUFFER_H

#include <string.h>

#ifndef COUNT_OF
#define COUNT_OF(x)	(int)((sizeof(x)/sizeof((x)[0])))
#endif

#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

//XiongWanPing 2018.12.22
//for stc,sim868 and windows
//stc15w4k32s4,SDRAM只有4KB,其中256 idata和3840 xdata,在mcu中已经是"豪华"版配置了!
//flash为32KB
typedef struct tagByteBuffer
{
	//注意:建议app只通过接口来使用ByteBuffer,不要直接使用下面的字段

	//单片机stack很小，并且不能用alloc类函数，所以mBuf交由app来分配
	//比如mcu环境下一般是xdata静态buffer
	unsigned char	*mBuf;
	unsigned short  mOffset;//有效数据起始偏移
	unsigned short	mBytes;
	unsigned short	mTotalBytes;//mBuf指向的buffer总字节数
}tagByteBuffer;

void ByteBuffer_Create(tagByteBuffer* obj);
void ByteBuffer_Init(struct tagByteBuffer* obj, unsigned char *buf, unsigned short totalBytes);

//建议不要直接调用Input,而是调用下面的WriteXXX函数
int ByteBuffer_Input(struct tagByteBuffer* obj, const unsigned char *text, unsigned short bytes);

int ByteBuffer_Write(struct tagByteBuffer* obj, const unsigned char *text, unsigned short bytes);
int ByteBuffer_WriteByte(struct tagByteBuffer* obj, unsigned char text);
int ByteBuffer_WriteString(struct tagByteBuffer* obj, const char *text);

unsigned short ByteBuffer_GetFreeBytes(struct tagByteBuffer* obj);
void ByteBuffer_MoveToHead(struct tagByteBuffer* obj);

unsigned short ByteBuffer_GetBytes(struct tagByteBuffer* obj);
char* ByteBuffer_GetData(struct tagByteBuffer* obj);

unsigned short ByteBuffer_Eat(struct tagByteBuffer* obj, unsigned short bytes);
unsigned short ByteBuffer_ReverseEat(struct tagByteBuffer* obj, unsigned short bytes);
void ByteBuffer_clear(struct tagByteBuffer* obj);

int ByteBuffer_empty(struct tagByteBuffer* obj);
int ByteBuffer_IsTailNull(struct tagByteBuffer* obj);

/*
用法:
必须先调用如下两个接口来初始化，然后才能调用其他接口
tagByteBuffer obj;
CreateByteBuffer(&obj);
obj.Init(&obj, buf, sizeof(buf));
*/

#endif
