#include "stdafx.h"
#include "ftpprotocol.h"
#include "ftpprotocolcb.h"

using namespace Bear::Core;
using namespace Bear::Core::Net::Ftp;

FtpProtocol::FtpProtocol(FtpProtocolCB * cb)
{
	mCB = cb;
}

void FtpProtocol::OnConnect()
{
	Output("220 OK\r\n");
}

int FtpProtocol::Input(const char *data, int bytes)
{
	auto ret = mInbox.Write((const LPVOID)data, bytes);
	if (ret != bytes)
	{
		ASSERT(FALSE);
		return -1;
	}

	auto len = mInbox.GetActualDataLength();
	if (len > 2048)
	{
		ASSERT(FALSE);
		return -1;
	}

	return ParseInbox();
}

int FtpProtocol::Output(const char *ack)
{
	return Output((LPVOID)ack, (int)strlen(ack));
}

int FtpProtocol::Output(LPVOID data, int bytes)
{
	auto ret = -1;
	if (mCB)
	{
		ret = mCB->Output(data, bytes);
	}

	return ret;
}

int FtpProtocol::ParseInbox()
{
	mInbox.MakeSureEndWithNull();

	while (1)
	{
		auto ps = (char*)mInbox.GetDataPointer();
		auto end = strstr(ps, "\r\n");
		if (!end)
		{
			return 0;
		}

		string cmd(ps, end - ps);
		if (mCB)
		{
			auto ret = mCB->OnCommand(cmd);
		}

		mInbox.Eat((int)(end - ps + 2));
	}

	return 0;
}
