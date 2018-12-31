#pragma once

namespace Bear {
namespace Core {
namespace FileSystem {

class CORE_EXPORT DataInterface
{
public:
	virtual ~DataInterface() {}
	virtual int Read(LPBYTE buf, int bytes) = 0;
	virtual int Write(LPBYTE buf, int bytes) = 0;
	virtual std::string GetFilePath() = 0;
};

struct tagTransferFileInfo
{
	tagTransferFileInfo()
	{
		Reset();
	}
	void Reset()
	{
		mRangeStart = 0;
		mTotalBytes = 0;
		mDoneBytes = 0;
		mDataContainer = nullptr;
		mWorking = false;
		mEOF = false;
		mError = false;
		mTag.clear();
		mHandler.reset();
		mAckMsg = 0;
	}

	std::string GetFilePath()
	{
		if (mDataContainer)
		{
			return mDataContainer->GetFilePath();
		}

		return "";
	}

	bool IsError()
	{
		return mError;
	}
	bool IsDone()
	{
		return mTotalBytes == mDoneBytes;
	}

	DWORD mRangeStart = 0;//支持断点续传
	DWORD mTotalBytes;
	DWORD mDoneBytes;//has send or recv
	std::shared_ptr<DataInterface>	mDataContainer;
	std::string				mTag;
	std::weak_ptr<Handler>	mHandler;
	UINT				mAckMsg = 0;
	bool				mWorking;
	bool				mEOF = false;
	bool				mError = false;
	bool IsOpen()const
	{
		return mDataContainer != nullptr;
	}
	bool IsWorking()const
	{
		return IsOpen() && mWorking && !mEOF && !mError;
	}

	//返回值:从0到100
	int GetPercent()
	{
		if (mTotalBytes > 0)
		{
			return (int)(mDoneBytes * 100.0 / mTotalBytes);
		}

		return 0;
	}
};

class CORE_EXPORT DataInterface_File :public DataInterface
{
public:
	DataInterface_File(FILE *file, const std::string& filePath)
	{
		mFile = std::shared_ptr<FILE>(file, ::fclose);
		mFilePath = filePath;
	}

	void SetFlushBytes(int bytes)
	{
		mFlushBytes = bytes;
	}
protected:
	int Read(LPBYTE buf, int bytes)
	{
		auto ret = (int)fread(buf, 1, bytes, mFile.get());
		return ret;
	}

	int Write(LPBYTE buf, int bytes)
	{
		auto ret = (int)fwrite(buf, 1, bytes, mFile.get());
		if (ret > 0)
		{
			mCacheBytes += (int)ret;
			if (mFlushBytes && mCacheBytes >= mFlushBytes)
			{
				fflush(mFile.get());

				mFlushBytes = 0;
				mCacheBytes = 0;
			}
		}
		return ret;
	}

	std::string GetFilePath()
	{
		return mFilePath;
	}

	std::shared_ptr<FILE>	mFile;
	std::string			mFilePath;

	long				mCacheBytes = 0;//当前有待fflush的字节数
	int					mFlushBytes = 0;//非0时，每次写入此字节数时自动fflush,目前是用于GPRS项目，避免2G上传意外丢失过多数据
};

}
}
}
