#pragma once
namespace Bear {
namespace Core
{

//XiongWanPing 2011.01.17
//ByteBuffer用来管理行缓冲,具有如下特点:
//.可以指定最大行缓冲长度
//.保证有效数据是连续的
class CORE_EXPORT ByteBuffer
{
	//DISABLE_COPY_CLASS(ByteBuffer);
public:
	ByteBuffer(void);
	virtual ~ByteBuffer(void);
	
	ByteBuffer& operator=(const ByteBuffer& src);
	ByteBuffer(const ByteBuffer& src)
	{
		m_pBuf = NULL;
		m_cbBuf = 0;
		m_cbMaxBuf = 0;
		m_nDataOff = 0;
		m_nData = 0;

		*this = src;
	}

	void SetName(const char *name)
	{
		mName = name;
	}

	void Lock()
	{
		mLocked = true;
	}

	void Unlock()
	{
		mLocked = false;
	}

	bool IsLocked()const
	{
		return mLocked;
	}

	void AssertNotLocked()const
	{
#ifdef _DEBUG
		ASSERT(!IsLocked());
#endif
	}

	int Append(const ByteBuffer& src, bool makeSureEndNull = true);
	int AppendHex(const string& hex, bool makeSureEndNull = true);

	//指定缓冲初始和最大尺寸
	int SetBufferSize(UINT nInitSize = 32, UINT nMaxSize = 16*1024 * 1024);
	int PrepareBuf(UINT minSize, bool zero = false);

	int MakeSureEndWithNull();
	bool IsEndWithNull();

	void SetUserData(ULONGLONG dwUserData)
	{
		mUserData = dwUserData;
	}

	ULONGLONG GetUserData()const
	{
		return mUserData;
	}

	// **************************************************************
	// Description: 移动数据到首地址
	// Parameters : 
	// Return	  : 成功返回0
	// Author	  : XiongWanPing
	// Date		  : 2011-01-19
	// Notice	  : 调用本函数来保证内部缓冲有最大的连续可写空闲空间
	// **************************************************************
	void MoveToHead()
	{
		AssertNotLocked();

		if (m_pBuf && m_nDataOff > 0)
		{
			if (m_nData > 0)
			{
				memmove(m_pBuf, m_pBuf + m_nDataOff, m_nData);
			}
			else
			{
				ASSERT(FALSE);
			}

			m_nDataOff = 0;
		}
	}

	//返回最大还可写入的字节数
	int GetMaxWritableBytes(bool reservedEndNull = true)
	{
		int bytes = GetActualDataLength();
		int ret = m_cbMaxBuf - bytes;
		if (reservedEndNull)
		{
			--ret;//-1是为'\0'保留一个字节
		}

		if (ret < 0)
		{
			ret = 0;
		}
		return ret;
	}

	//返回当前空闲字节数
	int GetFreeSize()
	{
		if (m_cbBuf >= m_nData)
		{
			return m_cbBuf - m_nData;
		}

		ASSERT(FALSE);
		return 0;
	}

	//返回当前尾部的空闲字数
	//一般与GetNewDataPointer()配合使用
	int GetTailFreeSize()
	{
		if (m_cbBuf >= m_nDataOff + m_nData)
		{
			return m_cbBuf - m_nDataOff - m_nData;
		}

		ASSERT(FALSE);
		return 0;
	}

	int GetBufferSize()const
	{
		return m_cbBuf;
	}

	//返回写入时新数据的buffer首地址,供直接写数据时使用
	LPBYTE GetNewDataPointer()
	{
		return GetDataPointer() + GetActualDataLength();
	}

	int ReadLine(string& line);

	//返回数据指针
	LPBYTE GetDataPointer()const;
	LPBYTE data()const
	{
		return GetDataPointer();
	}

	//返回有效数据的字节数
	int GetActualDataLength()const
	{
		return m_nData;
	}

	int length()const
	{
		return m_nData;
	}
	int bytes()const
	{
		return m_nData;
	}
	int GetDataLength()const
	{
		return m_nData;
	}

