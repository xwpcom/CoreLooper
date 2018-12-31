#pragma once
namespace Bear {
namespace Core
{
//XiongWanPing 2018.05.08
//https://www.cnblogs.com/daochong/p/6532795.html
class CRYPT_EXT_CLASS BCD
{
public:
	static BYTE SmallInt2BCD(int v)
	{
		ASSERT(v >= 0 && v < 100);

		int i = 0;
		int temp;
		temp = v % 100;
		BYTE bcd = ((temp / 10) << 4) + ((temp % 10) & 0x0F);
		return bcd;
	}

	static int BCD2SmallInt(BYTE v)
	{
		int i = 0;
		int length = 1;
		unsigned long dec = 0;
		int tmp = ((v >> 4) & 0x0F) * 10 + (v & 0x0F);
		dec += tmp * power(100, length - 1 - i);
		return dec;
	}

protected:
	static unsigned long power(int base, int times)
	{
		int i;
		unsigned long rslt = 1;
		for (i = 0; i < times; i++)
			rslt *= base;
		return rslt;
	}


};

}
}