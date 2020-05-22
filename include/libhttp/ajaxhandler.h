#pragma once
#include "file/virtualfolder.h"
#include "xml/xmlerror.h"

namespace Bear {

namespace Core {
using namespace FileSystem;
class UserInfo;
namespace Net {
namespace Http {

class AjaxHandler;
typedef AjaxHandler* (*PFN_AjaxCreateInstance)();
struct HTTP_EXPORT AjaxRuntimeClass
{
	std::string					mCommandName;//ajax command name
	std::string					mPermission;
	std::string					mClassName;
	PFN_AjaxCreateInstance	m_pCreateInstance;

	static map<std::string, AjaxRuntimeClass*>	*m_mapPackClass;
};

class HTTP_EXPORT AjaxClassInit
{
public:
	AjaxClassInit(AjaxRuntimeClass* pData);
};

//XiongWanPing 2016.03.25
class HTTP_EXPORT AjaxHandler
{
	friend class AjaxClassInit;
	friend class AjaxCommandHandler;
public:
	AjaxHandler()
	{
	}
	virtual ~AjaxHandler()
	{
	}
	virtual std::string  Process(const NameValue& params) = 0;

	virtual std::string GetExtraHeader()const
	{
		return mExtraHeader;
	}

	static AjaxHandler* CreateInstance(std::string commandName);
	void SetVirtualFolder(std::shared_ptr<VirtualFolder> vm)
	{
		mVirtualFolder = vm;
	}
	void SetContext(std::shared_ptr<Handler> handler)
	{
		mContext = handler;
	}
	void SetUserInfo(std::shared_ptr<UserInfo> userInfo)
	{
		mUserInfo = userInfo;
	}

	void SetPort(int port)
	{
		mPort = port;
	}

protected:
	int mPort = 0;
	std::string mExtraHeader;
	std::shared_ptr<VirtualFolder>		mVirtualFolder;
	std::weak_ptr<Handler>		mContext;
	std::shared_ptr<UserInfo>			mUserInfo;

	static AjaxRuntimeClass classAjaxHandler;

#define _RUNTIME_PACK_CLASS(class_name) ((AjaxRuntimeClass*)(&class_name::class##class_name))
	virtual AjaxRuntimeClass* GetRuntimePackClass() const
	{
		return _RUNTIME_PACK_CLASS(AjaxHandler);
	}
protected:
	//采用atexit在app退出时自动调用释放内存,否则VS会提示memory leak
	static void __cdecl Destroy();
};

//下面的macro是用来把各类串起来
#define DELCARE_PACK_CLASS_HELP(class_name,base_class_name)					\
public:																		\
	static AjaxHandler * CreateInstance()									\
	{																		\
		AjaxHandler * handler=new class_name();								\
		return handler;														\
	}																		\
	protected:																\
	static AjaxClassInit class##class_name##_init;							\
	static AjaxRuntimeClass class##class_name;								\
	virtual AjaxRuntimeClass* GetRuntimePackClass() const;					\
	public:


#define IMPLEMENT_AJAX_CLASS(class_name,commandName,permission)				\
	AjaxRuntimeClass class_name::class##class_name =						\
	{																		\
		commandName,permission,#class_name,class_name::CreateInstance,		\
	};																		\
	AjaxClassInit class_name::class##class_name##_init(&class##class_name);	\
	AjaxRuntimeClass* class_name::GetRuntimePackClass() const				\
		{ return _RUNTIME_PACK_CLASS(class_name); }							\


#define DECLARE_AJAX_CLASS(class_name)										\
	DELCARE_PACK_CLASS_HELP(class_name,AjaxHandler)
}
}
}
}