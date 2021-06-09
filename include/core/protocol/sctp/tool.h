#ifndef _TOOL_H
#define _TOOL_H

int IsPrintableString(const unsigned char *pData, int bytes);
int HexCharToByte(const char *pSrc, unsigned char *pDst, int cbDst);
int ByteToHexChar(const unsigned char *pByte, int cbByte, char *dest, int destBytes);

#endif
