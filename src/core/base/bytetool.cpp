#include "stdafx.h"
#include "bytetool.h"
#include "base/stringtool.h"

using namespace std;
using namespace Bear::Core;

//把BYTE数据以hex char保存到pDst中
int ByteTool::ByteToHexChar(const LPBYTE pByte, int cbByte, char *pDst, int cbDst)
{
	memset(pDst, 0, cbDst);

	if (cbDst <= cbByte * 2)
	{
		ASSERT(FALSE);
		return -1;
	}

	char szTmp[10];
	pDst[0] = 0;
	for (int i = 0; i<cbByte; i++)
	{
		_snprintf(szTmp, sizeof(szTmp) - 1, "%02x", pByte[i]);
		szTmp[sizeof(szTmp) - 1] = 0;
		strcat(pDst, szTmp);
	}

	return 0;
}

//从pSrc中读取cbDst个十六进制数据到pDst
//如果pSrc中的数据不足以填足pDst,则对应的pDst数据会以0代替.
int ByteTool::HexCharToByte(const char *pSrc, LPBYTE pDst, int cbDst)
{
	if (!pSrc || !pDst)
	{
		ASSERT(FALSE);
		return -1;
	}

	ASSERT(pSrc);
	memset(pDst, 0, cbDst);

	for (int i = 0; i<cbDst; i++)
	{
		//每个hex由pSrc中的两个hex字符组成
		if (pSrc[0] && pSrc[1])
		{
			char ch0 = (char)tolower(pSrc[0]);
			char ch1 = (char)tolower(pSrc[1]);

			int nHigh = 0;
			int nLow = 0;

			if (ch0 >= '0' && ch0 <= '9')
				nHigh = ch0 - '0';
			else if (ch0 >= 'a' && ch0 <= 'f')
				nHigh = ch0 - 'a' + 10;

			if (ch1 >= '0' && ch1 <= '9')
				nLow = ch1 - '0';
			else if (ch1 >= 'a' && ch1 <= 'f')
				nLow = ch1 - 'a' + 10;

			int value = ((nHigh << 4) | nLow);
			ASSERT(value <= 255);

			pDst[i] = (BYTE)value;
		}
		else
		{
			break;
		}

		pSrc += 2;
	}

	return 0;
}

string ByteTool::ByteToHexChar(const LPBYTE pByte, int cbByte,const char* fmt, int newLinePerBytes)
{
	string text;
	for (int i = 0; i < cbByte; i++)
	{
		text+=StringTool::Format(fmt, pByte[i]);

		if (newLinePerBytes && ((i + 1) % newLinePerBytes) == 0)
		{
			text += "\r\n";
		}
	}
	return text;
}

//注意是向box追加，并不清空box已有的数据
void ByteTool::HexString2Bin(const string& hexString, ByteBuffer& box)
{
	string hex(hexString);
	StringTool::makeHexPretty(hex);

	auto pSrc = hex.c_str();
	for (int i = 0; 1; i++)
	{
		//每个hex由pSrc中的两个hex字符组成
		if (pSrc[0] && pSrc[1])
		{
			char ch0 = (char)tolower(pSrc[0]);
			char ch1 = (char)tolower(pSrc[1]);

			int nHigh = 0;
			int nLow = 0;

			if (ch0 >= '0' && ch0 <= '9')
				nHigh = ch0 - '0';
			else if (ch0 >= 'a' && ch0 <= 'f')
				nHigh = ch0 - 'a' + 10;

			if (ch1 >= '0' && ch1 <= '9')
				nLow = ch1 - '0';
			else if (ch1 >= 'a' && ch1 <= 'f')
				nLow = ch1 - 'a' + 10;

			int value = ((nHigh << 4) | nLow);
			ASSERT(value <= 255);

			//pDst[i] = (BYTE)value;
			box.WriteByte(value);
		}
		else
		{
			break;
		}

		pSrc += 2;
	}

}

void ByteTool::HexString2Bin(const char* hexString, ByteBuffer& box)
{
	if (!hexString || !hexString[0])
	{
		return;
	}

	return HexString2Bin((string)hexString, box);
}
