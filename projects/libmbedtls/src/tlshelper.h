#pragma once
#include <string>
using namespace std;

namespace Bear {
namespace Core {
namespace Crypt {
class TlsProxy;

//XiongWanPing 2016.01.13
//Ê¹ÓÃmbedtls-2.2.1-apache,²Î¿¼ssl_mail_client.c
class TlsHelper
{
public:
	TlsHelper();
	virtual ~TlsHelper();
	int Init(SOCKET& s, string smtpServer, int port, string sender, string password);
	void UnInit();

	int write_ssl_and_get_response(string text)
	{
		int len = (int)text.length();
		if (len == 0)
		{
			return 0;
		}
		return write_ssl_and_get_response((LPBYTE)text.c_str(), len);
	}
	int write_ssl_and_get_response(const char *buf, int len)
	{
		return write_ssl_and_get_response((unsigned char *)buf, len);
	}
	int write_ssl_and_get_response(unsigned char *buf, size_t len);
	int write_ssl_data(unsigned char *buf, size_t len);
	int write_ssl_data(string text)
	{
		int len = (int)text.length();
		if (len == 0)
		{
			return 0;
		}
		return write_ssl_data((LPBYTE)text.c_str(), len);
	}
	int write_and_get_response(unsigned char *buf, size_t len);

protected:
	void Clear();

	shared_ptr<TlsProxy> mProxy;
};
}
}
}