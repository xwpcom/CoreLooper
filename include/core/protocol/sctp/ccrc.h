#ifndef _CCRC_H
#define _CCRC_H

//XiongWanPing 2018.12.24

//支持累积crc,注意顺序要一致
//首次调用时*crc必须为0xFFFF
unsigned short Crc16Ex(unsigned char *pData, int bytes, unsigned short *crc);

#endif
