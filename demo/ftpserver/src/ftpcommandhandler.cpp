#include "stdafx.h"
#include "ftpcommandhandler.h"
#include "ftpprotocol.h"
#include "net/channel.h"
#include "net/nettool.h"
#include "ftpdataserver.h"
#include "ftpdatainhandler.h"
#include "datasource_buffer.h"
#include "datasource_file.h"
#include "datasink_file.h"

using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Bear::Core::Net;
using namespace Bear::Core::Net::Ftp;

FtpCommandHandler::FtpCommandHandler()
{
	SetObjectName("FtpCommandHandler");

	mOutbox.PrepareBuf(1024 * 16);
	UpdateTickAlive();

	DW("%s(%p)", __func__, this);
}

FtpCommandHandler::~FtpCommandHandler()
{
	DW("%s(%p)", __func__, this);
}

struct tagFtpHandler
{
	const char*	action;
	int (FtpCommandHandler::*func)(const string& param);
	bool needLogin;
};


void FtpCommandHandler::OnTimer(long id)
{
	if(id == mTimerCheckAlive)
	{
		auto tickNow = ShellTool::GetTickCount64();
		int second = 180;
#ifdef _DEBUG
		second = 30;
#endif
		if (tickNow > mTickAlive + second * 1000)
		{
			if (mDataEndPoint)
			{
				mDataEndPoint->Close();
			}
		}

		return;
	}

	__super::OnTimer(id);
}

void FtpCommandHandler::UpdateTickAlive()
{
	mTickAlive = ShellTool::GetTickCount64();
}

void FtpCommandHandler::OnConnect(Channel*, long error, Bundle*)
{
	if (error == 0)
	{
		int second = 180;
#ifdef _DEBUG
		second = 30;
#endif
		SetTimer(mTimerCheckAlive,second * 1000);
		UpdateTickAlive();

		ASSERT(mConfig);
		mCurrentPath = mConfig->mVirtualFolder.GetVirtualRootPath();

		ASSERT(!mProtocol);
		mProtocol = make_shared<FtpProtocol>(this);
		mProtocol->OnConnect();
	}
}

void FtpCommandHandler::OnClose(Channel*)
{
	if (mDataEndPoint)
	{
		mDataEndPoint->Destroy();
		mDataEndPoint = nullptr;
	}
	
	Destroy();
}

void FtpCommandHandler::OnSend(Channel*)
{
	CheckSend();
}

void FtpCommandHandler::CheckSend()
{
	while (mDataEndPoint)
	{
		if (mOutbox.GetActualDataLength() == 0)
		{
			return;
		}

		LPBYTE frame = mOutbox.GetDataPointer();
		int frameLen = mOutbox.GetActualDataLength();
		int ret = mDataEndPoint->Send(frame, frameLen);
		if (ret > 0)
		{
			mOutbox.Eat(ret);

			if (ret < frameLen)
			{
				//只发了一部分,mOutbox中没发完的数据下次会再发送
				return;
			}
		}
		else
		{
			//发送出错
			return;
		}
	}
}

void FtpCommandHandler::OnReceive(Channel*)
{
	BYTE buf[4 * 1024];
	while (mDataEndPoint)
	{
		int ret = mDataEndPoint->Receive(buf, sizeof(buf) - 1);
		if (ret <= 0)
		{
			return;
		}

		buf[ret] = 0;
		UpdateTickAlive();

		if (mProtocol)
		{
			mProtocol->Input((char*)buf, ret);
		}
	}

}

int FtpCommandHandler::Output(LPVOID data, int bytes)
{
	auto ret = -1;
	if (mDataEndPoint)
	{
		ret=mDataEndPoint->Send(data, bytes);
	}
	else
	{
		DW("mDataEndPoint is null,skip send %d bytes", bytes);
	}

	return ret;
}

