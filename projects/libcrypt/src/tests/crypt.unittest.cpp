#include "stdafx.h"
#include "CppUnitTest.h"
#include "libcrypt.inl"
#include "libcrypt/tea.h"
#include "libcrypt/base64ex.h"
#include "xxtea.h"
#include "picosha2.h"
#include "libcrypt/ssl/sha1.h"
#include "hmac_sha1.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;


namespace Crypt
{

static const char* TAG = "Crypt";

TEST_CLASS(Crypt_)
{
public:
	TEST_METHOD(HmacSha1)
	{
		
		BYTE ack[20] = {0};
		auto data = "hello";
		auto key = "123";
		sha1_hmac_EX((LPBYTE)key,strlen(key),
			(LPBYTE)data,strlen(data),
			ack);
		int x = 0;

		{
			string text = "hello";
			string key = "123";
			auto ack=Bear::Core::Crypt::HmacSha1(text, key);
			LogV(TAG, "HMAC_SHA1(%s,%s)=%s", text.c_str(), key.c_str(), ack.c_str());
		}
	}
	
	TEST_METHOD(HmacSha1_Base64)
	{
		string text = "hello";
		string key = "123";
		auto ack1=Bear::Core::Crypt::HmacSha1_Base64(text, key);
		//auto ack2 = Bear::Core::Crypt::HmacSha1_Base64_(text, key);

		LogV(TAG, "ack1=%s", ack1.c_str());
		//LogV(TAG, "ack2=%s", ack2.c_str());
	}

	TEST_METHOD(Base64_Test)
	{
		auto enc = "MFkwEwYHKoZIzj0CAQYIKoEcz1UBgi0DQgAEKEaQ58FGqaFK01LvIpTyfPDMrT99fc/Kkz0Dnj959OBv4loAXYp/lY0egnJ1jVDQsINCYlgFgk6G7c3y3tnjQA==";
		
		ByteBuffer box;
		Base64::Decode(enc, box);
		auto data = box.data();
		auto bytes = box.length();
		auto hex = ByteTool::ByteToHexChar(box.data(), box.length());
		string mTag = "base64";
		LogV(mTag, "hex=%s",hex.c_str());
		int x = 0;
	}
	TEST_METHOD(Tea_Test)
	{
		const char *key= "123";
		auto val = "hello";
		
		Tea obj;
		obj.SetPassword(key);

		ByteBuffer in,enc,dec;
		in.Write(val);

		obj.Encode(in, enc);
		enc.MakeSureEndWithNull();
		auto encText = enc.data();

		obj.Decode(enc, dec);
		dec.MakeSureEndWithNull();
		auto decText = dec.data();
		Assert::IsTrue(strcmp((char*)decText,val)==0);
		int x = 0;	 
	}

	TEST_METHOD(XXTea_Encode)
	{
		/*
		unsigned char* xxtea_encrypt(unsigned char* data, xxtea_long data_len, unsigned char* key, xxtea_long key_len, xxtea_long * ret_length);
		unsigned char* xxtea_decrypt(unsigned char* data, xxtea_long data_len, unsigned char* key, xxtea_long key_len, xxtea_long * ret_length);
		*/

		char data[] = "hello";
		char key[] = "123";

		xxtea_long ret = 0;
		auto enc=xxtea_encrypt((LPBYTE)data, sizeof(data)-1, (LPBYTE)key, sizeof(key)-1,&ret);
		int x = 0;

		xxtea_long ackBytes = 0;
		auto dec=xxtea_decrypt(enc, ret, (LPBYTE)key, sizeof(key)-1,&ackBytes);
		int y = 0;
	}
};


}
