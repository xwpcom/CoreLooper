#include "stdafx.h"
#include "hmac_sha1.h"
#include <string>
#include <cmath>
#include "base64ex.h"
#include "libcrypt/ssl/sha1.h"

namespace Bear {
namespace Core {
using namespace std;
unsigned long Rol(unsigned long x, int y);
unsigned long Ror(unsigned long x, int y);
unsigned long f(unsigned long B, unsigned long C, unsigned long D, int t);

//在网页验证ok https://www.sojson.com/hash.html
//第2个tab,哈希,勾上HmacSha1,提供text和key
string Crypt::HmacSha1(const string& text, const string& key)
{
	string ack;

	BYTE ackBytes[20] = { 0 };
	sha1_hmac_EX(
		(LPBYTE)key.c_str(), (int)key.length(), 
		(LPBYTE)text.c_str(), (int)text.length(),
		ackBytes);

	for (int i = 0; i < COUNT_OF(ackBytes); i++)
	{
		StringTool::AppendFormat(ack, "%02X", ackBytes[i]);
	}

	return ack;
}

string Crypt::HmacSha1_Base64(const string& text, const string& key)
{
	long H[5] = { 0 };
	BYTE ackBytes[20] = { 0 };
	sha1_hmac_EX(
		(LPBYTE)key.c_str(), (int)key.length(),
		(LPBYTE)text.c_str(), (int)text.length(),
		ackBytes);
	memcpy(H, ackBytes, sizeof(ackBytes));

	return Base64::Encode((LPBYTE)H, sizeof(H));
}
}
}
