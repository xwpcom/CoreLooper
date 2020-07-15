#pragma once

namespace Bear {
namespace Core
{

//XiongWanPing 2019.11.06
//aliyun oss需要用到hmac_sha1

class CRYPT_EXT_CLASS Crypt
{
public:
	static string HmacSha1(const string& text, const string& key);
};


}
}