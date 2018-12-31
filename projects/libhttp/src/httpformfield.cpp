#include "stdafx.h"
#include "HttpFormField.h"
using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

HttpFormField::HttpFormField()
{
	mBoundaryBytes = 0;
	mTotalBytes = 0;
}

HttpFormField::~HttpFormField()
{

}

int HttpFormField::InitField(shared_ptr<HttpHeader> header, const string & fieldName, int rangeStart)
{
	mFieldName = fieldName;
	mBoundary = header->GetHeader().mFields.GetString("boundary");
	mBoundaryBytes = (int)mBoundary.length();
	mRangeStart = rangeStart;

	return 0;
}

//上层已确保inbox以'\0'结尾
HttpFormField::eResult HttpFormField::Input(ByteBuffer& inbox)
{
	ASSERT(inbox.IsEndWithNull());

	//接收所有数据，直到遇到\r\n--mBoundary
	LPBYTE data = inbox.GetDataPointer();
	int dataLen = inbox.GetActualDataLength();
	if (!data || dataLen < 4 + mBoundaryBytes)//4为\r\n--长度
	{
		//需要更多数据才能开始解析
		return eResult_NeedMoreData;
	}

	int nc = dataLen - 4 - mBoundaryBytes;
	//在[0..nc]里查找\r\n--mBoundary
	for (int i = 0; i < nc; i++)
	{
		if (data[i] == '\r' && data[i + 1] == '\n'  && data[i + 2] == '-' && data[i + 3] == '-')
		{
			int ret = memcmp(data + i + 4, mBoundary.c_str(), mBoundaryBytes);
			if (ret == 0)
			{
				if (i > 0)
				{
					mTotalBytes += i;
					if (Input(data, i))
					{
						return eResult_Error;
					}

					inbox.Eat(i);
				}

				//DV("mTotalBytes=%d", mTotalBytes);
				return eResult_Finish;
			}
		}
	}

	if (nc > 0)
	{
		mTotalBytes += nc;
		if (Input(data, nc))
		{
			return eResult_Error;
		}
		inbox.Eat(nc);
	}

	return eResult_NeedMoreData;
}

int HttpFormField::Input(LPBYTE data, int dataLen)
{
	int ret = mDataBox.Write(data, dataLen);
	if (ret != dataLen)
	{
		return -1;
	}

	return 0;
}

}
}
}
}
