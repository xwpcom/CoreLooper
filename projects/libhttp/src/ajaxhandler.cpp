#include "stdafx.h"
#include "ajaxhandler.h"
using namespace Bear::Core;
namespace Bear {
namespace Core {
namespace Net {
namespace Http {

AjaxRuntimeClass AjaxHandler::classAjaxHandler;
map<string, AjaxRuntimeClass*>* AjaxRuntimeClass::m_mapPackClass = nullptr;
static const char* TAG = "AjaxHandler";
AjaxClassInit::AjaxClassInit(AjaxRuntimeClass* pData)
{
	{
		static bool first = true;
		if (first)
		{
			first = false;
			atexit(AjaxHandler::Destroy);//atexit非常好用
		}
	}

	AjaxRuntimeClass *pd = &AjaxHandler::classAjaxHandler;
	if (!pd->m_mapPackClass)
	{
		pd->m_mapPackClass = new map<string, AjaxRuntimeClass*>;
		if (!pd->m_mapPackClass)
		{
			ASSERT(FALSE);
			return;
		}
	}

	map<string, AjaxRuntimeClass*>& mapPackClass = *pd->m_mapPackClass;
	if (mapPackClass[pData->mCommandName] != nullptr)
	{
		DW("item exits:%s,please check IMPLEMENT_AJAX_CLASS use unique command name", pData->mCommandName.c_str());
		ASSERT(FALSE);
	}
	mapPackClass[pData->mCommandName] = pData;
}

void AjaxHandler::Destroy()
{
	AjaxRuntimeClass *pd = &AjaxHandler::classAjaxHandler;
	if (pd->m_mapPackClass)
	{
		delete pd->m_mapPackClass;
		pd->m_mapPackClass = nullptr;
	}
}

AjaxHandler* AjaxHandler::CreateInstance(string commandName)
{
	const AjaxRuntimeClass *pd = &AjaxHandler::classAjaxHandler;

	if (!pd->m_mapPackClass)
	{
		DW("no found:%s", commandName.c_str());
		//ASSERT(FALSE);
		return NULL;
	}

	auto iter = pd->m_mapPackClass->find(commandName);
	if (iter != pd->m_mapPackClass->end() && iter->second->m_pCreateInstance)
	{
		auto handler = iter->second->m_pCreateInstance();
		return handler;
	}

	LogV(TAG,"no implement for %s", commandName.c_str());
	return nullptr;
}

}
}
}
}