int FtpCommandHandler::OnCommand(const string& cmd)
{
	{
		++mCommandIndex;
		DV("FtpCommandHandler(%p).cmd[%04d]=[%s]",this,mCommandIndex, cmd.c_str());
	}

	string action, param;
	auto pos = cmd.find(' ');
	if (pos == -1)
	{
		action = cmd;
	}
	else
	{
		action = cmd.substr(0, pos);
		param = cmd.substr(pos + 1);
	}


	static struct tagFtpHandler items[] =
	{
#undef ITEM
#define ITEM_NO_AUTH(x) #x,&FtpCommandHandler::OnCommand_##x,false,
#define ITEM(x) #x,&FtpCommandHandler::OnCommand_##x,true,
		ITEM_NO_AUTH(USER)
		ITEM_NO_AUTH(PASS)
		ITEM(SYST)
		ITEM(PWD)
		ITEM(REST)
		ITEM(TYPE)
		ITEM(PASV)
		ITEM(LIST)
		ITEM(DELE)
		ITEM(NOOP)
		ITEM(CWD)
		ITEM(CDUP)
		ITEM(RETR)
		ITEM(STOR)
		ITEM(MKD)
		ITEM(RMD)
		ITEM(RNFR)
		ITEM(RNTO)
		nullptr,nullptr,false,
	};

	auto ret = -1;
	bool found = false;
	for (tagFtpHandler *handler = &items[0]; handler->action; handler++)
	{
		if (action == handler->action)
		{
			if (handler->needLogin && !mLogined)
			{
				Output("530 need login\r\n");
				return 0;
			}

			found = true;
			ret = (this->*handler->func)(param);
			break;
		}
	}

	if (!found)
	{
		Output("500 unknown command\r\n");
	}
	return 0;
}

int FtpCommandHandler::OnCommand_USER(const string& param)
{
	mUser = param;
	mLogined = false;

	Output("331 need password\r\n");
	return 0;
}

int FtpCommandHandler::OnCommand_PASS(const string& param)
{
	auto password = param;
	bool ok = true;//for demo skip check
	if (ok)
	{
		mLogined = true;
		Output("230 login ok\r\n");
	}
	else
	{
		Output("530 login fail\r\n");
	}

	return 0;
}

int FtpCommandHandler::OnCommand_SYST(const string& param)
{
	Output("215 FtpServer is a Looper demo.\r\n");
	return 0;
}

int FtpCommandHandler::OnCommand_PWD(const string& param)
{
	Output(StringTool::Format("257 \"%s\" is current directory.\r\n", mCurrentPath.c_str()));
	return 0;
}

int FtpCommandHandler::OnCommand_REST(const string& param)
{
	//todo:restart transfer
	//ASSERT(FALSE);
	Output("200 OK to REST\r\n");
	return 0;
}

int FtpCommandHandler::OnCommand_TYPE(const string& param)
{
	Output("200 OK\r\n");
	return 0;
}

int FtpCommandHandler::OnCommand_NOOP(const string& param)
{
	Output("200 OK\r\n");
	return 0;
}

//返回虚拟绝对目录
string FtpCommandHandler::GetFullVirtualPathFile(const char *pszPathFile)
{
	if (!pszPathFile || pszPathFile[0] == 0)
	{
		return mCurrentPath;
	}
	if (File::IsPathSplitChar(pszPathFile[0]))
	{
		return pszPathFile;
	}

	string fullPath;
	if (mCurrentPath.empty())
	{
		ASSERT(FALSE);
		fullPath = pszPathFile;
	}
	else
	{
		int len = (int)mCurrentPath.length();
		BOOL bSplitChar = File::IsPathSplitChar(mCurrentPath[len - 1]);
		fullPath = StringTool::Format("%s%s%s",
			mCurrentPath.c_str(),
			bSplitChar ? "" : FILE_PATH_SPLIT,
			pszPathFile
		);
	}

	return fullPath;
}

int FtpCommandHandler::OnCommand_PASV(const string& param)
{
	int port = 0;
	auto obj = mDataServer.lock();
	if (!obj)
	{
		obj=make_shared<FtpDataServer>();
		mDataServer = obj;

		DV("%p.create FtpDataServer(%p)", this, obj.get());

		auto ret = obj->StartServer(0);
		ASSERT(ret == 0);

		AddChild(obj);
		obj->SignalOnConnect.connect(this, &FtpCommandHandler::OnDataHandlerConnect);

		DV("FtpDataServer port=%d", obj->GetPort());
	}

	port = obj->GetPort();
	string ip;
	auto desc=mDataEndPoint->GetLocalDesc();
	auto pos = desc.find(':');
	if (pos != -1)
	{
		ip = desc.substr(0, pos);
		StringTool::Replace(ip, ".", ",");
	}
	auto sz = StringTool::Format("227 Entering Passive Mode (%s,%d,%d).\r\n", ip.c_str(), port / 256, port % 256);
	Output(sz);
	return 0;
}

