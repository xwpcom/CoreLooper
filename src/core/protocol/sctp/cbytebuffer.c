#include "protocol/sctp/cbytebuffer.h"
#include "protocol/sctp/cbundle.h"
#include "protocol/sctp/csctp.h"

void ByteBuffer_Init(tagByteBuffer* obj, unsigned char *buf, unsigned short totalBytes)
{
	obj->mBuf = buf;
	obj->mOffset = 0;
	obj->mBytes = 0;
	obj->mTotalBytes = totalBytes;
	if (totalBytes)
	{
		memset(buf, 0, totalBytes);
	}
}

unsigned short ByteBuffer_GetFreeBytes(struct tagByteBuffer* obj)
{
	//-1是为'\0'保留
	if (obj->mBytes >= obj->mTotalBytes - 1)
	{
		return 0;
	}

	return obj->mTotalBytes - obj->mBytes -1;
}

int ByteBuffer_Input(struct tagByteBuffer* obj, const unsigned char *text, unsigned short bytes)
{
	unsigned short freeBytes = ByteBuffer_GetFreeBytes(obj);
	if (freeBytes == 0)
	{
		return 0;
	}

	ByteBuffer_MoveToHead(obj);
	obj->mOffset = 0;
	{
		unsigned short eatBytes = MIN(bytes, freeBytes);
		memcpy(obj->mBuf + obj->mBytes, text, eatBytes);
		obj->mBytes += eatBytes;
		
		if (obj->mBytes < obj->mTotalBytes)
		{
			int bytes = obj->mTotalBytes - obj->mBytes;
			memset(obj->mBuf + obj->mBytes, 0, bytes);//清0方便调试 //
		}

		return eatBytes;
	}
}

int ByteBuffer_Write(struct tagByteBuffer* obj, const unsigned char *text, unsigned short bytes)
{
	return ByteBuffer_Input(obj, text, bytes);
}

int ByteBuffer_WriteByte(struct tagByteBuffer* obj, unsigned char text)
{
	return ByteBuffer_Input(obj, &text, sizeof(text));
}

int ByteBuffer_WriteString(struct tagByteBuffer* obj, const char *text)
{
	if (text && text[0])
	{
		return ByteBuffer_Input(obj, (unsigned char*)text, (unsigned short)strlen(text));
	}
	
	return 0;
}

unsigned short ByteBuffer_GetBytes(struct tagByteBuffer* obj)
{
	return obj->mBytes;
}

char* ByteBuffer_GetData(struct tagByteBuffer* obj)
{
	if (obj->mBytes == 0)
	{
		return 0;
	}

	return (char*)obj->mBuf + obj->mOffset;
}

unsigned short ByteBuffer_ReverseEat(struct tagByteBuffer* obj, unsigned short bytes)
{
	if (obj->mBytes < bytes)
	{
		return 0;
	}

	obj->mBytes -= bytes;

	if (obj->mBytes == 0)
	{
		obj->mOffset = 0;
	}

	obj->mBuf[obj->mOffset + obj->mBytes] = 0;

	return bytes;
}

unsigned short ByteBuffer_Eat(struct tagByteBuffer* obj, unsigned short bytes)
{
	if (obj->mBytes < bytes)
	{
		return 0;
	}

	obj->mOffset += bytes;
	obj->mBytes -= bytes;
	
	if (obj->mBytes == 0)
	{
		obj->mOffset = 0;
		obj->mBuf[0] = 0;
	}

	return bytes;
}

void ByteBuffer_clear(struct tagByteBuffer* obj)
{
	obj->mOffset = 0;
	obj->mBytes = 0;
	if (obj->mBuf)
	{
		obj->mBuf[0] = 0;
	}
}

int ByteBuffer_empty(struct tagByteBuffer* obj)
{
	return obj->mBytes == 0 ? 1 : 0;
}

int ByteBuffer_IsTailNull(struct tagByteBuffer* obj)
{
	return (obj->mBuf[obj->mOffset + obj->mBytes] == 0) ? 1 : 0;
}

void ByteBuffer_MoveToHead(tagByteBuffer* obj)
{
	if (obj->mOffset && obj->mBytes)
	{
		memmove(obj->mBuf, obj->mBuf + obj->mOffset, obj->mBytes);
		obj->mOffset = 0;

		obj->mBuf[obj->mBytes] = 0;
	}

	obj->mOffset = 0;
}

void ByteBuffer_Create(tagByteBuffer* obj)
{
	memset(obj, 0,sizeof(*obj));
}
