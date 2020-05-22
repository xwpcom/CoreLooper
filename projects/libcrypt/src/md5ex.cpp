#include "stdafx.h"
#include "md5ex.h"
using namespace std;
using namespace Bear::Core;
// MD5相关操作定义
// *************************************************************************
/* Constants for MD5Transform routine. */

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static unsigned char g_MD5_PADDING[64] = 
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions. */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits. */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* 
FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) {(a) += F ((b), (c), (d)) + (x) + (unsigned long)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b);}
#define GG(a, b, c, d, x, s, ac) {(a) += G ((b), (c), (d)) + (x) + (unsigned long)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b);}
#define HH(a, b, c, d, x, s, ac) {(a) += H ((b), (c), (d)) + (x) + (unsigned long)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b);}
#define II(a, b, c, d, x, s, ac) {(a) += I ((b), (c), (d)) + (x) + (unsigned long)(ac); (a) = ROTATE_LEFT ((a), (s)); (a) += (b);}

// *************************************************************************


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMD5::CMD5(void)
{
	// 设置初始值. 
	m_nCount[0] = m_nCount[1] = 0;
	m_nState[0] = 0x67452301L;
	m_nState[1] = 0xefcdab89L;
	m_nState[2] = 0x98badcfeL;
	m_nState[3] = 0x10325476L;
}

CMD5::~CMD5(void)
{}