int FtpCommandHandler::OnCommand_LIST(const string& param)
{
	int port = 0;
	auto obj = mDataServer.lock();

	if (!obj)
	{
		Output("450 send PASV first\r\n");
		return 0;
	}
	
	obj->SetMode(eFtpDataMode_Out);
	Output("150 ASCII mode directory list.\r\n");

	SwitchDataStatus(eDataHandler_LIST);
	return 0;
}

//pszSvrPathFile是服务器本地path
string FtpCommandHandler::GetFtpListFileInfo(const char *pszSvrPathFile, const char *title)
{
	string ack;
	//ack.SetGrowBy(256);

	int ret = -1;

	BOOL bDir = File::PathIsDirectory(pszSvrPathFile);
	ack += (bDir) ? "drwx------" : "-rwx------";
	//ack+=" 1 ipcam group ";// groups
	ack += " 1 ftp ftp ";

	char file[MAX_PATH];
	memset(file, 0, sizeof(file));
	strncpy(file, pszSvrPathFile, sizeof(file) - 1);
	File::RemoveTailPathSplitChar(file);

#ifdef _MSC_VER	
	struct _stat32 fs = { 0 };
	ret = _stat32(file, &fs);
#else
	struct _stat fs = { 0 };
	ret = _stat(file, &fs);
#endif

	time_t timet = fs.st_mtime;
	struct tm *t = localtime(&timet);
	if (t)
	{
		struct tm tt = *t;

		ret = 0;
		ack += StringTool::Format("% 14d ", fs.st_size);

		char szTime[100];
		{
			szTime[0] = 0;
			//SYSTEMTIME sLocalTime;
			//GetLocalTime(&sLocalTime);
			struct tm sLocalTime = ShellTool::GetLocalTime();

			int fileYear = tt.tm_year + 1900;
			int sysYear = sLocalTime.tm_year + 1900;
			int sysMonth = sLocalTime.tm_mon + 1;
			if (fileYear>sysYear
				|| ((fileYear == sysYear) && (tt.tm_mon + 1>sysMonth))
				|| ((fileYear == sysYear) && (tt.tm_mon + 1>sysMonth))
				)
			{
				//XiongWanPing 2011.06.01
				//IE太TM智能了，如果文件日期超过当前系统时间，IE会自动“纠正"文件时间
				//所以当文件日期超过当前系统时间只返回年月日
			}
			else if (fileYear == sysYear)
			{
				//只有在文件修改日期年份与当前年份相同时，修改时间才能精确到分钟
				strftime(szTime, sizeof(szTime), "%b %d %H:%M ", &tt);
			}

			if (szTime[0] == 0)
			{
				strftime(szTime, sizeof(szTime), "%b %d %Y ", &tt);
			}
		}

		ack += szTime;

		if (title)
		{
			ack += title;
		}
		else
		{
			string name = File::GetPathLastName(pszSvrPathFile);
			ack += name;
		}
		//DT("[%s]=[%s]",name.c_str(),pszSvrPathFile);
		ack += "\r\n";
	}

	return (ret == 0) ? ack : "";
}

