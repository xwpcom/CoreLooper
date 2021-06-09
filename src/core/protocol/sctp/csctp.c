#include "protocol/sctp/csctp.h"
#include "protocol/sctp/ccrc.h"
#include "stdio.h"

#ifdef __C51__
extern void SCTP_SetError(tagSCTP* obj, const char * desc);
extern void SCTP_OnRecvCommandCB(tagSCTP *obj, const char *cmd, tagBundle *bundle);
extern void SCTP_OnCrcError(const char *body,int bytes);
#else
//c51上不支持成员函数指针，会报error C212: indirect call: parameters do not fit within registers
typedef void(*SCTP_OnRecvCommandCB)(tagSCTP *obj, const char *cmd, tagBundle *bundle);
typedef void(*STCP_OnError)(tagSCTP *obj, const char *desc);

static void SCTP_SetError(tagSCTP* obj, const char * desc)
{
	STCP_OnError cb = (STCP_OnError)obj->mOnErrorCB;
	if (cb)
	{
		cb(obj, desc);
	}
}
#endif

void SCTP_Create(tagSCTP* obj)
{
	memset(obj, 0, sizeof(*obj));
}

void SCTP_clear(struct tagSCTP *obj)
{
	obj->mSeq = 0;
	ByteBuffer_clear(&obj->mInbox);
	ByteBuffer_clear(&obj->mOutbox);
	Bundle_clear(&obj->mInboxBundle);
	Bundle_clear(&obj->mOutboxBundle);
}

//return 0 if check ok
//otherwise return -1
int  SCTP_CheckCrc(tagBundle *bundle)
{
	//除crc字段，要按依次累积所有字段
	int i;
	unsigned short crc = 0xFFFF;
	for (i = 0; i < bundle->mCount; ++i)
	{
		const char *name = bundle->mItems[i].name;
		if (!name[0])
		{
			continue;
		}

		if (strcmp(name, "crc") == 0)
		{
			continue;
		}

		Crc16Ex((unsigned char*)name, strlen(name), &crc);

		{
			const char *value = bundle->mItems[i].value;
			if (value[0])
			{
				Crc16Ex((unsigned char*)value, strlen(value), &crc);
			}
		}
	}

	{
		const char *crcCheck = Bundle_GetString(bundle, "crc");
		if (crcCheck && crcCheck[0])
		{
			char buf[10];
#ifdef _MSC_VER
			sprintf_s(buf, sizeof(buf), "%04X", (int)crc);
#else
			sprintf(buf, "%04X", (int)crc);
#endif

			if (strcmp(buf, crcCheck) == 0)
			{
				return 0;
			}
		}
	}

	return -1;
}

int SCTP_Parse(tagSCTP *obj)
{
	//buffer中可能包含多条命令
	const char *tail = "\r\n\r\n";
	const char *CRLF = "\r\n";
		
	while (!ByteBuffer_empty(&obj->mInbox))
	{
		char *body = ByteBuffer_GetData(&obj->mInbox);
		char *end = strstr(body, tail);
		int eatBytes = 0;
		if (!end)
		{
			//本条命令还没接收完整
			return 0;
		}

		eatBytes = end - body + 4;//4为tail长度

		//解析此命令
		end[2] = 0;//去掉一个\r\n

		Bundle_clear(&obj->mInboxBundle);

		//提取出所有字段
		while (body[0])
		{
			char *lineEnd = strstr(body, CRLF);
			if (!lineEnd)
			{
				break;
			}

			lineEnd[0] = 0;

			{
				//body为此行数据,格式name=value
				char *equalSign = strstr(body, "=");
				if (equalSign)
				{
					equalSign[0] = 0;
					Bundle_Push(&obj->mInboxBundle, body, equalSign + 1);
				}
			}

			body = lineEnd + 2;//转到下一行
		}

		{
			//检查crc
			int crcOK = (SCTP_CheckCrc(&obj->mInboxBundle)==0);
			if (!crcOK)
			{
				SCTP_SetError(obj, "crc error\r\n");
			}
#ifdef __C51__
			if(crcOK)
			{
				const char *cmd = Bundle_GetString(&obj->mInboxBundle, "cmd");
				SCTP_OnRecvCommandCB(obj, cmd, &obj->mInboxBundle);
			}
			else
			{
				SCTP_OnCrcError(ByteBuffer_GetData(&obj->mInbox) , ByteBuffer_GetBytes(&obj->mInbox));
			}
#else
			if (crcOK && obj->mOnRecvCommandCB)
			{
				SCTP_OnRecvCommandCB cb = (SCTP_OnRecvCommandCB)obj->mOnRecvCommandCB;
				const char *cmd = Bundle_GetString(&obj->mInboxBundle, "cmd");
				cb(obj, cmd, &obj->mInboxBundle);
			}
#endif
		}

		ByteBuffer_Eat(&obj->mInbox, (unsigned short)eatBytes);
	}

	return 0;
}

