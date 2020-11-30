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

class HTTP_EXPORT PostJsonHandler :public Handler
{
public:
	virtual string ProcessJson(JsonObject& json)
	{
		return "";
	}
};

class HTTP_EXPORT PostJsonManager :public Handler
{
	SUPER(Handler);

public:
	PostJsonManager();
	virtual shared_ptr<PostJsonHandler> CreatePostJsonHandler(const string& name) { return nullptr; }

};

}
}
}
}
