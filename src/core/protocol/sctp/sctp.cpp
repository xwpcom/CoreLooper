#include "stdafx.h"
#include "protocol/sctp/sctp.h"

namespace SCTP
{

static const char* TAG = "sctp";

Sctp::Sctp()
{
	memset(&mObject, 0, sizeof(mObject));
	mEnableSeq = true;

#ifdef _MSC_VER
	SetObjectName("Sctp");
#endif
}

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (char*)(address) - \
                                                  (char*)(&((type *)0)->field)))
#endif

void Sctp::SCTP_OnRecvCommandCB_(tagSCTP *obj, const char *cmd, tagBundle *bundle)
{
	Sctp *context = CONTAINING_RECORD(obj, Sctp, mObject);
	context->OnRecvCommand(cmd,bundle);
}

void Sctp::SCTP_OnErrorCB_(tagSCTP *obj, const char *desc)
{
	Sctp *context = CONTAINING_RECORD(obj, Sctp, mObject);
	context->OnError(desc);
}

void Sctp::OnRecvCommand(const char *cmd, tagBundle *bundle)
{
	SignalOnRecvCommand(this,cmd, bundle);
}

void Sctp::OnError(const char *desc)
{
	LogV(TAG,"%s,%s", __func__, desc);
}

void Sctp::Create()
{
	SCTP_Create(&mObject);

	ByteBuffer_Create(&mObject.mInbox);
	ByteBuffer_Init(&mObject.mInbox, mInbox, sizeof(mInbox));

	ByteBuffer_Create(&mObject.mOutbox);
	ByteBuffer_Init(&mObject.mOutbox, mOutbox, sizeof(mOutbox));

	Bundle_Create(&mObject.mInboxBundle, mInboxItems, COUNT_OF(mInboxItems));
	Bundle_Create(&mObject.mOutboxBundle, mOutboxItems, COUNT_OF(mOutboxItems));

	mObject.mOnRecvCommandCB = (void*)SCTP_OnRecvCommandCB_;
	mObject.mOnErrorCB = (void*)SCTP_OnErrorCB_;
}

void Sctp::clear()
{
	SCTP_clear(&mObject);
}

int Sctp::InputString(char *text)
{
	return InputData(text, (unsigned short)strlen(text));
}

int Sctp::InputData(void *text, unsigned short bytes)
{
	return SCTP_InputData(&mObject,(unsigned char*)text, bytes);
}

int Sctp::CheckCrc()
{
	return SCTP_CheckCrc(&mObject.mInboxBundle);
}

int Sctp::AddField(const char *name, const char *value)
{
	int ret=Bundle_Push(&mObject.mOutboxBundle, name, value);
	if (ret < 0)
	{
		LogW(TAG,"###fail AddField(name=%s,value=%s)",name,value);
	}
	return ret;
}

void Sctp::PrepareCreateOutboxData()
{
	ByteBuffer_clear(&mObject.mOutbox);
	Bundle_clear(&mObject.mOutboxBundle);
}

const tagByteBuffer& Sctp::CreateAckOutboxData(tagBundle * activeBundle)
{
	tagBundle *bundle = &mObject.mOutboxBundle;
	const char *cmd = Bundle_GetString(bundle, "cmd");
	if (strcmp(cmd, ".Ack") != -1)
	{
		if (mEnableSeq)
		{
			if (Bundle_Exists(bundle, "seq"))
			{
				LogW(TAG,"seq should add only by sctp");
			}

			//自动添加seq
			Bundle_Push(bundle, "seq", Bundle_GetString(activeBundle, "seq"));
		}
	}
	else
	{
		//LogW(TAG,"###missing seq for %s", cmd);
	}

	return CreateOutboxData();
}

const tagByteBuffer& Sctp::CreateOutboxData(int* ackSeq)
{
	mObject.mDisableSeq = !mEnableSeq;
	SCTP_CreateOutboxData(&mObject);
	if (ackSeq)
	{
		*ackSeq = mObject.mSeq;
	}

	return mObject.mOutbox;
}

}