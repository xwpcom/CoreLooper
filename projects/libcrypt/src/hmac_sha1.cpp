#include "stdafx.h"
#include "hmac_sha1.h"
#include <string>
#include <cmath>
#include "base64ex.h"

namespace Bear {
namespace Core {
using namespace std;
unsigned long Rol(unsigned long x, int y);
unsigned long Ror(unsigned long x, int y);
unsigned long f(unsigned long B, unsigned long C, unsigned long D, int t);

#pragma optimize("",off)
/*
XiongWanPing 2020.09.22,发现vs2019版本16.7.4 一个编译器优化bug

TEST_METHOD(crypt)
{
	string text = "hello";
	string secret = "123";
	auto sha = Crypt::HmacSha1(text, secret);
	LogV(TAG, "sha=[%s]", sha.c_str());
}

debug时输出如下正确结果
sha=[66A71BD37FAB86FDEA1AB80F1B760F0143248F8E]

release下输出哪下错误结果
sha=[5E49B6B1475223B998BADCFE10325476C3D2E1F0]
关闭release优化后，release下也能输出正确结果了
*/

//unsigned long H[5];
//unsigned long T[512] = { 0 };
void HMAC(const string& text, const string& key, long H[5]);
void SHA1(const string& s, long H[5]);
// HMAC function
void HMAC(const string& text, const string& key, long H[5])
{
	char c;
	string s;
	unsigned long Key[16] = { 0 };
	unsigned long X[16] = { 0 };
	unsigned long Y[16] = { 0 };
	unsigned long ipad = 0x36363636;
	unsigned long opad = 0x5c5c5c5c;
	int k;
	s = "";

	//Process string key into sub-key
	//Hash key in case it is less than 64 bytes
	if (key.length() > 64)
	{
		SHA1(key, H);
		Key[0] = H[0];
		Key[1] = H[1];
		Key[2] = H[2];
		Key[3] = H[3];
		Key[4] = H[4];
	}
	else
	{
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (4 * i + j <= (int)key.length())
				{
					k = key[4 * i + j];
				}
				else
				{
					k = 0;
				}
				if (k < 0)
				{
					k = k + 256;
				}
				Key[i] = Key[i] + (unsigned long)(k * pow(256, (double)3 - j));
			}
		}
	}

	int i = 0;
	for (i = 0; i < 16; i++)
	{
		X[i] = Key[i] ^ ipad;
		Y[i] = Key[i] ^ opad;
	}

	//Turn X-Array into a String
	for (i = 0; i < 16; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			c = ((X[i] >> 8 * (3 - j)) % 256);
			s = s + c;
		}
	}

	//Append text to string
	s = s + text;

	//Hash X-Array
	SHA1(s, H);

	s = "";

	//Turn Y-Array into a String
	for (i = 0; i < 16; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			c = ((Y[i] >> 8 * (3 - j)) % 256);
			s = s + c;
		}
	}

	//Append Hashed X-Array to Y-Array in string
	for (i = 0; i < 5; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			c = ((H[i] >> 8 * (3 - j)) % 256);
			s = s + c;
		}
	}

	//Hash final concatenated string
	SHA1(s, H);

}

// SHA-1 Algorithm
void SHA1(const string& s, long H[5])
{
	unsigned long K[80];
	unsigned long A, B, C, D, E, TEMP;
	int r, k, ln;
	H[0] = 0x67452301;
	H[1] = 0xefcdab89;
	H[2] = 0x98badcfe;
	H[3] = 0x10325476;
	H[4] = 0xc3d2e1f0;

	ln = s.length();
	r = int((ln + 1) / 64);

	if (((ln + 1) % 64) > 56)
	{
		r = r + 1;
	}

	// initialize Constants
	for (int t = 0; t < 80; t++)
	{
		if (t < 20)
		{
			K[t] = 0x5a827999;
		}

		if ((t > 19) & (t < 40))
		{
			K[t] = 0x6ED9EBA1;
		}
		if ((t > 39) & (t < 60))
		{
			K[t] = 0x8F1BBCDC;
		}
		if (t > 59)
		{
			K[t] = 0xca62c1d6;
		}
	}

	for (int l = 0; l <= r; l++)
	{
		unsigned long W[80] = { 0 };
		//Initialize Text
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				if (4 * i + j <= ln)
				{
					k = s[64 * l + 4 * i + j];
				}
				else
				{
					k = 0;
				}

				if (k < 0)
				{
					k = k + 256;
				}

				if (4 * i + j == ln)
				{
					k = 0x80;
				}

				W[i] = W[i] + (unsigned long)(k * pow(256, (double)3 - j));
			}
		}
		if ((W[14] == 0) & (W[15] == 0))
		{
			W[15] = 8 * s.length();
		}

		// Hash Cycle
		int t = 0;
		for (t = 16; t < 80; t++)
		{
			W[t] = Rol(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);
		}

		A = H[0];
		B = H[1];
		C = H[2];
		D = H[3];
		E = H[4];

		for (t = 0; t < 80; t++)
		{
			TEMP = Rol(A, 5) + f(B, C, D, t) + E + W[t] + K[t];
			E = D;
			D = C;
			C = Rol(B, 30);
			B = A;
			A = TEMP;
		}

		H[0] = H[0] + A;
		H[1] = H[1] + B;
		H[2] = H[2] + C;
		H[3] = H[3] + D;
		H[4] = H[4] + E;

		ln = ln - 64;
	}

}

unsigned long f(unsigned long B, unsigned long C, unsigned long D, int t)
{
	if (t < 20)
	{
		return ((B & C) ^ ((~B) & D));
	}
	if ((t > 19) & (t < 40))
	{
		return (B ^ C ^ D);
	}
	if ((t > 39) & (t < 60))
	{
		return ((B & C) ^ (B & D) ^ (C & D));
	}
	if (t > 59)
	{
		return (B ^ C ^ D);
	}

	return 0;
}


unsigned long Rol(unsigned long x, int y)
{
	if (y % 32 == 0) { return x; }
	else { return ((x << y) ^ (x >> -y)); }
}

unsigned long Ror(unsigned long x, int y)
{
	if (y % 32 == 0) { return x; }
	else { return ((x >> y) ^ (x << -y)); }
}

//代码来自https://github.com/RickMa1104/Hmac_Sha1_1
//在网页验证ok https://www.sojson.com/hash.html
//第2个tab,哈希,勾上HmacSha1,提供text和key
string Crypt::HmacSha1(const string& text, const string& key)
{
	long H[5] = { 0 };
	HMAC(text, key, H);

	string ack;
	for (int i = 0; i < COUNT_OF(H); i++)
	{
		StringTool::AppendFormat(ack, "%08X", H[i]);
	}

	return ack;
}

string Crypt::HmacSha1_Base64(const string& text, const string& key)
{
	long H[5] = { 0 };
	HMAC(text, key, H);

	{
		//2020.09.11,为了匹配aliyun sms加密,对H中的long进行字节顺序调整
		//https://help.aliyun.com/document_detail/101343.html?spm=a2c4g.11186623.6.619.13b47535axh4W1
		for (int i = 0; i < COUNT_OF(H); i++)
		{
			H[i] = htonl(H[i]);
		}
	}

	return Base64::Encode((LPBYTE)H, sizeof(H));
}

#pragma optimize("",on)
}
}
