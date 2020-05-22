#include "stdafx.h"
#include "libcrypt/base64ex.h"
using namespace std;
using namespace Bear::Core;

static const char base64_code[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const int base64_6bits[256] =
{
  -2, -3, -3, -3, -3, -3, -3, -3, -3, -1, -1, -1, -1, -1, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -1, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, 62, -3, -3, -3, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -3, -3, -3, -2, -3, -3,
  -3,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -3, -3, -3, -3, -3,
  -3, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -3, -3, -3, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3
}; /* -1: skip whitespace, -2: end of input, -3: error */

//Returns: the number of bytes stored in @dest, or -1 if invalid data was found.
int
Base64::base64_decode_EX(const char *src_b64,
               int        src_size,
               char       *dest,
               int        dest_size,
               bool     ignore_errors)
{
  int decoded;
  int i;
  int   n;
  int   bits;

  if(src_b64 == NULL || dest == NULL)
	  return -1;

  decoded = 0;
  n = 0;
  bits = 0;
  for (i = 0; (src_size != 0) && (i + 3 <= dest_size); src_b64++, src_size--)
    {
      bits = base64_6bits[(int) *src_b64 & 0xff];
      if (bits < 0)
        {
          if (bits == -2)
            break;
          else if ((bits == -3) && !ignore_errors)
            return -1;
          else
            continue;
        }
      decoded <<= 6;
      decoded += bits;
      if (++n >= 4)
        {
          /* we have decoded 4 source chars => 24 bits of output (3 chars) */
          dest[i++] = decoded >> 16;
          dest[i++] = (decoded >> 8) & 0xff;
          dest[i++] = decoded & 0xff;
          decoded = 0;
          n = 0;
        }
    }
  if ((n == 3) && (i + 2 <= dest_size))
    {
      /* 3 source chars (+ 1 padding "=") => 16 bits of output (2 chars) */
      dest[i++] = decoded >> 10;
      dest[i++] = (decoded >> 2) & 0xff;
    }
  else if ((n == 2) && (i + 1 <= dest_size))
    {
      /* 2 source chars (+ 2 padding "=") => 8 bits of output (1 char) */
      dest[i++] = decoded >> 4;
    }
  if (i < dest_size)
    dest[i] = 0;
  return i;
}

//Returns: the number of bytes stored in @dest.
static int
base64_encode_EX (const char *src,
               int        src_size,
               char       *dest_b64,
               int        dest_size,
               int         columns)
{
	UINT bits;
	int  i;
	int    n;
	int    c;

	if(src == NULL || dest_b64 == NULL)
		return -1;

	n = 0;
	bits = 0;
	c = 0;
	for (i = 0; (src_size != 0) && (i + 4 <= dest_size); src++, src_size--)
	{
		bits += *(BYTE *)src;
		if (++n == 3)
		{
			dest_b64[i++] = base64_code[(bits >> 18) & 0x3f];
			dest_b64[i++] = base64_code[(bits >> 12) & 0x3f];
			dest_b64[i++] = base64_code[(bits >> 6)  & 0x3f];
			dest_b64[i++] = base64_code[bits         & 0x3f];
			bits = 0;
			n = 0;
			if (columns > 0)
			{
				c += 4;
				if ((c >= columns) && (i < dest_size))
				{
					dest_b64[i++] = '\n';
					c = 0;
				}
			}
		}
		else
		{
			bits <<= 8;
		}
	}
	if ((n != 0) && (i + 4 <= dest_size))
	{
		if (n == 1)
		{
			dest_b64[i++] = base64_code[(bits >> 10) & 0x3f];
			dest_b64[i++] = base64_code[(bits >> 4)  & 0x3f];
			dest_b64[i++] = '=';
			dest_b64[i++] = '=';
		}
		else
		{
			dest_b64[i++] = base64_code[(bits >> 18) & 0x3f];
			dest_b64[i++] = base64_code[(bits >> 12) & 0x3f];
			dest_b64[i++] = base64_code[(bits >> 6)  & 0x3f];
			dest_b64[i++] = '=';
		}
	}
	if ((columns > 0) && ((c != 0) || (n != 0)) && (i + 1 < dest_size))
		dest_b64[i++] = '\n';
	if (i < dest_size)
		dest_b64[i] = 0;
	return i;
}

string  Base64::Encode(LPBYTE data, int dataLen)
{
	char *psz=base64Encode((char*)data,dataLen);
	string  sz(psz);
	delete[]psz;
	return sz;
}

string  Base64::Encode(string  szText)
{
	if(szText.length()<=0)
		return "";

	int cbBuf = (int)szText.length()*3;
	if(cbBuf<100)
		cbBuf=100;
	LPBYTE pBuf = new BYTE[cbBuf];
	ASSERT(pBuf);
	if(pBuf)
	{
		memset(pBuf,0,cbBuf);

		string  szEncode;
		int ret = base64_encode_EX (szText.c_str(),(int)szText.length(),(char*)pBuf,cbBuf-1,0);
		if(ret != -1)
		{
			szEncode = (const char*)pBuf;
		}

		delete []pBuf;
		pBuf=NULL;
		return szEncode;
	}
	return "";
}

int Base64::Decode(string  szText, ByteBuffer& box)
{
	ByteBuffer boxTmp;
	int cbBuf = (int)szText.length() * 3;
	if (cbBuf<100)
		cbBuf = 100;

	boxTmp.PrepareBuf(cbBuf);

	int outLen = 0;
	bool ok = Decode(szText, (char*)boxTmp.GetNewDataPointer(), boxTmp.GetTailFreeSize()-1,outLen);
	if (ok && outLen > 0)
	{
		box.Write(boxTmp.GetDataPointer(), outLen);
		box.MakeSureEndWithNull();
		return 0;
	}

	return -1;
}

bool Base64::Decode(string  szText, char * buf, int buflen, int & outlen)
{
	if (szText.length() == 0)
		return false;

	int cbBuf = (int)szText.length() * 3;
	if (cbBuf<100)
		cbBuf = 100;
	LPBYTE pBuf = new BYTE[cbBuf];
	memset(pBuf, 0, cbBuf);

	bool res = false;
	int ret = base64_decode_EX(szText.c_str(), (int)szText.length(), (char*)pBuf, cbBuf - 1, FALSE);
	if (ret != -1 && (ret <= buflen))
	{
		memcpy(buf, pBuf, ret);
		outlen = ret;
		res = true;
	}

	delete[]pBuf;
	pBuf = NULL;
	return res;
}

string  Base64::Decode(string  szText)
{
	if(szText.length()==0)
		return "";
	
	int cbBuf = (int)szText.length()*3;
	if(cbBuf<100)
		cbBuf=100;
	LPBYTE pBuf = new BYTE[cbBuf];
	memset(pBuf,0,cbBuf);

	string  szDecode;
	int ret = base64_decode_EX(szText.c_str(),(int)szText.length(),(char*)pBuf,cbBuf-1,FALSE);
	if(ret != -1)
	{
		szDecode = (const char*)pBuf;
	}

	delete []pBuf;
	pBuf=NULL;
	return szDecode;
}

//smtp base64

//计算出base64编码所需要的输出buf大小,此大小包括'\0'.
int Base64::CalcBase64EncDataLen(int cbInputSize)
{
	int nSize = cbInputSize;
	int size = (nSize + 2) / 57 * 2;
	size += nSize % 3 != 0 ? (nSize - nSize % 3 + 3) / 3 * 4 : nSize / 3 * 4;
	return size+1;
}

BOOL Base64::Base64Encode(LPCSTR szEncoding, int nSize,char *pszOutput,int cbOutput)
{
	//计算空间
	int size = (nSize + 2) / 57 * 2;
	size += nSize % 3 != 0 ? (nSize - nSize % 3 + 3) / 3 * 4 : nSize / 3 * 4;
	ASSERT(cbOutput > size);
	
	//Base64编码字符集：
	const char *pszBase64Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	memset(pszOutput, 0, cbOutput);

	LPCSTR szInput = szEncoding;

	int nBitsRemaining = 0, nPerRowCount = 0;//换行计数
    int nBitStorage = 0, nScratch = 0;
	int i, lp, endlCount;

	for(i=0, lp=0, endlCount = 0; lp < nSize; lp++)
	{
		nBitStorage = (nBitStorage << 8) | (szInput[lp] & 0xff);//1 byte//the lowest-byte to 0 not cycle
		nBitsRemaining += 8;//读了一个字节，加八位

		do//nBitStorage"剩下的位"记录变量
		{
			nScratch = nBitStorage >> (nBitsRemaining - 6) & 0x3f;//提出最前的六位
			pszOutput[i++] = pszBase64Alphabet[nScratch];
			nPerRowCount++;
			if(nPerRowCount >= 76)
			{
				pszOutput[i++] = '\r';
				pszOutput[i++] = '\n';
				endlCount += 2;
				nPerRowCount = 0;
			}
			nBitsRemaining -= 6;
		}while(nBitsRemaining >= 6);
	}

	if(nBitsRemaining != 0)
	{
		//原数据最后多一个两个字节时进入，编码未结束nBitsRemaining!=0
		nScratch = nBitStorage << (6-nBitsRemaining);//空位补0
		nScratch &= 0x3f;
		pszOutput[i++] = pszBase64Alphabet[nScratch];
		nPerRowCount++;
		if(nPerRowCount >= 76)
		{
			pszOutput[i++] = '\r';
			pszOutput[i++] = '\n';
			endlCount += 2;
			nPerRowCount = 0;
		}
	} 

	// Pad with '=' as per RFC 1521
	while((i - endlCount) % 4 != 0)
	{
		pszOutput[i++] = '=';
		nPerRowCount++;
		if(nPerRowCount >= 76)
		{
			pszOutput[i++] = '\r';
			pszOutput[i++] = '\n';
			endlCount += 2;
			nPerRowCount = 0;
		}
	}

	return TRUE;
}

static const char base64Char[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* Base64::base64Encode(char const* origSigned, unsigned origLength) 
{
	unsigned char const* orig = (unsigned char const*)origSigned; // in case any input bytes have the MSB set
	if (orig == NULL) return NULL;

	unsigned const numOrig24BitValues = origLength/3;
	BOOL havePadding = origLength > numOrig24BitValues*3;
	BOOL havePadding2 = origLength == numOrig24BitValues*3 + 2;
	unsigned const numResultBytes = 4*(numOrig24BitValues + (havePadding?1:0));
	char* result = new char[numResultBytes+1]; // allow for trailing '\0'
	memset(result, 0, numResultBytes + 1);

	// Map each full group of 3 input bytes into 4 output base-64 characters:
	unsigned i;
	for (i = 0; i < numOrig24BitValues; ++i) {
		result[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
		result[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
		result[4*i+2] = base64Char[((orig[3*i+1]<<2) | (orig[3*i+2]>>6))&0x3F];
		result[4*i+3] = base64Char[orig[3*i+2]&0x3F];
	}

	// Now, take padding into account.  (Note: i == numOrig24BitValues)
	if (havePadding) {
		result[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
		if (havePadding2) {
			result[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
			result[4*i+2] = base64Char[(orig[3*i+1]<<2)&0x3F];
		} else {
			result[4*i+1] = base64Char[((orig[3*i]&0x3)<<4)&0x3F];
			result[4*i+2] = '=';
		}
		result[4*i+3] = '=';
	}

	result[numResultBytes] = '\0';
	return result;
}

#if 0
//XiongWanPing 2010.05.11
//下面这个base64也可以用,并且看起来效率最高,以后有时间再测试一下

static const unsigned char base64_enc_map[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/'
};

static const unsigned char base64_dec_map[128] =
{
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127,  62, 127, 127, 127,  63,  52,  53,
     54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
    127,  64, 127, 127, 127,   0,   1,   2,   3,   4,
      5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
     25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
     29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
     39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
     49,  50,  51, 127, 127, 127, 127, 127
};

//Encode a buffer into base64 format
int base64_encode_EX( unsigned char *dst, int *dlen,unsigned char *src, int  slen )
{
    int i, n;
    int C1, C2, C3;
    unsigned char *p;

    if( slen == 0 )
        return( 0 );

    n = (slen << 3) / 6;

    switch( (slen << 3) - (n * 6) )
    {
        case  2: n += 3; break;
        case  4: n += 2; break;
        default: break;
    }

    if( *dlen < n + 1 )
    {
        *dlen = n + 1;
        return -1;
    }

    n = (slen / 3) * 3;

    for( i = 0, p = dst; i < n; i += 3 )
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }

    if( i < slen )
    {
        C1 = *src++;
        C2 = ((i + 1) < slen) ? *src++ : 0;

        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

        if( (i + 1) < slen )
             *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
        else *p++ = '=';

        *p++ = '=';
    }

    *dlen = p - dst;
    *p = 0;

    return( 0 );
}

//Decode a base64-formatted buffer
int base64_decode_EX( unsigned char *dst, int *dlen,unsigned char *src, int  slen )
{
    int i, j, n;
    unsigned long x;
    unsigned char *p;

    for( i = j = n = 0; i < slen; i++ )
    {
        if( ( slen - i ) >= 2 &&
            src[i] == '\r' && src[i + 1] == '\n' )
            continue;

        if( src[i] == '\n' )
            continue;

        if( src[i] == '=' && ++j > 2 )
            return -1;

        if( src[i] > 127 || base64_dec_map[src[i]] == 127 )
            return -1;

        if( base64_dec_map[src[i]] < 64 && j != 0 )
            return -1;

        n++;
    }

    if( n == 0 )
        return( 0 );

    n = ((n * 6) + 7) >> 3;

    if( *dlen < n )
    {
        *dlen = n;
        return -1;
    }

   for( j = 3, n = x = 0, p = dst; i > 0; i--, src++ )
   {
        if( *src == '\r' || *src == '\n' )
            continue;

        j -= ( base64_dec_map[*src] == 64 );
        x  = (x << 6) | ( base64_dec_map[*src] & 0x3F );

        if( ++n == 4 )
        {
            n = 0;
            if( j > 0 ) *p++ = (unsigned char)( x >> 16 );
            if( j > 1 ) *p++ = (unsigned char)( x >>  8 );
            if( j > 2 ) *p++ = (unsigned char)( x       );
        }
    }

    *dlen = p - dst;

    return( 0 );
}

#ifdef _DEBUG
int Base64::Test()
{
	string  usr="admin";

	BYTE dst[2000];
	int dstlen=sizeof(dst)-1;
	int ret = base64_encode_EX(dst,&dstlen,(LPBYTE)usr.c_str(),usr.length());

	BYTE ori[2000];
	int orilen=sizeof(ori)-1;
	ret = base64_decode_EX(ori,&orilen,dst,dstlen);
	ori[orilen]=0;
	return 0;
}
#endif

#endif
