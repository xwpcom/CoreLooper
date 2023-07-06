#pragma once


//XiongWanPing 2021.12.02
// 2023.07.06为实现SL651协议引入
//rtc用到bcd
//https://www.cnblogs.com/daochong/p/6532795.html
class CORE_EXPORT BcdTool
{
public:
	static int convert(unsigned char* dst, const unsigned char* src, int length);
	static unsigned long HextoDec(const unsigned char* hex, int length);
	static int DectoHex(int dec, unsigned char* hex, int length);
	static unsigned long power(int base, int times);
	static unsigned long  BCDtoDec(const unsigned char* bcd, int length);
	static int DectoBCD(int Dec, unsigned char* Bcd, int length);

	static int bcd2Dec(BYTE bcd);
	static time_t bcd2time(BYTE bcdYear, BYTE bcdMonth, BYTE bcdDay, BYTE bcdHour, BYTE bcdMinute, BYTE bcdSecond,int baseYear=2000);
	static BYTE byte2Bcd(BYTE v);

};
