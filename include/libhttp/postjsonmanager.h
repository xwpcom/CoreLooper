#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

/*
XiongWanPing 2020.11.30
������Ӧhttp post json����,ֻ֧��һ������Ӧ
�����ع�libhttpʱ����ʱ�俼������
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
