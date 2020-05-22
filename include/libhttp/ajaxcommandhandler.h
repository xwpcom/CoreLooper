#pragma once
#include "looper/handler.h"
#include "auth/authman.h"
#include "file/virtualfolder.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
namespace Net {
namespace Http {

//XiongWanPing 2016.03.25
class HTTP_EXPORT AjaxCommandHandler :public Handler
{
public:
	AjaxCommandHandler();
	virtual ~AjaxCommandHandler();
	void SetUserInfo(std::shared_ptr<UserInfo> userInfo)
	{
		mUserInfo = userInfo;
	}

	void SetVirtualFolder(std::shared_ptr<VirtualFolder> vm)
	{
		mVirtualFolder = vm;
	}

	void SetContext(std::shared_ptr<Handler> handler)
	{
		mContext = handler;
	}

	std::shared_ptr<Handler> GetContext()
	{
		return mContext.lock();
	}

	std::string  Process(std::string  url);
	std::string  GetExtraHeader();
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
	std::string  mExtraHeader;
	std::shared_ptr<UserInfo>		mUserInfo;
	std::shared_ptr<VirtualFolder>	mVirtualFolder;
	std::weak_ptr<Handler>	mContext;
};
}
}
}
}