string FtpCommandHandler::GetDirectoryList(const string& localPath)
{
	string ack;
	//ack.SetGrowBy(2048);

	if (localPath == mConfig->mVirtualFolder.GetVirtualRootPath())
	{
		list<shared_ptr<tagVirtualFolderPoint>>& lst = mConfig->mVirtualFolder.GetMountPoints();
		for (auto iter = lst.begin(); iter != lst.end(); ++iter)
		{
			string fi = GetFtpListFileInfo((*iter)->mPath.c_str(), (*iter)->mName.c_str());
			ack += fi;
		}
	}
	else
	{
		char szFolder[MAX_PATH + 1];
		strncpy(szFolder, localPath.c_str(), sizeof(szFolder) - 10);
		File::PathMakePretty(szFolder);

#ifdef WIN32
		HANDLE hFindFile = NULL;
		BOOL bFound = TRUE;
		WIN32_FIND_DATAA fd = { 0 };

		char szFind[MAX_PATH + 1];
		memset(szFind, 0, sizeof(szFind));
		strcpy(szFind, szFolder);
		strcat(szFind, "/*.*");
		File::PathMakePretty(szFind);

		hFindFile = FindFirstFileA(szFind, &fd);
		bFound = (hFindFile != INVALID_HANDLE_VALUE);

		while ((bFound = FindNextFileA(hFindFile, &fd)) != 0)
		{
			if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
				continue;

			string fi = GetFtpListFileInfo((string(szFolder) + "/" + fd.cFileName).c_str());
			//DT("%s", fi.c_str());
			ack += fi;
		}

		::FindClose(hFindFile);
		hFindFile = NULL;
#else
		struct dirent *entry = NULL;
		//DT("opendir[%s]\n",strLocalPath);
		DIR *dir = opendir(localPath.c_str());
		if (dir == NULL)
		{
			return "";
		}

		//DT("readdir,dir=0x%x\n",dir);
		while (TRUE)
		{
			//DT("readdir#1\n");
			entry = readdir(dir);
			//DT("readdir#2\n");

			if (entry == NULL)
				break;

			//DT("entry=0x%x\n",entry);
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			{
				continue;
			}

			string fi = GetFtpListFileInfo(string(szFolder) + "/" + entry->d_name);
			//DT("%s",fi.c_str());
			ack += fi;
		}
		closedir(dir);
		//*/
#endif
	}

	return ack;
}

