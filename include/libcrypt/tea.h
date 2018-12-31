#pragma once

#include "base/bytebuffer.h"

//#ifdef _MSC_VER
//XiongWanPing 2011.05.03
//TEA加密,可对任意长度数据加密(长度不需要整除8字节)
namespace Bear {
namespace Core
{
class CRYPT_EXT_CLASS Tea
{
public:
#ifdef _DEBUG
	int Test();
#endif

	Tea(int round = 16);
	~Tea(void);
	void SetPassword(const char *pszPassword);
	void SetPassword(std::string password)
	{
		return SetPassword(password.c_str());
	}

	int Encode(ByteBuffer& inbox, ByteBuffer& outbox);
	int Decode(ByteBuffer& inbox, ByteBuffer& outbox);

	std::string  EncodeTextWithBase64(std::string  plainText);
	std::string  DecodeTextWithBase64(std::string  cryptText);

protected:
	int Encode(const LPBYTE pData, int cbData, LPBYTE pEnc, int cbEnc);
	int Decode(const LPBYTE pEnc, int cbEnc, LPBYTE pData, int cbData);

	int GetEncodeLength(int cbData);

	void Encrypt(const ULONG *in, ULONG *out);
	void Decrypt(const ULONG *in, ULONG *out);
	int		m_round;	//iteration round to encrypt or decrypt
	BYTE	m_key[16];	//encrypt or decrypt key
};
//#endif
}
}