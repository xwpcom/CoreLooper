#pragma once

//XiongWanPing 2018.12.24

//支持累积crc,注意顺序要一致
//首次调用时*crc必须为0xFFFF
unsigned short Crc16Ex(unsigned char *pData, int bytes, unsigned short *crc)
{
	unsigned short  CRC = *crc;
	int i, j;

	//CRC = 0xFFFF;
	for (i = 0; i < bytes; i++)
	{
		CRC ^= *(pData + i);
		for (j = 0; j < 8; j++)
		{
			if (CRC & 0x1)
				CRC = (CRC >> 1) ^ 0xA001;
			else
				CRC = (CRC >> 1);
		}
	}
	//CRC = ((CRC >> 8) | (CRC << 8));
	*crc = CRC;
	return CRC;
}

class Crc16
{
public:
	static uint16_t Calc(uint8_t* data, int dataBytes)
	{
		uint16_t crc=0xFFFF;
		Crc16Ex(data, dataBytes,&crc);
		return crc;
	}
	
	static bool CrcMatched(uint8_t *d, int bytes, bool bigEndian=false)
	{
		if (bytes <= 2)
		{
			return false;
		}
		
		uint16_t crc = Calc(d,bytes-2);
		uint8_t b1=(crc >> 8) & 0xFF;
		uint8_t b2 = crc & 0xFF;

		if (bigEndian)
		{
			return d[bytes - 2] == b1 && d[bytes - 1] == b2;
		}

		return d[bytes - 2] == b2 && d[bytes - 1] == b1;
	}
	
};
