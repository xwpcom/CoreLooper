#include "stdafx.h"
#include "wshandler.h"
#include "ajaxcommandhandler.h"
#include "httptool.h"
#include "httpserver.h"
#include "ajaxhandler.h"
#ifndef __APPLE__
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

#define TAG "WSHandler"

#ifdef _DEBUG
class Ajax_TestWS2 :public AjaxHandler
{
	DECLARE_AJAX_CLASS(Ajax_TestWS2)
public:
	string Process(const NameValue& params)
	{
		static int idx = -1;
		//return u8"<Result>test2</Result>";//带中文字符时要加u8前缀
		return StringTool::Format("<Result>test#2.%04d</Result>", ++idx);
	}
};
IMPLEMENT_AJAX_CLASS(Ajax_TestWS2, "ws.test2", "")


class Ajax_TestWS :public AjaxHandler
{
	DECLARE_AJAX_CLASS(Ajax_TestWS)
public:
	string Process(const NameValue& params)
	{
		//return u8"<Result>test</Result>";
		static int idx = -1;
		return StringTool::Format("<Result>test#1.%04d</Result>", ++idx);
	}
};
IMPLEMENT_AJAX_CLASS(Ajax_TestWS, "ws.test", "")
#endif

WSHandler::WSHandler()
{
	SetObjectName("WSHandler");
	LogV(TAG,"%s,this=%p", __func__, this);
}

WSHandler::~WSHandler()
{
	CommonTextProtocolFactory::Destroy(mProtocol);
	LogV(TAG, "%s,this=%p", __func__, this);
}

void WSHandler::OnCreate()
{
	__super::OnCreate();

	mProtocol = CommonTextProtocolFactory::Create();
	mProtocol->SetCB(this);

	int second = 10;
#ifdef _DEBUG
	second = 1;
#endif
	SetTimer(mTimer_CheckAlive, second * 1000);
	UpdateTickAlive();
	//PerformTimer(mTimer_CheckAlive);
}

void WSHandler::OnDestroy()
{
	__super::OnDestroy();

	SignalDestroy(this);
}

//CommonTextProtocolCB#begin
void WSHandler::OnCommand(CommonTextProtocol* obj, const string& cmd, const Bundle& inputBundle, const ByteBuffer& inputBody, Bundle& ackBundle, ByteBuffer& ackBody)
{
	UpdateTickAlive();
}

//收到对方的通知时调用本接口
void WSHandler::OnNotify(CommonTextProtocol* obj, const string& cmd, const Bundle& bundle, const ByteBuffer& body)
{
	UpdateTickAlive();
	//DV("notify=%s", cmd.c_str());
}

//协议等出错时调用本接口
//正常情况下不会触发,仅用于开发调试
void WSHandler::OnError(CommonTextProtocol* obj, int error, const string& desc)
{

}

//有数据要发给对方时，会调用本接口
void WSHandler::Output(CommonTextProtocol* obj, const ByteBuffer& data)
{
	ByteBuffer box;
	box.Append(data);
	SignalSend(this, box);

	if (!box.empty())
	{
		LogV(TAG,"fail output");
		Destroy();
	}
}

//CommonTextProtocolCB#end

void WSHandler::UpdateTickAlive()
{
	mTickAlive = ShellTool::GetTickCount64();
}

void WSHandler::OnTimer(long id)
{
	if(id == mTimer_CheckAlive)
	{
		auto tickNow = ShellTool::GetTickCount64();
		int second = mTimeoutSeconds;
		if (tickNow > mTickAlive + second * 1000)
		{
			LogI(TAG,"%p destroy due to timeout",this);
			Destroy();
		}

		return;
	}

	__super::OnTimer(id);
}

void WSHandler::OnWebSocketRecv(Handler*, LPBYTE data, int bytes)
{
	if (mProtocol)
	{
		mProtocol->Input(data, bytes);
	}
}

void WSHandler::OnWebSocketClosed(Handler*)
{
	Destroy();
}

}
}
}
}
#endif

