#pragma once
namespace Bear {
namespace Core
{
//XiongWanPing 2009.09.18
class CRYPT_EXT_CLASS Base64
{
public:
#ifdef _DEBUG
	int Test();
#endif

	static std::string  Encode(std::string  szText);
	static std::string  Decode(std::string  szText);
	static std::string  Encode(LPBYTE data, int dataLen);

	static int Decode(std::string  szText, ByteBuffer& box);
	static int
		base64_decode_EX(const char *src_b64,
			int        src_size,
			char       *dest,
			int        dest_size,
			bool     ignore_errors);

	// base64Encode:
	// returns a 0-terminated std::string that the caller is responsible for delete[]ing.
	static char* base64Encode(char const* orig, unsigned origLength);
protected:
	static bool Decode(std::string  szText, char * buf, int buflen, int & outlen);


	//smtp
	static BOOL Base64Encode(LPCSTR szEncoding, int nSize, char *pszOutput, int cbOutput);
	static int  CalcBase64EncDataLen(int cbInputSize);


};
}
}