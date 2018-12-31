#pragma once

#include "ftpprotocolcb.h"
#include "ftpconfig.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
class FtpProtocol;
class FtpDataHandler;
class FtpDataServer;

//XiongWanPing 2018.05.23
//
class FtpCommandHandler :public Handler, public FtpProtocolCB
{
	friend class FtpServer;
	SUPER(Handler);
public:
	FtpCommandHandler();
	~FtpCommandHandler();

	void SetConfig(shared_ptr<tagFtpServerConfig> config)
	{
		mConfig = config;
	}

protected:
	void OnTimer(long id);

	int Output(LPVOID data, int bytes);
	int Output(const char *ack)
	{
		return Output((LPVOID)ack, (int)strlen(ack));
	}
	int Output(const string& ack)
	{
		return Output((LPVOID)ack.c_str(), (int)ack.length());
	}
	void CheckSend();

	virtual void OnClose(Channel*);
	virtual void OnSend(Channel*);
	virtual void OnReceive(Channel*);

	virtual void OnConnect(Channel*, long, Bundle*);

	int OnCommand(const string& cmd);
	int OnCommand_USER(const string& param);
	int OnCommand_PASS(const string& param);
	int OnCommand_SYST(const string& param);
	int OnCommand_PWD(const string& param);
	int OnCommand_REST(const string& param);
	int OnCommand_TYPE(const string& param);
	int OnCommand_NOOP(const string& param);
	int OnCommand_PASV(const string& param);
	int OnCommand_LIST(const string& param);
	int OnCommand_DELE(const string& param);
	int OnCommand_CWD(const string& param);
	int OnCommand_CDUP(const string& param);
	int OnCommand_RETR(const string& param);
	int OnCommand_STOR(const string& param);
	int OnCommand_RMD(const string& param);
	int OnCommand_MKD(const string& param);
	int OnCommand_RNFR(const string& param);
	int OnCommand_RNTO(const string& param);

	int DoChangeDirectory(const string& dir);

	void OnDataHandlerConnect(shared_ptr<FtpDataHandler> obj);
	void OnDataTransferDone(FtpDataHandler* obj);

	ByteBuffer	mOutbox;
	shared_ptr<FtpProtocol> mProtocol;

	string	mUser;
	bool	mLogined = false;

	ULONGLONG mTickAlive = 0;
	void UpdateTickAlive();

	string GetFtpListFileInfo(const char *pszSvrPathFile, const char *title = nullptr);
	string GetDirectoryList(const string& localPath);
	string	GetFullVirtualPathFile(const char *pszPathFile);
	string	mCurrentPath;//mCurrentPath是相对于FtpServer的虚拟根目录的

#ifdef _MSC_VER
	void SystemTimeToTimet(SYSTEMTIME st, time_t *pt);
#endif

	shared_ptr<tagFtpServerConfig> mConfig;
	shared_ptr<Channel> mDataEndPoint;
	weak_ptr<FtpDataServer> mDataServer;

	//keep sync with GetDataStatusDesc
	enum eDataStatus
	{
#undef  ITEM
#define ITEM(x) x
		ITEM(eDataHandler_NONE),
		ITEM(eDataHandler_LIST),
		ITEM(eDataHandler_RETR),
		ITEM(eDataHandler_STOR),
	};

	eDataStatus mDataStatus = eDataHandler_NONE;
	void SwitchDataStatus(eDataStatus status);
	static const char *GetDataStatusDesc(eDataStatus status);

	string mParam_RETR;
	string mParam_STOR;
	string mParam_RenameFrom;

	weak_ptr<FtpDataHandler> mDataHandler;
	int mCommandIndex = -1;//for debug
	long mTimerCheckAlive = 0;
};
}
}
}
}