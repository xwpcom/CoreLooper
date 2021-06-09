#ifndef _CBUNDLE_H
#define _CBUNDLE_H
#include "cbytebuffer.h"

//XiongWanPing 2018.12.22

//为支持mcu,这里的name和value都没有自行复制，而是直接指向调用者的buffer
//由调用方保持buffer持续有效
typedef struct tagKeyValue
{
	const char *name;
	const char *value;
}tagKeyValue;

typedef struct tagBundle
{
	tagKeyValue *mItems;
	unsigned char mCount;
	unsigned char mTotalCount;
}tagBundle;

void Bundle_Create(tagBundle* obj, tagKeyValue *items,unsigned char totalCount);
const char* Bundle_GetString(tagBundle* obj, const char *name);
int Bundle_GetInt(tagBundle* obj, const char *name);
unsigned int  Bundle_GetUInt(tagBundle* obj, const char* name);
unsigned char Bundle_GetByte(tagBundle* obj, const char *name);
void Bundle_clear(tagBundle* obj);
int Bundle_Push(tagBundle* obj,const char *name, const char *value);
int Bundle_Exists(tagBundle* obj, const char *name);

#endif
