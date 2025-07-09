#pragma once
#include "httpheader.h"
#include "libhttp/httpconfig.h"
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2016.09.08
//解析client提交的http form data字段
//HttpFormField默认采用ByteBuffer缓存数据，如果在处理大文件上传，请定义子类自行处理
class HttpFormField
{
public:
	HttpFormField();
	virtual ~HttpFormField();

	int InitField(std::shared_ptr<HttpHeader> header, const std::string & fieldName, int rangeStart = 0);
	void SetFileName(const string& filename)
	{
		mFileName = filename;
	}
	virtual void SetConfig(std::shared_ptr<tagWebServerConfig> config)
	{
		mWebConfig = config;
	}

	void setHttpPostCommandHandler(HttpPostCommandHandler* obj)
	{
		mHttpPostCommandHandler = obj;
	}

	enum eResult
	{
		eResult_NeedMoreData,
		eResult_Finish,
		eResult_Error,
	};

	eResult Input(ByteBuffer& inbox);

	bool IsDataReady()const
	{
		return mDataReady;
	}

	virtual void SetDataReady(bool ready = true)
	{
		mDataReady = ready;
	}

	ByteBuffer& GetDataBox()
	{
		return mDataBox;
	}

	//当接收post数据失败或不完整时，会调用本接口
	virtual void OnPostFail()
	{
	}

	Bundle& params()
	{
		return mParams;
	}

protected:
	virtual int Input(LPBYTE data, int dataLen);
protected:
	std::string 	mFieldName;
	string			mFileName;
	std::string 	mBoundary;
	int			mBoundaryBytes = -1;
	int			mTotalBytes = -1;
	bool		mDataReady = false;//数据完整可用
	int			mRangeStart = 0;

	ByteBuffer	mDataBox;

	std::shared_ptr<tagWebServerConfig> mWebConfig;

	Bundle mParams;
	HttpPostCommandHandler* mHttpPostCommandHandler = nullptr;

};
}
}
}
}