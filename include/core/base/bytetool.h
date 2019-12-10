#pragma once
namespace Bear {
namespace Core
{
class CORE_EXPORT ByteTool
{
public:
	static int HexCharToByte(const char *pSrc, LPBYTE pDst, int cbDst);
	static int ByteToHexChar(const LPBYTE pByte, int cbByte, char *pDst, int cbDst);
	static std::string ByteToHexChar(const LPBYTE pByte, int cbByte,const char *fmt="%02x");
	static void HexString2Bin(const char* hexString,ByteBuffer& box);
};
}
}