#pragma once

namespace Bear {
namespace Core
{

class CRYPT_EXT_CLASS CMD5
{
public:
	CMD5(void);
	virtual ~CMD5(void);

	// ***********************
	// 说明: 更新当前的MD5值
	// 参数: pInputBuf 用于进行计算的数据指针, nLen 数据长度
	void	Update(unsigned char *pInputBuf, unsigned int nLen);
	static string MD5(const string& text);
	static string md5(const string& text);
	static string Encode(const string& text)
	{
		return MD5(text);
	}
	static string encode(const string& text)
	{
		return md5(text);
	}
	static string fileMD5(const string& filePath);
	// ***********************
	// 说明: 获取当前的MD5值
	// 参数: pDigest 输出的MD5值, 大小为16字节的字符串类型
	void	Final(unsigned char pDigest[16]);

	std::string  GetResult();

	// ***********************
	// 说明: 把MD5值转换成十六进制字符串
	// 参数: pOutput 输出字符串首地址, nLen 字符串的长度, pDigest 要转换的MD5值
	static void	FormatHex(char *pOutput, int nLen, const unsigned char pDigest[16]);

protected:

	// ***********************
	// 计算MD5相关操作函数
	void	Encode(unsigned char *pOutput, unsigned long *pInput, unsigned int nLen);
	void	Decode(unsigned long *pOutput, unsigned char *pInput, unsigned int nLen);
	void	Transform(unsigned long nState[4], unsigned char szBlock[64]);

	void	Memset(unsigned char* pOutput, int nValue, unsigned int nLen);
	void	Memcpy(unsigned char* pOutput, unsigned char* pInput, unsigned int nLen);

protected:

	// ***********************
	// 数据成员
	unsigned long m_nState[4];			/* state (ABCD) */
	unsigned long m_nCount[2];			/* number of bits, modulo 2^64 (lsb first) */
	unsigned char m_szBuffer[64];		/* input buffer */
};

}
}
