#include "stdafx.h"
#include "auth/userman.h"
#include "libcrypt/base64ex.h"
#include "libcrypt/tea.h"
#include "string/textprotocol.h"
#include "looper/looper.h"
#ifndef __APPLE__
//#include "src/system/PlatformHandler.h"
#endif
using namespace Bear;
using namespace Bear::Core;

#define IDS_USR "user"

//判断字符串是否可以当作有效的用户名
//用户名不能为空,不能包含\r,\n,并且有长度限制
BOOL UserMan::IsValidUsrString(const string& sz)
{
	if (sz.empty()
		|| sz.find("\r") != -1
		|| sz.find("\n") != -1
		|| sz.find(":") != -1
		)
	{
		return FALSE;
	}

	int len = (int)sz.length();
	return len<=31;
}

//判断字符串是否可以当作有效的密码
//密码不能包含\r,\n,并且有长度限制
BOOL UserMan::IsValidPwdString(const string& sz)
{
	if (sz.find("\r") != -1
		|| sz.find("\n") != -1
		|| sz.find(":") != -1
		)
	{
		return FALSE;
	}

	int len = (int)sz.length();
	return len<=31;
}

UserMan::UserMan()
{
	m_pcb=NULL;
	SetObjectName("UserMan");
}

UserMan::~UserMan()
{
	EmptyUserInfo();
}

void UserMan::EmptyUserInfo()
{
	mUserInfoList.clear();
}

eUserLevel UserMan::GetUserLevel(const string& usr,const string& pwd)
{
	shared_ptr<IniFile>ini= GetIni();

	if(usr.empty() && pwd.empty())
	{
		//匿名用户
		return (eUserLevel)ini->GetInt(IDS_USR,"anonymous_Level",eUser_Invalid);
	}

	for (auto iter = mUserInfoList.begin(); iter != mUserInfoList.end(); ++iter)
	{
		if(iter->name == usr)
		{
			if (iter->password == pwd)
			{
				return iter->level;
			}

			return eUser_Invalid;
		}
	}

	return eUser_Invalid;
}

//当加载配置文件或者修改用户信息之后要调用LoadUserInfo()缓存用户信息
int UserMan::LoadUserInfo()
{
	shared_ptr<IniFile> ini = GetIni();
	if (!ini)
	{
		return -1;
	}

	//解析用户信息
	EmptyUserInfo();

	Tea tea;
	tea.SetPassword("device" + string(".") + "password");

	int nc=ini->GetInt("user","count");
	if(nc>0)
	{
		for(int i=0;i<nc;i++)
		{
			string section=StringTool::Format("user%d",i);

			tagUserInfo ui;
			ui.id=ini->GetInt(section,"id");
			ui.name=ini->GetString(section,"name");
			string text = ini->GetString(section, "password");
			ui.password=tea.DecodeTextWithBase64(text);
			ui.level=(eUserLevel)ini->GetInt(section,"level");

			if (IsValidUsrString(ui.name) && IsValidPwdString(ui.password))
			{
				//DV("user[%d]=%s,text=[%s],password=[%s],level=%d", i, ui.name.c_str(), text.c_str(),ui.password.c_str(),ui.level);
				mUserInfoList.push_back(ui);
			}
		}
	}
	
	if(mUserInfoList.size()==0)
	{
		//添加默认管理员账号
		tagUserInfo ui;
		ui.id=0;
		ui.name="admin";
		ui.password="admin";
		ui.level=eUser_Admin;

		mUserInfoList.push_back(ui);
		SyncUserInfo(ui.name);
	}

	return 0;
}

//http,rtsp修改user info之后，必须调用SyncUserInfo()把数据提交给IniFile
//输入参数:usrSrc表示是此用户的信息发生了改变
int UserMan::SyncUserInfo(const string& usrSrc)
{
	SaveUserInfo();

	if(m_pcb && !usrSrc.empty())
	{
		m_pcb->OnUserChanged(usrSrc);
	}
	
	return 0;
}

//除user之外，是否还存在其他管理员?
//在修改权限，删除用户时，要保证执行操作之后至少还存在一个管理员用户
BOOL UserMan::IsExistAdminExclude(string user)
{
	for(auto iter= mUserInfoList.begin();iter!= mUserInfoList.end();++iter)
	{
		if (user==iter->name)
			continue;
		if(iter->level == eUser_Admin)
			return TRUE;
	}
	
	return FALSE;
}

//根据用户名得到tagUserInfo
tagUserInfo *UserMan::FindUserInfo(const string& usr)
{
	for (auto iter = mUserInfoList.begin(); iter != mUserInfoList.end(); ++iter)
	{
		if (usr == iter->name)
		{
			return &*iter;
		}
	}
	return NULL;
}

//要与enum eUserLevel同步
eUserLevel UserMan::String2UserLevel(const char *psz)
{
	if(StringTool::CompareNoCase(psz,"Admin")==0)
		return eUser_Admin;
	if(StringTool::CompareNoCase(psz,"Operator")==0)
		return eUser_Operator;
	if(StringTool::CompareNoCase(psz,"Guest")==0)
		return eUser_Guest;
	return eUser_Invalid;
}

