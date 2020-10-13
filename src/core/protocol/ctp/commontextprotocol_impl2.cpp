#include "stdafx.h"
#include "commontextprotocol_impl2.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

static const char* TAG = "CommonTextProtocol_Impl2";

CommonTextProtocol_Impl2::CommonTextProtocol_Impl2()
{
	mInbox.PrepareBuf(
		//16//测试超大req时的处理 //
		16 * 1024
	);

	mOutbox.PrepareBuf(4 * 1024);
}

CommonTextProtocol_Impl2::~CommonTextProtocol_Impl2()
{
}

int CommonTextProtocol_Impl2::Input(void* data, int dataBytes)
{
	if (!mCB)
	{
		ASSERT(mCB);
		return 0;
	}

	mReset = false;
	LPBYTE d = (LPBYTE)data;
	int leftBytes = dataBytes;
	int eatBytes = 0;

	//在处理包时可能调用Reset()
	//Reset后要忽略未决数据//
	while (leftBytes > 0 && !mReset)
	{
		int maxBytes = mInbox.GetMaxWritableBytes();
		if (maxBytes <= 0)
		{
			return eatBytes;
		}

		int inputBytes = MIN(leftBytes, maxBytes);
		int ret = mInbox.Write(d, inputBytes);
		if (ret != inputBytes || mInbox.MakeSureEndWithNull())
		{
			LogW(TAG,"inbox overflow");
			return eatBytes;
		}

		d += ret;
		leftBytes -= ret;
		eatBytes += ret;

		ret = ParseInbox();
		if (ret)
		{
			LogW(TAG, "parse error");
			return OnError("parse fail");
		}
	}

	return eatBytes;
}

const char* CommonTextProtocol_Impl2::stristr(const char* psz0, const char* psz1)
{
#ifdef _MSC_VER
	return StrStrIA(psz0, psz1);
#else
	return strcasestr(psz0, psz1);
#endif
}

int CommonTextProtocol_Impl2::ParseInbox()
{
	const LPBYTE data = mInbox.data();
	const int dataLen = mInbox.length();

	if (!data || dataLen <= 0)
	{
		//ASSERT(FALSE);
		return -1;
	}

	mInbox.Lock();

	LPBYTE ps = data;
	int cbLeft = dataLen;

	//解析出data中所有完整的command & ack
	while (1)
	{
		ASSERT(cbLeft == (data + dataLen - ps));//还没有解析的字节数
		ASSERT(cbLeft >= 0);
		if (cbLeft <= 0)//已解析完所有数据,
		{
			break;
		}

		{
			char* psz = (char*)ps;
			auto bCmdOK = false;//是否收到完整的ack
									//检查\r\n\r\n
			const char* pszHeaderTail = strstr(psz, "\r\n\r\n");//注意这里不要用StrStrI,它和utf8有点不兼容
			if (!pszHeaderTail)
			{
				// command/ack还没有接收完成
				break;
			}
			auto bodyStart = pszHeaderTail + 4;//4 is "\r\n\r\n";
			int headerLength = (int)(pszHeaderTail - psz + 4);
			int nContentLength = 0;
			//检查是否存在Content-Length
			{
				const char* pszKey = "\r\n" CTP_CMD_BODY_LENGTH "=";
				const char* pszLength = stristr(psz, pszKey);
				//注意只能搜索\r\n\r\n之前的Content-Leghth,避免和下一条命令串扰
				if (pszLength && pszLength < pszHeaderTail)
				{
					char* pszEnd = (char*)stristr(pszLength + strlen(pszKey), "\r\n");
					if (pszEnd)
					{
						string szValue(pszLength + strlen(pszKey));
						nContentLength = atoi(szValue.c_str());
					}
				}
			}

			if (cbLeft < headerLength + nContentLength)
			{
				// command/ack还没有接收完成
				break;
			}

			//ok,已找到一个完整的 command/ack
			//mTickKeepAlive = ShellTool::GetTickCount64();
			Bundle headerItems;//提取header中的字段
			{
				string header(psz, (int)(pszHeaderTail - psz));
				ParseHeaderItems(header, headerItems);
			}

			mInputBody.clear();
			if (nContentLength > 0)
			{
				mInputBody.Write((LPVOID)bodyStart, nContentLength);
			}

			string cmd = headerItems.GetString(CTP_CMD);
			OnCommand(cmd, headerItems, mInputBody);

			int nEat = headerLength + nContentLength;
			ps += nEat;
			cbLeft -= nEat;

			//继续解析剩下的数据//
		}
	}

	mInbox.Unlock();

	int eat = dataLen - cbLeft;
	ASSERT(eat >= 0);
	if (mReset)
	{
		mInbox.clear();
	}
	else if (eat > 0)
	{
		mInbox.Eat(eat);
	}

	return 0;
}

