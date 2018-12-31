#pragma once

//XiongWanPing 2013.07.04
namespace Bear {
namespace Core
{
//要与UserLevel2String()和String2UserLevel()同步
enum eUserLevel
{
	eUser_Invalid = -1,
	eUser_Min = eUser_Invalid,
	eUser_Admin = 0,
	eUser_Operator,
	eUser_Guest,
	eUser_Max = eUser_Guest,
};


class HTTP_EXPORT IUserInfo
{
public:
	virtual ~IUserInfo() {};

	virtual void SetUserPassword(std::string user, std::string password) = 0;
	virtual void SetUserLevel(eUserLevel userLevel) = 0;
	virtual std::string GetUserName() = 0;

	virtual eUserLevel GetCurUserLevel()const = 0;
	virtual eUserLevel GetUserLevel(std::string user, std::string password) = 0;

	virtual bool IsAuthAction(const char *pszAction, const char *pszUserGroup) = 0;
	virtual bool HasAuth(std::string action) = 0;
};
}

}