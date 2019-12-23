#pragma once
#include "core/protocol/ctp/commontextprotocol.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Protocol {
namespace CTP {

//XiongWanPing 2017.09.23
//通用文本协议,详情见文档"CTP协议.docx"
//为方便移植，应该只采用c++和stl
//本类只负责实现解包和打包的框架
//上层需要实现CommonTextProtocolCB来完成网络收发和具体业务处理
class CommonTextProtocol_Impl :public CommonTextProtocol
{
public:
	CommonTextProtocol_Impl();
	virtual ~CommonTextProtocol_Impl();

	void SetCB(CommonTextProtocolCB* cb)
	{
		mCB = cb;
	}
	void ResetX();

	//收到网络包时调用本接口来解析处理
	virtual int Input(void *data, int dataBytes);

protected:
	virtual int AddCommand(const string&cmd);
	virtual int AddCommand(const string&cmd, const Bundle& bundle);
	virtual int AddCommand(const string&cmd, const Bundle& bundle, const ByteBuffer& body);

	virtual int AddCommandEx(const string&cmd, const Bundle& bundle, const ByteBuffer& body, bool needAck = true);

	virtual int AddNotify(const string&cmd);
	virtual int AddNotify(const string&cmd, const Bundle& bundle);
	virtual int AddNotify(const string&cmd, const Bundle& bundle, const ByteBuffer& body);

	static const char *stristr(const char *psz0, const char *psz1);

	int ParseInbox();
	void ParseHeaderItems(const string& header, Bundle& headerItems);
	void OnCommand(const string&cmd, const Bundle& inputBundle, const ByteBuffer& inputBody);
	void OnCommandAck(const string&cmd, const Bundle& ackBundle, const ByteBuffer& ackBody);

	int OnError(string error);

	CommonTextProtocolCB* mCB = nullptr;

	ByteBuffer mInputBody;
	bool mReset = false;
	int mSeq = -1;

	map<int, Bundle> mWaitAckItems;
private:
	ByteBuffer mInbox;
	ByteBuffer mOutbox;

};

}
}
}
}
}