#ifdef _MSC_VER
void FtpCommandHandler::SystemTimeToTimet(SYSTEMTIME st, time_t *pt)
{
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	LONGLONG ll;

	ULARGE_INTEGER ui;
	ui.LowPart = ft.dwLowDateTime;
	ui.HighPart = ft.dwHighDateTime;

	LONGLONG d = ft.dwHighDateTime;
	ll = (d << 32) + ft.dwLowDateTime;

	*pt = (DWORD)((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
}
#endif

const char *FtpCommandHandler::GetDataStatusDesc(eDataStatus value)
{
#undef ITEM
#define ITEM(x)	(char*)x,#x
	static char *arr[] =
	{
		ITEM(eDataHandler_NONE),
		ITEM(eDataHandler_LIST),
		ITEM(eDataHandler_RETR),
		ITEM(eDataHandler_STOR),
	};

	for (int i = 0; i < COUNT_OF(arr); i += 2)
	{
		if (value == (ULONGLONG)arr[i])
			return arr[i + 1];
	}

	ASSERT(FALSE);
	return "Unknown";
}

void FtpCommandHandler::SwitchDataStatus(eDataStatus status)
{
	if (mDataStatus != status)
	{
		DW("%p.%s(%s)",this, __func__, GetDataStatusDesc(status));
		mDataStatus = status;

		if (status != eDataHandler_NONE)
		{
			auto obj = mDataHandler.lock();
			if (obj)
			{
				mDataHandler.reset();

				OnDataHandlerConnect(obj);
				SwitchDataStatus(eDataHandler_NONE);
			}
		}
	}
}

void FtpCommandHandler::OnDataHandlerConnect(shared_ptr<FtpDataHandler> obj)
{
	{
		static int idx = -1;
		++idx;
		DV("FtpCommandHandler(%p).OnDataHandlerConnect,idx=%04d", this, idx);
	}
	//ASSERT(mDataStatus != eDataHandler_NONE);

	switch (mDataStatus)
	{
	case eDataHandler_LIST:
	{
		string fullVirtualPath = GetFullVirtualPathFile("");
		string localPath = mConfig->mVirtualFolder.Virtual2LocalPathFile(fullVirtualPath, false);
		string ack = GetDirectoryList(localPath);

		obj->SignalSendDone.connect(this, &FtpCommandHandler::OnDataTransferDone);

		{
			auto handler = make_shared<DataSource_Buffer>();
			handler->SetData(ack);
			handler->SignalDataChanged.connect(obj.get(), &FtpDataHandler::OnDataChanged);

			obj->AddChild(handler);
			obj->SetDataSource(handler);
		}

		break;
	}
	case eDataHandler_RETR:
	{
		string path = mParam_RETR;
		string fullVirtualPathFile = GetFullVirtualPathFile(path.c_str());
		string localFile = mConfig->mVirtualFolder.Virtual2LocalPathFile(fullVirtualPathFile, false);
		Output("150 Opening BINARY mode data connection for file transfer.\r\n");
		
		obj->SignalSendDone.connect(this, &FtpCommandHandler::OnDataTransferDone);
		{
			auto handler = make_shared<DataSource_File>();
			auto ret=handler->SetFilePath(localFile);
			if (ret == 0)
			{
				handler->SignalDataChanged.connect(obj.get(), &FtpDataHandler::OnDataChanged);

				obj->AddChild(handler);
				obj->SetDataSource(handler);
			}
			else
			{
				Output("550 fail,transfer aborted\r\n");
			}
		}
		break;
	}
	case eDataHandler_STOR:
	{
		string path = mParam_STOR;
		string fullVirtualPathFile = GetFullVirtualPathFile(path.c_str());
		string localFile = mConfig->mVirtualFolder.Virtual2LocalPathFile(fullVirtualPathFile, false);
		Output("150 Opening BINARY mode data connection for file transfer.\r\n");

		obj->SignalSinkDone.connect(this, &FtpCommandHandler::OnDataTransferDone);
		{
			auto handler = make_shared<DataSink_File>();
			auto ret = handler->SetFilePath(localFile);
			if (ret == 0)
			{
				//handler->SignalDataChanged.connect(obj.get(), &FtpDataInHandler::OnDataChanged);

				obj->AddChild(handler);
				obj->SetDataSink(handler);
			}
			else
			{
				Output("550 fail,transfer aborted\r\n");
			}
		}

		break;
	}
	default:
	{
		ASSERT(!mDataHandler.lock());

		//socket创建时还没收到client发送的LIST,STOR或RETR命令，这里缓存，等收到命令后再处理
		mDataHandler = obj;
		break;
	}
	}
}

void FtpCommandHandler::OnDataTransferDone(FtpDataHandler* obj)
{
	//ASSERT(mDataStatus == eDataHandler_LIST || mDataStatus == eDataHandler_RETR);
	Output("226 Transfer complete\r\n");
	SwitchDataStatus(eDataHandler_NONE);
}

int FtpCommandHandler::DoChangeDirectory(const string& dir)
{
	if (dir.empty())
	{
		return 0;
	}
	if (dir == mConfig->mVirtualFolder.GetVirtualRootPath())
	{
		mCurrentPath = dir;
		return 0;
	}

	if (dir == "..")
	{
		int len = (int)mCurrentPath.length();
		for (int i = len - 1; i >= 0; i--)
		{
			if (File::IsPathSplitChar(mCurrentPath[i]))
			{
				string newDir = mCurrentPath.substr(0, i);
				if (i == 0)
				{
					mCurrentPath = mConfig->mVirtualFolder.GetVirtualRootPath();
				}
				else
				{
					mCurrentPath = newDir;
				}
				return 0;
			}
		}
		return 0;
	}

	string clientFullPath;
	if (File::IsPathSplitChar(dir[0]))
	{
		clientFullPath = dir;
	}
	else
	{
		clientFullPath = File::ConcatPath(mCurrentPath.c_str(), dir.c_str());
	}

	BOOL bPathValid = FALSE;
	list<shared_ptr<tagVirtualFolderPoint>>& lst = mConfig->mVirtualFolder.GetMountPoints();
	for (auto iter = lst.begin(); iter != lst.end(); ++iter)
	{
		string name0 = "/" + (*iter)->mName;
		string name1 = "/" + (*iter)->mName + "/";
		if (clientFullPath == name0 || clientFullPath.find(name1) == 0)
		{
			bPathValid = TRUE;
			break;
		}
	}

	if (bPathValid)
	{
		string svrPath = mConfig->mVirtualFolder.Virtual2LocalPathFile(dir, false);
		BOOL bDir = File::PathIsDirectory(svrPath.c_str());
		if (bDir)
		{
			mCurrentPath = clientFullPath;
			return 0;
		}
	}

	return -1;
}

int FtpCommandHandler::OnCommand_CDUP(const string& param)
{
	int ret = DoChangeDirectory("..");
	string ack;
	if (ret == 0)
	{
		Output(StringTool::Format("250 \"%s\" is current directory.\r\n", mCurrentPath.c_str()));
	}
	else
	{
		Output("550 Permission denied.\r\n");
	}
	return 0;
}

int FtpCommandHandler::OnCommand_RETR(const string& param)
{
	int port = 0;
	auto obj = mDataServer.lock();

	if (!obj)
	{
		Output("450 send PASV first\r\n");
		return 0;
	}

	obj->SetMode(eFtpDataMode_Out);
	mParam_RETR = param;
	SwitchDataStatus(eDataHandler_RETR);
	return 0;
}

int FtpCommandHandler::OnCommand_STOR(const string& param)
{
	int port = 0;
	auto obj = mDataServer.lock();

	if (!obj)
	{
		Output("450 send PASV first\r\n");
		return 0;
	}
	
	obj->SetMode(eFtpDataMode_In);
	mParam_STOR = param;
	SwitchDataStatus(eDataHandler_STOR);
	return 0;
}

int FtpCommandHandler::OnCommand_CWD(const string& param)
{
	auto& path = param;
	if (path == "..")
	{
		OnCommand_CDUP("CDUP");
		return 0;
	}

	string fullVirtualPath = GetFullVirtualPathFile(path.c_str());
	string localPath = mConfig->mVirtualFolder.Virtual2LocalPathFile(fullVirtualPath, false);
	BOOL bDir = File::PathIsDirectory(localPath.c_str());
	string ack;
	if (bDir)
	{
		mCurrentPath = fullVirtualPath;
		ack += StringTool::Format("250 \"%s\" is current directory\r\n", mCurrentPath.c_str());
	}
	else
	{
		ack += "550 fail\r\n";
	}

	Output(ack);
	return 0;
}

#ifndef _MSC_VER
#define _unlink unlink
#endif

int FtpCommandHandler::OnCommand_DELE(const string& param)
{
	string fullVirtualPathFile = GetFullVirtualPathFile(param.c_str());
	string localFile = mConfig->mVirtualFolder.Virtual2LocalPathFile(fullVirtualPathFile, false);

	if (_unlink(localFile.c_str()) == 0)
	{
		Output("250 delete ok\r\n");
	}
	else
	{
		Output("550 fail delete\r\n");
	}

	return 0;
}

int FtpCommandHandler::OnCommand_MKD(const string& param)
{
	string fullVirtualPath = GetFullVirtualPathFile(param.c_str());
	string szLocalPath = mConfig->mVirtualFolder.Virtual2LocalPathFile(fullVirtualPath, false);
	if (File::MakeSureDirectoryPathExists(szLocalPath.c_str()))
	{
		Output("250 ok\r\n");
	}
	else
	{
		Output("450 fail create directory.\r\n");
	}

	return 0;
}

int FtpCommandHandler::OnCommand_RMD(const string& param)
{
	string virtualPath = GetFullVirtualPathFile(param.c_str());
	string localPath = mConfig->mVirtualFolder.Virtual2LocalPathFile(virtualPath, false);
	if (!mConfig->mVirtualFolder.IsVirtualRootDir(virtualPath.c_str(), false)
		&& File::RemoveDirectory(localPath.c_str()))
	{
		Output("250 ok\r\n");
	}
	else
	{
		Output("450 Fail RemoveDirectory\r\n");
	}

	return 0;
}

//rename由RNFR和RNTO共同完成
int FtpCommandHandler::OnCommand_RNFR(const string& param)
{
	string oldVirtualName = GetFullVirtualPathFile(param.c_str());
	string oldLocalName = mConfig->mVirtualFolder.Virtual2LocalPathFile(oldVirtualName, false);
	if (mConfig->mVirtualFolder.IsVirtualRootDir(oldVirtualName.c_str(), false))
	{
		Output("450 fail\r\n");
		return 0;
	}

	Output("350 send newFileName please\r\n");
	mParam_RenameFrom = oldLocalName;
	return 0;
}

int FtpCommandHandler::OnCommand_RNTO(const string& param)
{
	string newVirtualName = GetFullVirtualPathFile(param.c_str());
	string newLocalName = mConfig->mVirtualFolder.Virtual2LocalPathFile(newVirtualName, false);

	char Old[MAX_PATH] = { 0 };
	char New[MAX_PATH] = { 0 };
	strncpy(Old, mParam_RenameFrom.c_str(), sizeof(Old));
	strncpy(New, newLocalName.c_str(), sizeof(New));
	File::PathMakePretty(Old);
	File::PathMakePretty(New);

	auto ret = rename(Old, New);
	if (ret == 0)
	{
		Output("200 ok\r\n");
	}
	else
	{
		Output("450 fail\r\n");
	}

	return 0;
}
