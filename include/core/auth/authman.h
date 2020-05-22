#pragma once
#include "userinfo.h"
namespace Bear {
namespace Core
{
#define IDS_ADMIN		"admin"
#define IDS_OPERATOR	"operator"
#define IDS_GUEST		"guest"

class UserMan;

//XiongWanPing 2013.07.04
class HTTP_EXPORT UserInfo :public IUserInfo
{
public:
	UserInfo();
	virtual ~UserInfo();

	void SetUserMan(std::shared_ptr<UserMan> man)
	{
		mUserMan = man;
	}

	//IUserInfo#begin
	void SetUserPassword(std::string user, std::string password);
	void SetUserLevel(eUserLevel userLevel);
	eUserLevel GetCurUserLevel()const
	{
		return mUserLevel;
	}

	std::string GetUserName()
	{
		return mUser;
	}
	std::string GetPassword()
	{
		return mPassword;
	}

	eUserLevel GetUserLevel(std::string user, std::string password);
	bool HasAuth(std::string action);
	bool IsAuthAction(const char *pszAction, const char *pszUserGroup = NULL);
	bool IsAuthAction(std::string action, std::string userGroup = "")
	{
		return IsAuthAction(action.c_str(), userGroup.c_str());
	}
	//IUserInfo#end

protected:
	std::string			mUser;
	std::string			mPassword;
	eUserLevel			mUserLevel;
	std::shared_ptr<UserMan> mUserMan;
};
}
}