#pragma once
#include "auth/authman.h"
//#include "virtualfolder.h"


namespace Bear {
namespace Core {
namespace Net {
namespace Http {


//XiongWanPing 2019.11.30
class HTTP_EXPORT JsonHandler :public Handler
{
	//SUPER(Handler);
public:
	JsonHandler();
	virtual ~JsonHandler();
	void SetUserInfo(shared_ptr<UserInfo> userInfo)
	{
		mUserInfo = userInfo;
	}
	void setHttpRequest(HttpRequest* obj) {
		mHttpRequest = obj;
	}

	void SetVirtualFolder(shared_ptr<VirtualFolder> vm)
	{
		mVirtualFolder = vm;
	}

	string Process(string url);
	string GetExtraHeader();
	void SetPort(int port)
	{
		mPort = port;
	}
	int GetPort()const
	{
		return mPort;
	}
protected:
	int	mPort = 0;
	string mExtraHeader;
	shared_ptr<UserInfo>				mUserInfo;
	shared_ptr<VirtualFolder>			mVirtualFolder;
	HttpRequest* mHttpRequest = nullptr;

};

}
}
}
}
