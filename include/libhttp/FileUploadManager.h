#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Http {


/*
XiongWanPing 2021.12.20
*/

class HTTP_EXPORT FileUploadManager :public Handler
{
	SUPER(Handler);
public:
	FileUploadManager();
	virtual void OnUploadFile(const string& filePath, Bundle& params);
protected:
	void OnCreate();

};

}
}
}
}