//要与enum eUserLevel同步
const char *UserMan::UserLevel2String(eUserLevel userLevel)
{
	static const char *arr[]=
	{
		"Admin",
		"Operator",
		"Guest",
	};
	if(userLevel>=0 && userLevel<(int)COUNT_OF(arr))
	{
		return arr[userLevel];
	}
	return "Invalid";
}

eUserManError UserMan::AddUser(const string& usr,const string& pwd,eUserLevel level)
{
	if(!IsValidUsrString(usr)|| !IsValidPwdString(pwd))
	{
		return eUserManError_InvalidParam;
	}

	tagUserInfo *pui=FindUserInfo(usr);
	if(pui)
	{
		return eUserManError_UserExists;
	}

	//添加用户
	tagUserInfo ui;
	pui = &ui;
	pui->name=usr;
	pui->password=pwd;
	pui->level=level;
	//找到现存最大的id
	int max_id = 0;
	for (auto iter = mUserInfoList.begin(); iter != mUserInfoList.end(); ++iter)
	{
		if (max_id < iter->id)
		{
			max_id = iter->id;
		}
	}

	pui->id=max_id+1;
	mUserInfoList.push_back(ui);
	SyncUserInfo(usr);
	return eUserManError_Success;
}

//修改用户名和密码
eUserManError UserMan::EditUserPassword(const string& user, const string& password, const string& newUser, const string& newPassword)
{
	if (!IsValidUsrString(newUser) || !IsValidPwdString(newPassword))
	{
		return eUserManError_InvalidParam;
	}

	tagUserInfo *info = FindUserInfo(user);
	if (newUser != user)
	{
		if (!info || info->password != password)
		{
			return eUserManError_ActionNotPermitted;
		}

		tagUserInfo *info2 = FindUserInfo(newUser);
		if (info2)
		{
			return eUserManError_UserExists;
		}
	}

	if (info)
	{
		bool passwordChanged = (info->password != newPassword);

		info->name = newUser;
		info->password = newPassword;
		SaveUserInfo();
		/*
#ifndef __APPLE__
		{
			if (passwordChanged && info->name == IPCAM_PLATFORM_USER)
			{
				//重新登录平台以上报最新的密码
				shared_ptr<PlatformHandler> obj = dynamic_pointer_cast<PlatformHandler>(Looper::Object("Board/PlatformHandler"));
				if (obj)
				{
					obj->OnPlatformUserPasswordChanged();
				}
			}
		}
#endif
*/

		return eUserManError_Success;
	}

	return eUserManError_Unknown;
}

//修改usr的用户名和级别
eUserManError UserMan::EditUser(const string& usr,const string& pwd,eUserLevel level)
{
	if(!IsValidUsrString(usr)|| !IsValidPwdString(pwd))
	{
		return eUserManError_InvalidParam;
	}

	auto lst = mUserInfoList;
	tagUserInfo *pui=FindUserInfo(usr);
	if(pui)
	{
		pui->password=pwd;
		pui->level=level;
		SyncUserInfo(usr);
		return eUserManError_Success;
	}

	return eUserManError_NotExists;
}

eUserManError UserMan::DeleteUser(const string& usr)
{
	//不能删除最后一个管理员
	BOOL bFind = FALSE;
	for (auto iter = mUserInfoList.begin(); iter != mUserInfoList.end();++iter)
	{
		if (iter->name != usr)
		{
			continue;
		}

		bFind = TRUE;
		if(iter->level == eUser_Admin && !IsExistAdminExclude(iter->name))
		{
			//没有权限删除最后一个管理员
			return eUserManError_ActionNotPermitted;
		}

		mUserInfoList.erase(iter);
		SyncUserInfo(usr);
		return eUserManError_Success;
	}

	return eUserManError_NotExists;
}

void UserMan::OnCreate()
{
	__super::OnCreate();
	LoadUserInfo();
}

shared_ptr<IniFile> UserMan::GetIni()
{
	//shared_ptr<IniFile> ini = Looper::CurrentLooper()->GetIniFile();
	ASSERT(false);
	return nullptr;
}

int UserMan::SaveUserInfo()
{
	shared_ptr<IniFile> ini = GetIni();

	Tea tea;
	tea.SetPassword("device" + string(".") + "password");

	{
		//采用新格式保存
		ini->SetInt("user", "count", (int)mUserInfoList.size());

		int idx = -1;
		for (auto iter = mUserInfoList.begin(); iter != mUserInfoList.end(); ++iter)
		{
			++idx;
			string section=StringTool::Format("user%d", idx);

			ini->SetString(section, "name", iter->name.c_str());
			ini->SetString(section, "password", tea.EncodeTextWithBase64(iter->password).c_str());
			ini->SetInt(section, "id", iter->id);
			ini->SetInt(section, "level", iter->level);
		}
	}
	return 0;
}
