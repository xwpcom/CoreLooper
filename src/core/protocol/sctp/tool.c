#include "protocol/sctp/tool.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

int IsPrintableString(const unsigned char *pData, int bytes)
{
	int i;
	for (i = 0; i < bytes; i++)
	{
		char ch = pData[i];
		if (!isprint(ch) && ch!='\r' && ch!='\n')
		{
			return 0;
		}
	}

	return 1;
}

//从pSrc中读取cbDst个十六进制数据到pDst
//如果pSrc中的数据不足以填足pDst,则对应的pDst数据会以0代替.
int HexCharToByte(const char *pSrc, unsigned char *pDst, int cbDst)
{
	int i=0;
	if (!pSrc || !pDst)
	{
		return -1;
	}

	memset((char*)pDst, 0, cbDst);

	for (i = 0; i < cbDst; i++)
	{
		//每个hex由pSrc中的两个hex字符组成
		if (pSrc[0] && pSrc[1])
		{
			char ch0 = tolower(pSrc[0]);
			char ch1 = tolower(pSrc[1]);

			int nHigh = 0;
			int nLow = 0;
			int value=0;

			if (ch0 >= '0' && ch0 <= '9')
				nHigh = ch0 - '0';
			else if (ch0 >= 'a' && ch0 <= 'f')
				nHigh = ch0 - 'a' + 10;

			if (ch1 >= '0' && ch1 <= '9')
				nLow = ch1 - '0';
			else if (ch1 >= 'a' && ch1 <= 'f')
				nLow = ch1 - 'a' + 10;

			value = ((nHigh << 4) | nLow);
			//ASSERT(value <= 255);

			pDst[i] = (unsigned char)value;
		}
		else
		{
			break;
		}

		pSrc += 2;
	}

	return 0;
}

//在simapp TEST(Tool, ByteToHexChar)中单元测试
int ByteToHexChar(const unsigned char *pByte, int cbByte, char *dest, int destBytes)
{
	char *d = dest;
	int i = 0;
	d[0] = 0;

	if (cbByte * 2 >= destBytes)//确保dest以'\0'结尾
	{
		return 0;
	}

	for (i = 0; i < cbByte; i++)
	{
		char buf[8];
		int value = pByte[i];
#ifdef _MSC_VER
		sprintf_s(buf, "%02X", value);
#else
		sprintf(buf, "%02X", value);
#endif

		d[0] = buf[0];
		d[1] = buf[1];
		d += 2;
	}

	d[0] = 0;//结尾加'\0'

	return cbByte * 2;
}
