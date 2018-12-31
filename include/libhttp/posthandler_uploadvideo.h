#pragma once
#include "HttpPostCommandHandler.h"
#include "HttpFormField_File.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//处理http post上传录像文件
class PostHandler_UploadVideo :public HttpPostCommandHandler
{
public:
	PostHandler_UploadVideo();
	~PostHandler_UploadVideo();

protected:
	std::shared_ptr<HttpFormField> CreateField(const std::string & fieldName);
};
}
}
}
}