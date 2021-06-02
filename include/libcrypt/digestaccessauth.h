#pragma once
namespace Bear {
namespace Core
{

#define HASHLEN 16
typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN + 1];

//XiongWanPing 2015.11.08
//HTTP digest access auth
class CRYPT_EXT_CLASS DigestAccessAuth
{
public:
	string  CreateResponse(const string& realm, const string& qop, const string& nonce, const string& cnonce, const string& cmd,
		const string& uri, const string& user, const string& password);

#define IN
#define OUT
	static void DigestCalcHA1(
		const IN char * pszAlg,
		const IN char * pszUserName,
		const IN char * pszRealm,
		const IN char * pszPassword,
		const IN char * pszNonce,
		const IN char * pszCNonce,
		OUT HASHHEX SessionKey
	);

	static void DigestCalcResponse(
		IN HASHHEX HA1,           /* H(A1) */
		const IN char * pszNonce,       /* nonce from server */
		const IN char * pszNonceCount,  /* 8 hex digits */
		const IN char * pszCNonce,      /* client nonce */
		const IN char * pszQop,         /* qop-value: "", "auth", "auth-int" */
		const IN char * pszMethod,      /* method from the request */
		const IN char * pszDigestUri,   /* requested URL */
		IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
		OUT HASHHEX Response      /* request-digest or response-digest */
	);
	static void CvtHex(
		IN HASH Bin,
		OUT HASHHEX Hex
	);

	static void GetRand(char value[33]);
};
}
}