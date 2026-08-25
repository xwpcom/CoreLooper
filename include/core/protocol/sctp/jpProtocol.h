#pragma once
using namespace Bear::Core;

using JpFrameCB = std::function<void(const char *,int bytes)>;
/*
XiongWanPing 2026.08.25
文档:jp协议设计.md
sctp可和jp互相转换
*/
class CORE_EXPORT JpDemux
{
public:
	JpDemux();
	void setFrameCB(JpFrameCB cb)
	{
		mFrameCB = cb;
	}

	void inputData(void *data,int bytes);
protected:
	void onRecvJsonText(const string& text);

	JpFrameCB mFrameCB;

	ByteBuffer mInbox;
};
