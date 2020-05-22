#pragma once

#include "auth/userinfo.h"
namespace Bear {
namespace Core
{
using namespace FileSystem;
struct tagUserInfo
{
	int			id;//用户唯一标识
	std::string		name;//限制唯一性
	std::string		password;
	eUserLevel	level;
};

class IUserManEventCB
{
public:
	virtual ~IUserManEventCB() {};
	virtual int OnUserChanged(const std::string& usr) = 0;
};

enum eUserManError
{
	eUserManError_Unknown = -1,
	eUserManError_Success = 0,
	eUserManError_InvalidParam,
	eUserManError_UserExists,
	eUserManError_ActionNotPermitted,
	eUserManError_NotExists,
};

//XiongWanPing 2012.01.19
//用户,账号管理
class HTTP_EXPORT UserMan :public Handler
{
	SUPER(Handler)
public:
	UserMan();
	~UserMan();

	void SetUserManEventCB(IUserManEventCB	*pcb)
	{
		m_pcb = pcb;
	}

	eUserManError AddUser(const std::string& usr, const std::string& pwd, eUserLevel level);
	eUserManError EditUser(const std::string& usr, const std::string& pwd, eUserLevel level);
	eUserManError DeleteUser(const std::string& usr);
	eUserManError EditUserPassword(const std::string& usr, const std::string& pwd, const std::string& newUsr, const std::string& newPassword);

	static eUserLevel String2UserLevel(const char *psz);
	static const char *UserLevel2String(eUserLevel userLevel);

	static BOOL IsValidUsrString(const std::string& sz);
	static BOOL IsValidPwdString(const std::string& sz);

	BOOL IsExistAdminExclude(std::string user);
	tagUserInfo *FindUserInfo(const std::string& usr);
	eUserLevel GetUserLevel(const std::string& usr, const std::string& pwd);
	int SyncUserInfo(const std::string& usrSrc);
protected:
	int LoadUserInfo();
	int SaveUserInfo();
	shared_ptr<IniFile> GetIni();
	void OnCreate();
	std::list<tagUserInfo> mUserInfoList;
	void EmptyUserInfo();

	IUserManEventCB	*m_pcb;
};
}
}