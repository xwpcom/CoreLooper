#pragma once
namespace Bear {
namespace Core
{

//XiongWanPing 2017.01.09
class CRYPT_EXT_CLASS Crc16
{
public:
	static WORD Calc(LPBYTE data, int dataBytes);
	static uint16_t calc(const void* data, int dataBytes,uint16_t *crc) 
	{
		return Crc16Ex((const unsigned char*)data, dataBytes, crc);
	}
	//支持累积crc,注意顺序要一致
	//首次调用时*crc必须为0xFFFF
	static unsigned short Crc16Ex(const unsigned char* pData, int bytes, unsigned short* crc);

	static bool CrcMatched(const LPBYTE d, int bytes,bool bigEndian=false);
	static int  FillCrc(LPBYTE d, int bytes);

};
}
}