int CommonTextProtocol_Impl2::OnError(string error)
{
	return -1;
}

int CommonTextProtocol_Impl2::AddCommand(const string& cmd)
{
	Bundle bundle;
	ByteBuffer body;
	return AddCommand(cmd, bundle, body);
}

int CommonTextProtocol_Impl2::AddCommand(const string& cmd, const Bundle& bundle)
{
	ByteBuffer body;
	return AddCommand(cmd, bundle, body);
}

int CommonTextProtocol_Impl2::AddCommand(const string& cmd, const Bundle& bundle, const ByteBuffer& body)
{
	if (!mCB)
	{
		ASSERT(mCB);
		return -1;
	}

	auto seq = ++mSeq;
	string ackHeader;
	StringTool::AppendFormat(ackHeader,
		"%s=%s\r\n"
		"%s=%d\r\n"
		, CTP_CMD, cmd.c_str()
		, CTP_CMD_SEQ, seq
	);

	const int bodyBytes = body.GetActualDataLength();
	if (bodyBytes > 0)
	{
		StringTool::AppendFormat(ackHeader, "%s=%d\r\n", CTP_CMD_BODY_LENGTH, bodyBytes);
	}

	{
		auto& items = bundle.mItems;
		if (items.size() > 0)
		{
#ifdef _DEBUG
			ASSERT(!bundle.IsKeyExists(CTP_CMD));
			ASSERT(!bundle.IsKeyExists(CTP_CMD_SEQ));
			ASSERT(!bundle.IsKeyExists(CTP_CMD_BODY_LENGTH));
#endif
			if (bundle.IsKeyExists(CTP_CMD)
				|| bundle.IsKeyExists(CTP_CMD_SEQ)
				|| bundle.IsKeyExists(CTP_CMD_BODY_LENGTH)
				)
			{
				ASSERT(false);
				return -1;
			}

			for (auto iter = items.begin(); iter != items.end(); ++iter)
			{
				StringTool::AppendFormat(ackHeader, "%s=%s\r\n", iter->first.c_str(), iter->second.c_str());
			}
		}
	}

	ackHeader += "\r\n";

	mOutbox.Write(ackHeader.c_str());
	if (bodyBytes > 0)
	{
		mOutbox.Append(body);
	}

	mOutbox.MakeSureEndWithNull();
	mCB->Output(this, mOutbox);
	mOutbox.clear();
	return 0;
}

CommonTextProtocol2* CommonTextProtocolFactory2::Create()
{
	return new CommonTextProtocol_Impl2();
}

void CommonTextProtocolFactory2::Destroy(CommonTextProtocol2*& obj)
{
	CommonTextProtocol_Impl2* pThis = (CommonTextProtocol_Impl2*)obj;
	delete pThis;
	obj = nullptr;
}

void CommonTextProtocol_Impl2::ParseHeaderItems(const string& sz, Bundle& bundle)
{
	TextSeparator demux(sz.c_str(), "\r\n");
	string item;
	while (1)
	{
		int ret = demux.GetNext(item);
		if (ret)
		{
			break;
		}

		if (item.empty())
		{
			continue;
		}

		auto pos = item.find('=');
		string name, value;
		if (pos == -1)
		{
			name = item;
		}
		else
		{
			name = item.substr(0, pos);
			value = item.substr(pos + 1);
		}

		bundle.Set(name, value);
	}
}

void CommonTextProtocol_Impl2::OnCommand(const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody)
{
	ASSERT(mCB);
	//ASSERT(!cmd.empty());
	if (cmd.empty())
	{
		return;
	}

	Bundle ackBundle;
	ByteBuffer ackBody;
	mCB->OnCommand(this, cmd, inputBundle, mInputBody);
}

void CommonTextProtocol_Impl2::ResetX()
{
	mInbox.clear();
	mInputBody.clear();
	mOutbox.clear();

	mReset = true;
	mSeq = -1;
}

}
}
}
}
}