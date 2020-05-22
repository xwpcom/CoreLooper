#include "stdafx.h"
#include "tlshelper.h"
#include "tlsproxy.h"

using namespace Bear::Core;

namespace Bear {
namespace Core {
namespace Crypt {

TlsHelper::TlsHelper()
{
	mProxy = make_shared<TlsProxy>();
	//Clear();
}

void TlsHelper::Clear()
{
}

TlsHelper::~TlsHelper()
{
	UnInit();
}

int TlsHelper::write_ssl_data(unsigned char *buf, size_t len)
{
	return mProxy->write_ssl_data(buf, len);
}

int TlsHelper::write_ssl_and_get_response(unsigned char *buf, size_t len)
{
	return mProxy->write_ssl_and_get_response(buf, len);
}

int TlsHelper::write_and_get_response(unsigned char *buf, size_t len)
{
	return mProxy->write_and_get_response(buf, len);
}

int TlsHelper::Init(SOCKET& s, string smtpServer, int port, string sender, string password)
{
	return mProxy->Init(s, smtpServer, port, sender, password);
}


void TlsHelper::UnInit()
{
	mProxy->UnInit();
}

}
}
}