	// **************************************************************
	// Description: 空写指定的字节数
	// Parameters : 
	// Return	  : 成功返回0
	// Author	  : XiongWanPing
	// Date		  : 2011-01-19
	// Notice	  : 为减少一次memcpy,有时调用者直接向ByteBuffer的buf写数据
	//				此场景下，由调用者保证ByteBuffer buf的有效性:
	//				一般事先调用MoveToHead()保证ByteBuffer的有效数据位于缓冲最开头
	//				再调用GetFreeSize()得到最大可写字节数,
	//				写入数据后再调用WriteDirect()来通知ByteBuffer增加有效字节数
	// **************************************************************
	int WriteDirect(int nInc)
	{
		AssertNotLocked();

		if (m_nDataOff + m_nData + nInc <= m_cbBuf)
		{
			m_nData += nInc;
			return 0;
		}

		ASSERT(FALSE);
		return -1;
	}

	BOOL IsInited()
	{
		return m_pBuf != NULL;
	}

	//返回缓冲是否为空
	bool IsEmpty()const
	{
		return m_nData == 0;
	}

	//清除有效数据
	virtual void clear()
	{
		AssertNotLocked();

		m_nDataOff = 0;
		m_nData = 0;
	}

	bool empty()const
	{
		return IsEmpty();
	}

	void EmptyEx()
	{
		AssertNotLocked();

		if (m_pBuf)
		{
			delete[]m_pBuf;
			m_pBuf = NULL;
		}
		m_cbBuf = 0;
		m_cbMaxBuf = 0;
		m_nDataOff = 0;
		m_nData = 0;
	}

	/*
	写入string和char时,大概率会把.data()当字符串，所以自动保证以\0结尾
	*/
	int Write(const std::string& str)
	{
		AssertNotLocked();

		int ret = Write((LPVOID)str.c_str(), (int)str.length());
		MakeSureEndWithNull();
		return ret;
	}

	int Write(const char* str)
	{
		AssertNotLocked();

		if (!str || str[0] == 0)
		{
			return 0;
		}

		int ret = Write((LPVOID)str, (int)strlen(str));
		MakeSureEndWithNull();
		return ret;
	}

	//写入指定的数据到缓存
	int Write(const LPVOID pData, int cbData);
	int Write(const uint8_t* data, int cbData)
	{
		return Write((const LPVOID)data, cbData);
	}

	//写一个字节到缓存
	int WriteByte(BYTE data);
	int Write(WORD data)
	{
		return Write(&data, sizeof(data));
	}
	int WriteBE(WORD data)
	{
		WriteByte((BYTE)(data>>8));
		return WriteByte((BYTE)(data&0xFF));
	}
	int WriteBE(int data)
	{
		WriteByte((BYTE)(data >> 24));
		WriteByte((BYTE)(data >> 16));
		WriteByte((BYTE)(data >> 8));
		return WriteByte((BYTE)(data >> 0));
	}
	int Write(int data)
	{
		return Write(&data, sizeof(data));
	}
	int Eat(int cbEat);

	// **************************************************************
	// Description: 从尾部开始消耗指定字节的数据
	// Parameters : 
	// Return	  : 成功返回0
	// Author	  : XiongWanPing
	// Date		  : 2011-01-19
	// Notice	  : 
	// **************************************************************
	int ReverseEat(int cbEat)
	{
		AssertNotLocked();

		if (cbEat < 0)
		{
			ASSERT(FALSE);
			return -1;
		}

		if (m_nData >= cbEat)
		{
			m_nData -= cbEat;

			if (m_nData == 0)
			{
				//消耗完所有数据时，转到首地址
				m_nDataOff = 0;
			}

			return 0;
		}

		//ASSERT(FALSE);
		return -1;
	}

protected:
	LPBYTE	m_pBuf;		//buffer首地址
	int		m_cbBuf;	//当前m_pBuf指向的buffer总字节数
	int		m_cbMaxBuf;	//允许buffer扩展到的最大字节数,m_cbBuf<=m_cbMaxBuf

	int		m_nDataOff;	//有效数据起始偏移
	int		m_nData;	//有效数据字节数
	bool	mLocked = false;//加锁后为只读，不允许改动，可用来调试
	ULONGLONG	mUserData=0;//用户自定义数据，ByteBuffer不会对其做任何操作

	std::string	mName;//给ByteBuffer加个标记，方便诊断问题
};
}
}