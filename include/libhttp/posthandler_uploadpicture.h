#pragma once
#include "httppostcommandhandler.h"
#include "httpformfield_file.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//处理http post上传图片
class PostHandler_UploadPicture :public HttpPostCommandHandler
{
public:
	PostHandler_UploadPicture();
	~PostHandler_UploadPicture();

protected:
	std::shared_ptr<HttpFormField> CreateField(const std::string & fieldName);
};
}
}
}
}