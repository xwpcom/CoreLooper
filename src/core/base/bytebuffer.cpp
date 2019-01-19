#include "stdafx.h"
#include "base/bytebuffer.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif
using namespace Bear::Core;

ByteBuffer::ByteBuffer(void)
{
	m_pBuf = NULL;
	m_cbBuf = 0;
	m_cbMaxBuf=0;
	m_nDataOff=0;
	m_nData=0;
	mUserData=0;
}

ByteBuffer::~ByteBuffer(void)
{
	EmptyEx();
}

// **************************************************************
// Description: 指定缓冲初始和最大尺寸
// Parameters : 
// Return	  : 成功返回0
// Author	  : XiongWanPing
// Date		  : 2011-01-17
// Notice	  : 
// **************************************************************
int ByteBuffer::SetBufferSize(UINT nInitSize,UINT nMaxSize)
{
	if(m_pBuf)
	{
		ASSERT(FALSE);
		return -1;
	}

	if(nInitSize>nMaxSize)
	{
		nMaxSize=nInitSize;
	}

	m_cbBuf = 0;
	m_cbMaxBuf=0;
	m_nDataOff=0;
	m_nData=0;

	m_pBuf = new BYTE[nInitSize];
	if(!m_pBuf)
	{
		DW("fail ByteBuffer::SetBufferSize,nInitSize=%d",nInitSize);
		return -1;
	}

	m_cbMaxBuf=nMaxSize;
	m_cbBuf=nInitSize;
	m_nDataOff=0;
	m_nData=0;
	return 0;
}

// **************************************************************
// Description: 写入指定的数据到缓存
// Parameters : 
// Return	  : 返回成功写入的字节数,失败时返回0
// Author	  : XiongWanPing
// Date		  : 2011-01-17
// Notice	  : 
// **************************************************************
int ByteBuffer::Write(const LPVOID data,int dataLen)
{
	LPBYTE pData = (LPBYTE)data;
	int cbData = dataLen;
	if(!pData || cbData<=0)
	{
		//ASSERT(FALSE);
		return 0;
	}

	if(!m_pBuf)
	{
		SetBufferSize();
		if(!m_pBuf)
		{
			return 0;
		}
	}
	
	if(m_nData+cbData>m_cbBuf)
	{
		if(m_nData+cbData>m_cbMaxBuf)
		{
			DW("box overflow,name=[%s],m_cbMaxBuf=%d,m_nData=%d,cbData=%d",mName.c_str(),m_cbMaxBuf,m_nData,cbData);
			
			//write partly
			cbData = m_cbMaxBuf - m_nData;
		}

		//以stepsize为步长增加buffer
		int stepsize=1024*8;
		int newsize=((m_nData+cbData+stepsize-1)/stepsize)*stepsize;
		if(newsize>m_cbMaxBuf)
		{
			newsize=m_cbMaxBuf;
		}

		LPBYTE pBuf = new BYTE[newsize];
		if(!pBuf)
		{
			DW("fail new BYTE[%d]",newsize);
			return 0;
		}

		memcpy(pBuf,m_pBuf+m_nDataOff,m_nData);
		delete []m_pBuf;
		m_pBuf=pBuf;

		m_cbBuf=newsize;
		m_nDataOff=0;
	}
	
	//下面可以认为buffer是足够的

	int off=m_nDataOff+m_nData;
	ASSERT(off<=m_cbBuf);
	int left=m_cbBuf-off;
	if(left<cbData)
	{
		//移动数据到首地址
		memmove(m_pBuf,m_pBuf+m_nDataOff,m_nData);
		m_nDataOff=0;
		off=m_nDataOff+m_nData;
		left=m_cbBuf-off;
	}

	memcpy(m_pBuf+m_nDataOff+m_nData,pData,cbData);
	m_nData+=cbData;

	return cbData;
}

// **************************************************************
// Description: 写一个字节
// Parameters : 
// Return	  : 成功返回写入的字节数
// Author	  : XiongWanPing
// Date		  : 2011-01-19
// **************************************************************
int ByteBuffer::WriteByte(BYTE data)
{
	if (!m_pBuf)
	{
		SetBufferSize();
		if (!m_pBuf)
		{
			return 0;
		}
	}

	if(m_nDataOff+m_nData<m_cbBuf)
	{
		//快速添加
		m_pBuf[m_nDataOff+m_nData]=data;
		m_nData++;
		return 1;
	}

	//采用普通添加
	int ret = Write(&data,sizeof(data));
	return ret;
}

//准备buffer空间,保证至少有minSize字节
//本函数会清空当前已经存在的数据
//成功时返回0,否则返回-1
int ByteBuffer::PrepareBuf(UINT minSize,bool zero)
{
	clear();
	int ret=0;
	int cbFree=GetFreeSize();
	if(cbFree<(int)minSize)
	{
		EmptyEx();
		ret=SetBufferSize(minSize);
	}

	if(ret==0)
	{
		if(minSize>0 && zero)
		{
			memset(GetDataPointer(),0,minSize);
		}
	}

	return ret;
}

bool ByteBuffer::IsEndWithNull()
{
	int len = GetActualDataLength();
	if (len > 0 && m_cbBuf>m_nDataOff+ len)
	{
		return m_pBuf[m_nDataOff + len -1] == 0 || m_pBuf[m_nDataOff + len] == 0;
	}

	return true;
}

//为方便解析字符串，确保以'\0'结尾
int ByteBuffer::MakeSureEndWithNull()
{
	if(!m_pBuf)
	{
		return 0;
	}

	int cbData=GetActualDataLength();
	if(cbData>0)
	{
		LPBYTE pData=GetDataPointer();
		if(pData[cbData-1]=='\0')
		{
			return 0;
		}
	}
	
	//写入一个'\0'然后回写
	int ret=WriteByte(0);
	if(ret!=1)
	{
		ASSERT(FALSE);
		return -1;
	}

	ReverseEat(1);
	return 0;
}

int ByteBuffer::Append(const ByteBuffer& src, bool makeSureEndNull)
{
	int ret = 0;
	int bytes = src.GetActualDataLength();
	if (bytes>0)
	{
		ret = Write(src.GetDataPointer(), bytes);
	}

	if (makeSureEndNull)
	{
		MakeSureEndWithNull();
	}
	return ret;
}

LPBYTE ByteBuffer::GetDataPointer()const
{
	//ASSERT(m_pBuf);
	auto p = m_pBuf + m_nDataOff;
	if (p)
	{
		//DW("ByteBuffer data pointer is nullptr");
	}
	return p;
}
