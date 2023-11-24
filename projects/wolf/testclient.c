#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>
#include <wolfssl/test.h>
#include <examples/client/client.h>
#include <wolfssl/error-ssl.h>
#include <wolfssl/callbacks.h>
static int handShakeCB(HandShakeInfo* info)
{
	printf("%s\r\n", __FUNCTION__);
	//(void)info;
	printf("cipherName=[%s]\r\n", info->cipherName);
	for (int i = 0; i < info->numberPackets; i++) {
		printf("i=%d,[%s]", i, info->packetNames[i]);
	}
	printf("negotiationError=%d\r\n", info->negotiationError);


	return 0;
}

static int timeoutCB(TimeoutInfo* info)
{
	printf("%s\r\n", __FUNCTION__);
	(void)info;
	return 0;
}

int testWolfSSL(const char* host, int port)
{
	printf("%s %s:%d\r\n", __FUNCTION__, host, port);
	int ret = wolfSSL_Init();
	printf("wolfSSL_Init=%d\r\n", ret);

	WOLFSSL_CTX* ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
	wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);//不加的话wireshark中能看到client向server发Fatal Alert - Unknown CA

	/*
	ret = wolfSSL_CTX_load_verify_locations(ctx, "D:/iot.jjyip.com.p12"
		//, 0
		, R"(D:\os\wolfssl\certs\external)"
	);
	LogV(TAG, "wolfSSL_CTX_load_verify_locations ret=%d", ret);
	*/

	int s = -1;
	{
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//ip = "210.76.75.21";
		//port = 5443;
		//ip = host2ip("iot.jjyip.com");
		//ip = host2ip("163.com");
		//port = 443;

		struct sockaddr_in servAddr;
		servAddr.sin_addr.s_addr = inet_addr(host);
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(port);

		struct timeval tval;
		tval.tv_sec = 5;
		tval.tv_usec = 0;
		int ret = connect(s, (struct sockaddr*)&servAddr, sizeof(struct sockaddr));
		printf("connec(%s:%d) ret = %d\r\n", host, port, ret);

	}

	//https://github.com/dktran65/MyWolfssl
	/*
	Note 2) wolfSSL takes a different approach to certificate verification than OpenSSL does.
	The default policy for the client is to verify the server,
	this means that if you don't load CAs to verify the server you'll get a connect error,
	no signer error to confirm failure (-188).
	If you want to mimic OpenSSL behavior of having SSL_connect succeed
	even if verifying the server failsand reducing security you can do this by calling :
	wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
	另一个解决办法是在wolfSSL_CTX_load_verify_locations用第三个参数加载root pem
	*/
	//wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);

	WOLFSSL* ssl = wolfSSL_new(ctx);
	ret = wolfSSL_set_fd(ssl, s);
	//wolfSSL_set_tlsext_host_name(ssl, "163.com");

	printf("wolfSSL_set_fd ret = %d\r\n", ret);
	char req[256] = { 0 };
	snprintf(req, sizeof(req),
			 "GET / HTTP/1.1\r\n"
			 "\r\n"
	);

	//Sleep(2000);
	//ret = wolfSSL_connect(ssl);
	Timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	ret = wolfSSL_connect_ex(ssl, handShakeCB, timeoutCB, timeout);
	printf("wolfSSL_connect ret=%d\r\n", ret);
	ret = wolfSSL_write(ssl, req, strlen(req));
	printf("wolfSSL_write ret= %d\r\n", ret);

	char ack[8 * 1024] = { 0 };
	ret = wolfSSL_read(ssl, ack, sizeof(ack) - 1);
	printf("wolfSSL_read ret = %d\r\n", ret);
	if (ret > 0)
	{
		ack[ret] = 0;
		printf("recv:%s\r\n", ack);
	}

	//auto _read_bio = BIO_new(BIO_s_mem());

	return 0;
}

int main()
{
	printf("hello\r\n");
	testWolfSSL("210.76.75.21", 5443);//hwCloud obs
	//testWolfSSL("47.106.193.63", 443);//jjy

	return 0;
}
