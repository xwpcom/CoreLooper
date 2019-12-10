#pragma once
namespace Bear {
namespace Core
{

//XiongWanPing 2017.01.09
class CRYPT_EXT_CLASS Crc16
{
public:
	static WORD Calc(LPBYTE data, int dataBytes);
};
}
}