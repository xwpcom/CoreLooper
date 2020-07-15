#include "stdafx.h"
#include "hmac_sha256.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>  
#include <openssl/err.h>

#if defined(_WIN32)
#if defined(_WIN64)

//64bit
#if !defined(NDEBUG)
#pragma  comment (lib,"libssl64MDd")
#pragma  comment (lib,"libcrypto64MDd")
#else
#pragma  comment (lib,"libssl64MD")
#pragma  comment (lib,"libcrypto64MD")
#endif // !defined(NDEBUG)

#else

//32 bit
#if !defined(NDEBUG)
#pragma  comment (lib,"libssl32MDd")
#pragma  comment (lib,"libcrypto32MDd")

#else
#pragma  comment (lib,"libssl32MD")
#pragma  comment (lib,"libcrypto32MD")
#endif // !defined(NDEBUG)

#endif //defined(_WIN64)
#endif // defined(_WIN32)

namespace Bear {
namespace Core {

vector<uint8_t> HmacSha256::HMAC_SHA256(const vector<uint8_t>& key, const vector<uint8_t>& value)
{
	unsigned int len = SHA256_DIGEST_LENGTH;
	unsigned char hash[SHA256_DIGEST_LENGTH];
	size_t keyLen = key.size();
	size_t valueLen = value.size();

	HMAC_CTX* hmac = HMAC_CTX_new();
	HMAC_Init_ex(hmac, (unsigned char*)key.data(), keyLen, EVP_sha256(), NULL);
	HMAC_Update(hmac, (unsigned char*)value.data(), valueLen);
	HMAC_Final(hmac, hash, &len);
	HMAC_CTX_free(hmac);

	return std::vector<uint8_t>((uint8_t*)hash, (uint8_t*)hash + SHA256_DIGEST_LENGTH);
}

}
}

