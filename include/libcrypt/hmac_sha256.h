#pragma once

namespace Bear {
namespace Core{

//XiongWanPing 2020.06.19
//采用openssl实现 hmac sha256

class CRYPT_EXT_CLASS HmacSha256
{
public:
	static vector<uint8_t> HMAC_SHA256(const vector<uint8_t>& key, const vector<uint8_t>& value);

};

}
}