int SCTP_InputData(tagSCTP *obj, unsigned char *text, unsigned short textBytes)
{
	{
		//为避免后续字符串解析出错，当遇到非法0x00字符时主动修改
		unsigned char *p = text;
		int i = 0;
		int hasZero = 0;
		for (i=0;i<textBytes;i++)
		{
			if (p[i] == 0)
			{
				p[i] = 0xCD;
				hasZero = 1;
			}
		}

#ifdef _CONFIG_SERIAL_CAMERA
		if (hasZero)
		{
			printf("sctp get 0x00 char!\r\n");
		}
#endif

	}

	//int len = (int)strlen((char*)text);
	{
		int bytes = ByteBuffer_Write(&obj->mInbox, text, textBytes);
		if (bytes != textBytes)
		{
			SCTP_SetError(obj, "data/protocol error");
			SCTP_clear(obj);
			return -1;
		}
	}

	return SCTP_Parse(obj);
}

int SCTP_InputString(tagSCTP *obj, char *text)
{
	return SCTP_InputData(obj, (unsigned char *)text,(unsigned short)strlen(text));
}

int SCTP_CreateOutboxData(tagSCTP *obj)
{
	tagBundle *bundle = &obj->mOutboxBundle;
	tagByteBuffer *buffer = &obj->mOutbox;

	char seqBuffer[10];

	int i;
	unsigned short crc = 0xFFFF;
	seqBuffer[0] = 0;
	{
		//如果是ack包，需要由app自行添加seq
		//如果不是ack包，这里自动添加seq
		const char *cmd = Bundle_GetString(bundle, "cmd");
		/*
		if (strstr(cmd, ".Ack"))
		{
			//没有seq时打印警告
			if (!Bundle_Exists(bundle, "seq"))
			{
				//DV("###missing seq for %s", cmd);
			}
		}
		else
		*/
		if (!obj->mDisableSeq)
		{
			if (!Bundle_Exists(bundle, "seq"))
			{
				++obj->mSeq;
#ifdef _MSC_VER
				sprintf_s(seqBuffer,sizeof(seqBuffer), "%d", (int)obj->mSeq);
#else
				sprintf(seqBuffer, "%d", (int)obj->mSeq);
#endif
				Bundle_Push(bundle, "seq", seqBuffer);
			}
		}
	}

	ByteBuffer_clear(buffer);

	for (i = 0; i < bundle->mCount; ++i)
	{
		const char *name = bundle->mItems[i].name;
		if (!name[0])
		{
			continue;
		}

		if (strcmp(name, "crc") == 0)
		{
			continue;
		}

		Crc16Ex((unsigned char*)name, strlen(name), &crc);

		{
			const char *value = bundle->mItems[i].value;
			if (value[0])
			{
				Crc16Ex((unsigned char*)value, strlen(value), &crc);
			}
		}
	}

	{
		char buf[10];
#ifdef _MSC_VER
		sprintf_s(buf, sizeof(buf),"%04X", (int)crc);
#else
		sprintf(buf, "%04X", (int)crc);
#endif
		Bundle_Push(bundle,"crc", buf);//注意buf临时有效

		for (i = 0; i < bundle->mCount; ++i)
		{
			tagKeyValue *item = &bundle->mItems[i];
			if (item->name[0])
			{
				ByteBuffer_WriteString(buffer, item->name);
				ByteBuffer_WriteString(buffer, "=");
				ByteBuffer_WriteString(buffer, item->value);
				ByteBuffer_WriteString(buffer, "\r\n");
			}
		}

		ByteBuffer_WriteString(buffer, "\r\n");

		Bundle_clear(bundle);
	}

	return 0;
}

int SCTP_IsReservedKey(const char* name)
{
	static const char* arr[] =
	{
		"cmd",
		"crc",
		"seq",
	};

	for (int i = 0; i < COUNT_OF(arr); ++i)
	{
		if (strcmp(name, arr[i]) == 0)
		{
			return 1;
		}
	}

	return 0;
}

