#include "protocol/sctp/ccrc.h"

unsigned short Crc16Ex(unsigned char *pData, int bytes, unsigned short *crcInitValue)
{
	unsigned short  CRC = *crcInitValue;
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
	*crcInitValue = CRC;
	return CRC;
}
