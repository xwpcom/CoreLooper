//原文链接：https ://blog.csdn.net/XiaoYuWen1242466468/java/article/details/78274891

#ifndef XXTEA_H
#define XXTEA_H

#include <stddef.h> /* for size_t & NULL declarations */

#if defined(_MSC_VER)

typedef unsigned __int32 xxtea_long;

#else

#if defined(__FreeBSD__) && __FreeBSD__ < 5
/* FreeBSD 4 doesn't have stdint.h file */
#include <inttypes.h>
#else
#include <stdint.h>
#endif

typedef uint32_t xxtea_long;

#endif /* end of if defined(_MSC_VER) */

#define XXTEA_MX (z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (k[p & 3 ^ e] ^ z)
#define XXTEA_DELTA 0x9e3779b9

unsigned char* xxtea_encrypt(unsigned char* data, xxtea_long data_len, unsigned char* key, xxtea_long key_len, xxtea_long* ret_length);
unsigned char* xxtea_decrypt(unsigned char* data, xxtea_long data_len, unsigned char* key, xxtea_long key_len, xxtea_long* ret_length);

#endif