// **************************************************************
// Function	  : Update
// Description: 更新当前的MD5值
// Parameters : pInputBuf 用于进行计算的数据指针, nLen 数据长度
// Return	  : 空
// Author	  : duanbingyang
// Date		  : 2010-08-19
// Revisions  : 
// **************************************************************
void CMD5::Update(unsigned char *pInputBuf, unsigned int nLen)
{
	unsigned int i, index, partLen;

	/* Compute number of bytes mod 64 */
	index = (unsigned int)((m_nCount[0] >> 3) & 0x3F);

	/* Update number of bits */
	if ((m_nCount[0] += ((unsigned long)nLen << 3))	< ((unsigned long)nLen << 3))
		m_nCount[1]++;
	m_nCount[1] += ((unsigned long)nLen >> 29);

	partLen = 64 - index;

	/* Transform as many times as possible. */
	if (nLen >= partLen)
	{
		Memcpy((unsigned char*)&m_szBuffer[index], (unsigned char*)pInputBuf, partLen);
		Transform(m_nState, m_szBuffer);

		for (i = partLen; i + 63 < nLen; i += 64)
			Transform(m_nState, &pInputBuf[i]);

		index = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	Memcpy((unsigned char*)&m_szBuffer[index], (unsigned char*)&pInputBuf[i], nLen-i);
}

// **************************************************************
// Function	  : Final
// Description: 获取当前的MD5值
// Parameters : pDigest 输出的MD5值
// Return	  : 空
// Author	  : duanbingyang
// Date		  : 2010-08-19
// Revisions  : 
// **************************************************************
void CMD5::Final(unsigned char *pDigest)
{
	unsigned char bits[8];
	unsigned int index, padLen;

	/* Save number of bits */
	Encode(bits, m_nCount, 8);

	/* Pad out to 56 mod 64. */
	index = (unsigned int)((m_nCount[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	Update(g_MD5_PADDING, padLen);

	/* Append length (before padding) */
	Update(bits, 8);

	/* Store state in digest */
	Encode(pDigest, m_nState, 16);

	// 编码结束, 恢复初始值. 
	m_nCount[0] = m_nCount[1] = 0;
	m_nState[0] = 0x67452301L; m_nState[1] = 0xefcdab89L; m_nState[2] = 0x98badcfeL; m_nState[3] = 0x10325476L;
}

// **************************************************************
// Function	  : FormatHex
// Description: 把MD5值转换成十六进制字符串
// Parameters : pOutput 输出字符串首地址, nLen 字符串的长度, pDigest 要转换的MD5值
// Return	  : 空
// Author	  : duanbingyang
// Date		  : 2010-08-19
// Revisions  : 
// **************************************************************
void CMD5::FormatHex(char *pOutput, int nLen, const unsigned char pDigest[16])
{
	int i, j=0;

	memset(pOutput, 0x00, nLen);
	for (i=0; i<16; ++i)
	{
		j += sprintf(pOutput+j, "%02X", pDigest[i]);
	}
}

// **************************************************************
// Function	  : Memset
// Description: 用整形数据值更新, 字符数组数据
// Parameters : pOutput 存放数据指针, nValue 用于设置的数值, nLen 长度
// Return	  : 空
// Author	  : duanbingyang
// Date		  : 2010-08-19
// Revisions  : 
// **************************************************************
void CMD5::Memset(unsigned char* pOutput, int nValue, unsigned int nLen)
{
	unsigned int i;

	for (i = 0; i < nLen; i++)
		((char *)pOutput)[i] = (char)nValue;
}

// **************************************************************
// Function	  : Memcpy
// Description: 内存数据拷贝
// Parameters : pOutput 目标数据指针, pInput 源数据指针, nLen 拷贝的长度
// Return	  : 空
// Author	  : duanbingyang
// Date		  : 2010-08-19
// Revisions  : 
// **************************************************************
void CMD5::Memcpy(unsigned char* pOutput, unsigned char* pInput, unsigned int nLen)
{
	unsigned int i;

	for (i = 0; i < nLen; i++)
		pOutput[i] = pInput[i];
}

// **************************************************************
// Function	  : Encode
// Description: 数据编码操作
// Parameters : pOutput 目标数据指针, pInput 源数据指针, nLen 拷贝的长度
// Return	  : 空
// Author	  : duanbingyang
// Date		  : 2010-08-19
// Revisions  : 
// **************************************************************
void CMD5::Encode(unsigned char *pOutput, unsigned long *pInput, unsigned int nLen)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < nLen; i++, j += 4) 
	{
		pOutput[j] = (unsigned char)(pInput[i] & 0xff);
		pOutput[j+1] = (unsigned char)((pInput[i] >> 8) & 0xff);
		pOutput[j+2] = (unsigned char)((pInput[i] >> 16) & 0xff);
		pOutput[j+3] = (unsigned char)((pInput[i] >> 24) & 0xff);
	}
}

// **************************************************************
// Function	  : Decode
// Description: 数据解码操作
// Parameters : pOutput 目标数据指针, pInput 源数据指针, nLen 拷贝的长度
// Return	  : 空
// Author	  : duanbingyang
// Date		  : 2010-08-19
// Revisions  : 
// **************************************************************
void CMD5::Decode(unsigned long *pOutput, unsigned char *pInput, unsigned int nLen)
{
	unsigned int i, j;

	for (i = 0, j = 0; j < nLen; i++, j += 4)
		pOutput[i] = ((unsigned long)pInput[j]) | (((unsigned long)pInput[j+1]) << 8) | (((unsigned long)pInput[j+2]) << 16) | (((unsigned long)pInput[j+3]) << 24);
}

// **************************************************************
// Function	  : Transform
// Description: MD5加密
// Parameters : pOutput 目标数据指针, pInput 源数据指针, nLen 拷贝的长度
// Return	  : 空
// Author	  : duanbingyang
// Date		  : 2010-08-19
// Revisions  : 
// **************************************************************
void CMD5::Transform(unsigned long nState[4], unsigned char* szBlock)
{
	unsigned long a = nState[0], b = nState[1], c = nState[2], d = nState[3], x[16];

	Decode(x, szBlock, 64);

	/* Round 1 */
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
	FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
	FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
	FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
	FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
	FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
	FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

	/* Round 2 */
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
	GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
	GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
	GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
	HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
	II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
	II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
	II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
	II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
	II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
	II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
	II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

	nState[0] += a;
	nState[1] += b;
	nState[2] += c;
	nState[3] += d;

	/* Zeroize sensitive information. */
	Memset((unsigned char*)x, 0, sizeof(x));
}

string  CMD5::GetResult()
{
	BYTE buf[16];
	Final(buf);

	char ack[64];
	memset(ack, 0, sizeof(ack));
	FormatHex(ack,sizeof(ack),buf);
	return ack;
}
