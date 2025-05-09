﻿#pragma once
#include "bytebuffer.h"
namespace Core {
class ByteTool
{
public:
	static int HexCharToByte(const char *pSrc, LPBYTE pDst, int cbDst);
	static int ByteToHexChar(const LPBYTE pByte, int cbByte, char *pDst, int cbDst);
	static std::string ByteToHexChar(const LPBYTE pByte, int cbByte,const char *fmt="%02x",int newLinePerBytes=0);
	static void HexString2Bin(const char* hexString,ByteBuffer& box);
	static void HexString2Bin(const string& hexString, ByteBuffer& box);
};

}
