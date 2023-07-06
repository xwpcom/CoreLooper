#include "stdafx.h"
#include "bcdtool.h"
//#include "datetime.h"

//#if defined _CONFIG_CPP

//https://www.cnblogs.com/daochong/p/6532795.html
//单元测试:libmaster TEST_METHOD(bcd)

//功能：二进制取反   
//输入：const unsigned char *src  二进制数据   
//      int length                待转换的二进制数据长度   
//输出：unsigned char *dst        取反后的二进制数据   
//返回：0    success   
int BcdTool::convert(unsigned char* dst, const unsigned char* src, int length)
{
	int i;
	for (i = 0; i < length; i++)
	{
		dst[i] = src[i] ^ 0xFF;
	}
	return 0;
}

//功能：十六进制转为十进制   
//输入：const unsigned char *hex         待转换的十六进制数据   
//      int length                       十六进制数据长度   
//返回：int  rslt                        转换后的十进制数据   
//思路：十六进制每个字符位所表示的十进制数的范围是0 ~255，进制为256   
//      左移8位(<<8)等价乘以256   
unsigned long BcdTool::HextoDec(const unsigned char* hex, int length)
{
	int i;
	unsigned long rslt = 0;
	for (i = 0; i < length; i++)
	{
		rslt += (unsigned long)(hex[i]) << (8 * (length - 1 - i));

	}
	return rslt;
}

//功能：十进制转十六进制   
//输入：int dec                     待转换的十进制数据   
//      int length                  转换后的十六进制数据长度   
//输出：unsigned char *hex          转换后的十六进制数据   
//思路：原理同十六进制转十进制   
//////////////////////////////////////////////////////////   
int BcdTool::DectoHex(int dec, unsigned char* hex, int length)
{
	int i;
	for (i = length - 1; i >= 0; i--)
	{
		hex[i] = (dec % 256) & 0xFF;
		dec /= 256;
	}
	return 0;
}

//功能：求权   
//输入：int base                    进制基数   
//      int times                   权级数   
//返回：unsigned long               当前数据位的权   
unsigned long BcdTool::power(int base, int times)
{
	int i;
	unsigned long rslt = 1;
	for (i = 0; i < times; i++)
		rslt *= base;
	return rslt;
}

//功能：BCD转10进制   
//输入：const unsigned char *bcd     待转换的BCD码   
//      int length                   BCD码数据长度   
//返回：unsigned long               当前数据位的权   
//   
//思路：压缩BCD码一个字符所表示的十进制数据范围为0 ~ 99,进制为100   
//      先求每个字符所表示的十进制值，然后乘以权   
unsigned long  BcdTool::BCDtoDec(const unsigned char* bcd, int length)
{
	int i, tmp;
	unsigned long dec = 0;
	for (i = 0; i < length; i++)
	{
		tmp = ((bcd[i] >> 4) & 0x0F) * 10 + (bcd[i] & 0x0F);
		dec += tmp * power(100, length - 1 - i);
	}
	return dec;
}

//功能：十进制转BCD码   
//输入：int Dec                      待转换的十进制数据   
//      int length                   BCD码数据长度   
//输出：unsigned char *Bcd           转换后的BCD码   
//思路：原理同BCD码转十进制   
int BcdTool::DectoBCD(int Dec, unsigned char* Bcd, int length)
{
	int i;
	int temp;
	for (i = length - 1; i >= 0; i--)
	{
		temp = Dec % 100;
		Bcd[i] = ((temp / 10) << 4) + ((temp % 10) & 0x0F);
		Dec /= 100;
	}
	return 0;
}

int BcdTool::bcd2Dec(BYTE bcd)
{
	return ((bcd >> 4) & 0x0F) * 10 + (bcd & 0x0F);
}

//bcdYear从0x00到0x99,表示2000到2099年
//#define BCD_BASE_YEAR 2000 //1970
time_t BcdTool::bcd2time(BYTE bcdYear, BYTE bcdMonth, BYTE bcdDay, BYTE bcdHour, BYTE bcdMinute, BYTE bcdSecond,int baseYear)
{
	tm t = {0};
	t.tm_year= baseYear + bcd2Dec(bcdYear)-1900;
	t.tm_mon= bcd2Dec(bcdMonth)-1;
	t.tm_mday= bcd2Dec(bcdDay);
	t.tm_hour = bcd2Dec(bcdHour);
	t.tm_min = bcd2Dec(bcdMinute);
	t.tm_sec = bcd2Dec(bcdSecond);

	auto ret=mktime(&t);
	//auto ret=mDateTime.mktime(&t);
	return ret;
}

/*
	LogV(TAG, "%02x:%02x:%02x"
		, BcdTool::byte2Bcd(12)
		, BcdTool::byte2Bcd(34)
		, BcdTool::byte2Bcd(56)
	);
打印 12:34:56

*/

BYTE BcdTool::byte2Bcd(BYTE v)
{
	BYTE buf[1];
	DectoBCD(v, (LPBYTE)&buf, 1);
	return buf[0];
}

//#endif
