#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

/*
XiongWanPing 2020.11.30
负责响应http post json命令,只支持一次性响应
后续重构libhttp时才有时间考虑完善
*/

class HttpRequest;
class HTTP_EXPORT PostJsonHandler :public Handler
{
public:
	virtual string ProcessJson(JsonObject& json)
	{
		return "";
	}

	void setHttpRequest(HttpRequest* obj) {
		mHttpRequest = obj;
	}

	HttpRequest *mHttpRequest = nullptr;
};

class HTTP_EXPORT PostJsonManager :public Handler
{
	SUPER(Handler);

public:
	PostJsonManager();
	virtual shared_ptr<PostJsonHandler> CreatePostJsonHandler(const string& name) 
	{
		auto it = mJsonItems.find(name);
		if (it != mJsonItems.end())
		{
			auto obj = it->second();
			return obj;
		}

		return nullptr; 
	}

	void BindJson(const string& name, std::function<shared_ptr<PostJsonHandler>()> fn)
	{
		mJsonItems[name] = fn;
	}
protected:
	unordered_map<string, std::function<shared_ptr<PostJsonHandler>()>> mJsonItems;

};

}
}
}
}
