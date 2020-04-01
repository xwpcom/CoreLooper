#include "stdafx.h"
#include "CppUnitTest.h"
#include "libcrypt.inl"
#include "libcrypt/tea.h"

#ifdef _MSC_VER_DEBUG
#define new DEBUG_NEW
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;


namespace Crypt
{

TEST_CLASS(Tea_UnitTest)
{
public:
	TEST_METHOD(Test)
	{
		const char *key= "1234567890123456";
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
};


}
