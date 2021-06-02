#include "stdafx.h"
#include "digestaccessauth.h"
#include "md5ex.h"
using namespace std;
using namespace Bear::Core;

void DigestAccessAuth::GetRand(char value[33])
{
	static int idx = 0;
	srand((int)ShellTool::GetTickCount64());
	string  seed=StringTool::Format(FMT_LONGLONG, ShellTool::GetTickCount64() + rand() + (++idx));
	CMD5 md5;
	HASH HA1;
	md5.Update((LPBYTE)seed.c_str(), (int)seed.length());
	md5.Final((LPBYTE)HA1);
	CvtHex(HA1, value);
	//DT("value=%s",value);
}

void DigestAccessAuth::CvtHex(
	IN HASH Bin,
	OUT HASHHEX Hex
	)
{
	unsigned short i;
	unsigned char j;

	for (i = 0; i < HASHLEN; i++) {
		j = (Bin[i] >> 4) & 0xf;
		if (j <= 9)
			Hex[i * 2] = (j + '0');
		else
			Hex[i * 2] = (j + 'a' - 10);
		j = Bin[i] & 0xf;
		if (j <= 9)
			Hex[i * 2 + 1] = (j + '0');
		else
			Hex[i * 2 + 1] = (j + 'a' - 10);
	};
	Hex[HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */
void DigestAccessAuth::DigestCalcHA1(
	const IN char * pszAlg,
	const IN char * pszUserName,
	const IN char * pszRealm,
	const IN char * pszPassword,
	const IN char * pszNonce,
	const IN char * pszCNonce,
	OUT HASHHEX SessionKey
	)
{
	CMD5 md5;
	HASH HA1;

	md5.Update((LPBYTE)pszUserName, (int)strlen(pszUserName));
	md5.Update((LPBYTE)":", 1);
	md5.Update((LPBYTE)pszRealm, (int)strlen(pszRealm));
	md5.Update((LPBYTE)":", 1);
	md5.Update((LPBYTE)pszPassword, (int)strlen(pszPassword));
	md5.Final((LPBYTE)HA1);
	CvtHex(HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestAccessAuth::DigestCalcResponse(
	IN HASHHEX HA1,           /* H(A1) */
	const IN char * pszNonce,       /* nonce from server */
	const IN char * pszNonceCount,  /* 8 hex digits */
	const IN char * pszCNonce,      /* client nonce */
	const IN char * pszQop,         /* qop-value: "", "auth", "auth-int" */
	const IN char * pszMethod,      /* method from the request */
	const IN char * pszDigestUri,   /* requested URL */
	IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
	OUT HASHHEX Response      /* request-digest or response-digest */
	)
{
	HASH HA2;
	HASH RespHash;
	HASHHEX HA2Hex;

	// calculate H(A2)
	CMD5 md5;
	md5.Update((LPBYTE)pszMethod, (int)strlen(pszMethod));
	md5.Update((LPBYTE)":", 1);
	md5.Update((LPBYTE)pszDigestUri, (int)strlen(pszDigestUri));
	md5.Final((LPBYTE)HA2);
	CvtHex(HA2, HA2Hex);

	// calculate response
	//MD5Init(&Md5Ctx);
	md5.Update((LPBYTE)HA1, HASHHEXLEN);
	md5.Update((LPBYTE)":", 1);
	md5.Update((LPBYTE)pszNonce, (int)strlen(pszNonce));
	md5.Update((LPBYTE)":", 1);
	if (*pszQop)
	{
		md5.Update((LPBYTE)pszNonceCount, (int)strlen(pszNonceCount));
		md5.Update((LPBYTE)":", 1);
		md5.Update((LPBYTE)pszCNonce, (int)strlen(pszCNonce));
		md5.Update((LPBYTE)":", 1);
		md5.Update((LPBYTE)pszQop, (int)strlen(pszQop));
		md5.Update((LPBYTE)":", 1);
	};
	md5.Update((LPBYTE)HA2Hex, HASHHEXLEN);
	md5.Final((LPBYTE)RespHash);
	CvtHex(RespHash, Response);
}

string  DigestAccessAuth::CreateResponse(const string&  realm, const string& qop,const string& nonce, const string& cnonce,const string& cmd,
	const string& uri, const string& user, const string& password)
{
	const char * pszNonce = nonce.c_str();
	const char* pszCNonce = cnonce.c_str();
	const char * pszUser = user.c_str();
	const char * pszRealm = realm.c_str();
	const char * pszPass = password.c_str();
	char szNonceCount[9] = "00000001";
	const char * pszMethod = cmd.c_str();
	const char * pszQop = qop.c_str();
	const char * pszURI = uri.c_str();
	const char * pszAlg = "";
	HASHHEX HA1;
	HASHHEX HA2 = "";
	HASHHEX Response;
	DigestCalcHA1(pszAlg, pszUser, pszRealm, pszPass, pszNonce, pszCNonce, HA1);
	DigestCalcResponse(HA1, pszNonce, szNonceCount, pszCNonce, pszQop, pszMethod, pszURI, HA2, Response);
	
	 //DT("Response = %s", Response);
	 string  ack = Response;
	 return ack;
}
