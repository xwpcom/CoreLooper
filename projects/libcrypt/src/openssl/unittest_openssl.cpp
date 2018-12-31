#include "stdafx.h"
#include "CppUnitTest.h"

#include<openssl\ssl.h>
#include "openssl/base.h"
#include <openssl/bio.h>


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

//#pragma comment(lib, "event_core.lib") // libevent2.0
#define OPENSSL_LIB_FOLDER "E:\\os\\openssl.source\\lib\\lib\\"
#pragma comment(lib, OPENSSL_LIB_FOLDER "libcrypto.lib")
#pragma comment(lib, OPENSSL_LIB_FOLDER "libssl.lib")
#pragma comment(lib,"Crypt32.lib")

int openssl_main(int argc, char **argv);

namespace TestServer
{

TEST_CLASS(TestOpenSSL)
{
	TEST_METHOD(AsyncOpenSSL)
	{
		char *args[] = {"test"
			,"-async"
			//,"-partial-write"
			};
		//wireshark filter:
		// ip.addr==123.58.180.7 && tcp.port==443
		openssl_main(COUNT_OF(args),args);
	}
};

}
