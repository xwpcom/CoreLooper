﻿#pragma once
#include "file/virtualfolder.h"
namespace Bear {
namespace Core {
using namespace FileSystem;
using namespace std;
class UserMan;

namespace Net {
namespace Http {
class AjaxCommandHandler;
class JsonHandler;

struct HTTP_EXPORT tagUriFilePathMap
{
	string mUri;
	string mFilePath;
};

class HttpRequestHandler;
class HTTP_EXPORT HttpRequestFilter
{
public:
	virtual ~HttpRequestFilter() {}
	virtual shared_ptr<HttpRequestHandler>	OnHttpRequest(const string & uri) = 0;
};

class WebSocketHandler;
class HTTP_EXPORT WSFactory
{
public:
	virtual shared_ptr<WebSocketHandler> CreateWebSocketHandler(const string& url) = 0;
};

struct HTTP_EXPORT tagWebServerConfig
{
	shared_ptr<VirtualFolder>		mVirtualFolder;
	weak_ptr<AjaxCommandHandler>	mAjaxCommandHandler;
	weak_ptr<JsonHandler>			mJsonHandler;
	string							mWebRootFolder;
	vector<tagUriFilePathMap>		mMaps;
	shared_ptr<UserMan>				mUserMan;
	string							mMediaRootPath;//用来存放设备或客户端上传来的图片和录像
	shared_ptr<HttpRequestFilter>	mHttpRequestFilter;
	shared_ptr<WSFactory>			mWSFacotry;
};

}
}
}
}
