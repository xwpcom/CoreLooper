#include "stdafx.h"
#include "CppUnitTest.h"
#include "libcrypt.inl"
#include "libcrypt/tea.h"
#include "libcrypt/base64ex.h"
#include "xxtea.h"
#include "picosha2.h"

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

TEST_CLASS(Crypt)
{
public:
	TEST_METHOD(Base64_Test)
	{
		auto enc = "4bP0vuLHTko=";
		
		ByteBuffer box;
		Base64::Decode(enc, box);
		auto data = box.data();
		auto bytes = box.length();
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
