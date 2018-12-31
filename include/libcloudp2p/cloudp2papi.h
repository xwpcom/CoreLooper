#pragma once
#ifdef _MSC_VER
#define WIN32DLL
#endif

#include "ppcs_error.h"

extern "C"
{
#include "ppcs_api.h"
}

namespace Bear {
namespace Core {
namespace Net {
namespace CloudP2P {

#define _CONFIG_CLOUD_P2P_JJY	//启用JJY尚云服务器
#ifdef _CONFIG_CLOUD_P2P_JJY
//JJY p2p服务器
#define CLOUD_P2P_INIT_STRING	"EBGCEBBBKLJDHFJFEGGDFBEGHHNNBHPIAGCPHMBHHFKKNNJHBLHMAJJABGKOMHLFABNLPDDDKINPFLHHJAIHIMAG"
#define CLOUD_P2P_INIT_STRING_APP "EBGIEABAKJJFHPJPEKGGENENHIMJGCNGGJECAGCJBDIHKNLJCMBJCNOAHPLDJMKHBDMJLKCGPMMMAJDDIANGIBBANGPI"
//尚云互联胡工建议设备端用InitString，APP端用InitStringApp
//InitString是域名和IP一起加密生成的。用在设备端，防止IP更换时，只需域名重新指向就可以了，不用更新固件
//InitStringApp用在APP端，是纯IP加密生成的。不需要域名解析。

#else
//尚云demo服务器
#define CLOUD_P2P_INIT_STRING	"EBGAEIBIKHJJGFJKEOGCFAEPHPMAHONDGJFPBKCPAJJMLFKBDBAGCJPBGOLKIKLKAJMJKFDOOFMOBECEJIMM"
#endif

#define P2P_CONNECT_FLAGS(enableLanSearch,enableP2P,onlyServerRelay,timeout) ((enableLanSearch?1:0)| (enableP2P?timeout:(0x0F<<1)) | (onlyServerRelay?(1<<6):0))

//XiongWanPing 2016.07.14
//封装测试尚云P2P
//尚云P2P用法请见文档cloud.p2p.usage.txt
class CLOUD_P2P_EXT_CLASS CloudP2PApi
{
	friend class CloudP2PDataChannel;
public:
	CloudP2PApi();
	virtual ~CloudP2PApi();

	static UINT GetVersion();
	static int NetworkDetect();
	static int LoginStatus_Check(char& status);

	static int Listen(const std::string& uid, UINT timeout_second, const std::string& APILicense);
	static int BreakListen();
	static int BreakConnect();
	static int Check(int sid);

	static int Initialize(std::string param);
	static int DeInitialize();
	static int CloseSid(int& sid, bool force = true);

	static int Connect(std::string uid, BYTE flags = P2P_CONNECT_FLAGS(1, 1, 1, 0x0E));

	static int GetOutboxBytes(int sid);
	static int GetInboxBytes(int sid);

	static int iPN_StringEnc(const char *keystr, const char *src, char *dest, unsigned int maxsize);
	static int iPN_StringDnc(const char *keystr, const char *src, char *dest, unsigned int maxsize);

#ifdef _CONFIG_TEST_P2P
	int TestServer();
	int TestClient();
#endif

	static const char *GetErrorDesc(int error);

protected:
	int OnAccept(int sid);

	static int Write(int sid, LPVOID data, int dataBytes);
	static int Read(int sid, LPVOID buf, int bufBytes);

protected:
#ifdef _CONFIG_TEST_P2P
	std::string mUid;//p2p device id
#endif
};
}
}
}
}
