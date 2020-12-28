#include "pch.h"
#include "LogFilterPage.h"
#include "afxdialogex.h"
#include "resource.h"
#include "logmanager.h"
#include "SelectItemPage.h"

/*
XiongWanPing 2020.02.05
*/

enum
{
	ID_APP_LAST= ID_APP_FIRST+256,
};

enum
{
	eTimer_countDown,
};

IMPLEMENT_DYNAMIC(LogFilterPage, BasePage)

LogFilterPage::LogFilterPage(CWnd* pParent /*=nullptr*/)
	: BasePage(IDD_LogFilterPage, pParent)
{

}

LogFilterPage::~LogFilterPage()
{
}

void LogFilterPage::DoDataExchange(CDataExchange* pDX)
{
	BasePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APP, mBtnApp);
	DDX_Control(pDX, IDC_CMB_LEVEL, mComboLevel);
}

BEGIN_MESSAGE_MAP(LogFilterPage, BasePage)
	ON_EN_CHANGE(IDC_EDIT_APP, &LogFilterPage::OnEnChangeEditApp)
	ON_EN_CHANGE(IDC_EDIT_TAG, &LogFilterPage::OnEnChangeEditTag)
	ON_EN_CHANGE(IDC_EDIT_PROCESS, &LogFilterPage::OnEnChangeEditProcess)
	ON_BN_CLICKED(IDC_CLEAR_FILTER, &LogFilterPage::OnBnClickedClearFilter)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_ON, &LogFilterPage::OnBnClickedCheckDefaultOn)
	ON_BN_CLICKED(IDC_CLEAR, &LogFilterPage::OnBnClickedClear)
	ON_WM_INITMENUPOPUP()
	ON_BN_CLICKED(IDC_APP, OnBtnApp)
	ON_BN_CLICKED(IDC_TAG, &LogFilterPage::OnBnClickedTag)
	ON_CBN_SELCHANGE(IDC_CMB_LEVEL, &LogFilterPage::OnCbnSelchangeCmbLevel)
	ON_BN_CLICKED(IDC_CHK_AUTO_SCROLL, &LogFilterPage::OnBnClickedChkAutoScroll)
END_MESSAGE_MAP()

void LogFilterPage::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	ShellTool::OnInitMenuPopupHelper(this, pPopupMenu, nIndex, bSysMenu);
}

struct tagLevelDesc
{
	const char* title;
	eDTLevel level;
};

static const tagLevelDesc gLevel[] =
{
	"verbose",DT_VERBOSE,
	"debug",DT_DEBUG,
	"info",DT_INFO,
	"warn",DT_WARN,
	"error",DT_ERROR,
};

int LogFilterPage::Init()
{
	ASSERT(mLogManager);

	{
		//mMenu.LoadMenu(IDR_MENU_FILTER);
		/*
		mBtnApp.m_hMenu = mMenu.GetSubMenu(0)->GetSafeHmenu();
		mBtnApp.SizeToContent();
		mBtnApp.m_bOSMenu = FALSE;
		mBtnApp.mProxy = this;
		*/
	}

	{
		auto& cmb = mComboLevel;

		USES_CONVERSION;
		for (auto i = 0; i < (int)COUNT_OF(gLevel); i++)
		{
			const char* item = gLevel[i].title;
			cmb.AddString(A2T(item));
		}

		LoadCombo(IDC_CMB_LEVEL, _T("level"));
		if (cmb.GetCurSel() == -1)
		{
			cmb.SetCurSel(0);
		}
	}

	__super::Init();

	RefreshFilter();
	return 0;
}

void LogFilterPage::LoadConfig()
{
	__super::LoadConfig();

	LoadDlgItemString(IDC_EDIT_APP, _T("app"));
	LoadDlgItemString(IDC_EDIT_TAG, _T("tag"));
	//LoadDlgItemString(IDC_EDIT_PROCESS, _T("process"));
	//LoadCheck(IDC_CHECK_DEFAULT_ON, _T("defaultOn")); 
	LoadCombo(IDC_CMB_LEVEL, _T("level"));
	LoadCheck(IDC_CHK_AUTO_SCROLL, _T("autoScroll"));

	mAutoScroll = IsDlgButtonChecked(IDC_CHK_AUTO_SCROLL);
}

void LogFilterPage::SaveConfig()
{
	__super::SaveConfig();

	SaveDlgItemString(IDC_EDIT_APP, _T("app"));
	SaveDlgItemString(IDC_EDIT_TAG, _T("tag"));
	SaveCombo(IDC_CMB_LEVEL, _T("level"));
	SaveCheck(IDC_CHK_AUTO_SCROLL, _T("autoScroll"));
	//SaveDlgItemString(IDC_EDIT_PROCESS, _T("process"));
	//SaveCheck(IDC_CHECK_DEFAULT_ON, _T("defaultOn"));
}

void LogFilterPage::OnEnChangeEditApp()
{
	RefreshFilter();
}

void LogFilterPage::OnEnChangeEditTag()
{
	RefreshFilter();
}

void LogFilterPage::OnEnChangeEditProcess()
{
	RefreshFilter();
}

void LogFilterPage::OnBnClickedCheckDefaultOn()
{
	RefreshFilter();
}

void LogFilterPage::RefreshFilter()
{
	auto& info = mFilterInfo;
	info.clear();

	USES_CONVERSION;
	auto apps = T2A(GetText(IDC_EDIT_APP));
	auto tags = T2A(GetText(IDC_EDIT_TAG));
	auto process = T2A(GetText(IDC_EDIT_PROCESS));
	//info.defaultOn = IsDlgButtonChecked(IDC_CHECK_DEFAULT_ON);
	auto sel = mComboLevel.GetCurSel();
	if (sel != -1)
	{
		mFilterInfo.level = gLevel[sel].level;
	}

	struct tagEditMap
	{
		string fields;
		unordered_map<string, bool>& mapItems;
		bool& onlyDisable;
	};

	tagEditMap arr[] = { 
		apps,info.apps,info.onlyDisableApp,
		tags,info.tags,info.onlyDisableTag,
	};

	for (size_t i = 0; i < COUNT_OF(arr); ++i)
	{
		auto& fields = arr[i].fields;
		vector<string> items;
		StringTool::ExtractSubString(fields, ',', items);

		auto& mapItems = arr[i].mapItems;
		for (auto& item : items)
		{
			if (item.empty())
			{
				continue;
			}

			auto pos = item.find('!');
			if (pos == 0)
			{
				mapItems[item.c_str() + 1] = false;
			}
			else
			{
				mapItems[item] = true;
				arr[i].onlyDisable = false;
			}
		}
	}

	//info.dump();

	SignalFilterChanged();
}

void LogFilterPage::OnBnClickedClearFilter()
{
	SetDlgItemText(IDC_EDIT_APP,_T(""));
	SetDlgItemText(IDC_EDIT_TAG, _T(""));
	//SetDlgItemText(IDC_EDIT_PROCESS, _T(""));
	//CheckDlgButton(IDC_CHECK_DEFAULT_ON, true);
	mComboLevel.SetCurSel(0);
	RefreshFilter();
}

int LogFilterPage::Filter(shared_ptr<LogItem> item)
{
	return mFilterInfo.Filter(item);
}

void LogFilterPage::tagFilter::dump()
{
	return;

	DV("filter.dump.begin");
	for (auto& item : apps)
	{
		DV("app[%s]=%d", item.first.c_str(), item.second);
	}
	for (auto& item : tags)
	{
		DV("tag[%s]=%d", item.first.c_str(), item.second);
	}

	//DV("defaultOn=%d", defaultOn);

	DV("filter.dump.end");
}

int LogFilterPage::tagFilter::Filter(shared_ptr<LogItem> item)
{
	if (item->level > level)
	{
		return -1;
	}

	if(!apps.empty())
	{
		auto iter = apps.find(item->appName);
		if (onlyDisableApp && iter == apps.end())
		{
			return 0;
		}

		if (iter == apps.end() || !iter->second)
		{
			return -1;
		}
	}

	if(!tags.empty())
	{
		auto iter = tags.find(item->tag);
		
		if (onlyDisableTag && iter == tags.end())
		{
			return 0;
		}

		if (iter == tags.end() || !iter->second)
		{
			return -1;
		}
	}

	return 0;
}

void LogFilterPage::OnBnClickedClear()
{
	SignalClearLog();
}

void LogFilterPage::OnApp(UINT id)
{

}

void LogFilterPage::OnUpdateApp(CCmdUI* pCmdUI)
{

}

BEGIN_MESSAGE_MAP(CMFCMenuButtonEx, CMFCMenuButton)
	ON_WM_SETFOCUS()
	ON_UPDATE_COMMAND_UI_RANGE(0, 0xFFFF, OnUpdateItem)
	ON_WM_INITMENUPOPUP()
END_MESSAGE_MAP()

void CMFCMenuButtonEx::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	ShellTool::OnInitMenuPopupHelper(this, pPopupMenu, nIndex, bSysMenu);
}

void CMFCMenuButtonEx::OnUpdateItem(CCmdUI* pCmdUI)
{
	if (mProxy)
	{
		mProxy->OnUpdateItem(pCmdUI);
	}
}

void LogFilterPage::OnUpdateItem(CCmdUI* pCmdUI)
{
	auto id = pCmdUI->m_nID;
	if (id >= ID_APP_FIRST && id<=ID_APP_LAST)
	{
		auto menu = pCmdUI->m_pMenu;

		if (id == ID_APP_FIRST)
		{
			auto& apps = mLogManager->apps();
			int i = 0;

			USES_CONVERSION;
			for (auto& app : apps)
			{
				DWORD flags = MF_ENABLED | MF_STRING;
				auto check = true;
				if (check)
				{
					flags |= MF_CHECKED;
				}
				menu->AppendMenu(flags, id + i+1, A2T(app.first.c_str()));
			}
			
		}

	}
}

void LogFilterPage::OnBtnApp()
{
	SelectItemPage dlg;
	dlg.mTitle = "app";
	dlg.mItemTitle = "app";
	auto& items=dlg.mItems;
	{
		for (auto& item : mLogManager->apps())
		{
			auto& app = item.first;
			bool checked = false;
			items[item.first] = checked;
		}

		for (auto& item : mFilterInfo.apps)
		{
			items[item.first] = item.second;
		}
	}

	auto ret=dlg.DoModal();
	if (ret == IDOK)
	{
		auto& apps = mFilterInfo.apps;
		apps.clear();
		string textApps;
		for (auto& item : dlg.mItems)
		{
			if (item.second)
			{
				apps[item.first] = true;
				
				if (!textApps.empty())
				{
					textApps += ",";
				}
				
				textApps += item.first;
			}
		}

		USES_CONVERSION;
		SetText(IDC_EDIT_APP, A2T(textApps.c_str()));
	}
}

void LogFilterPage::OnBnClickedTag()
{
	SelectItemPage dlg;
	dlg.mTitle = "tag";
	dlg.mItemTitle = "tag";
	auto& items = dlg.mItems;
	{
		for (auto& item : mLogManager->tags())
		{
			auto& app = item.first;
			bool checked = false;
			items[item.first] = checked;
		}

		for (auto& item : mFilterInfo.tags)
		{
			items[item.first] = item.second;
		}
	}

	auto ret = dlg.DoModal();
	if (ret == IDOK)
	{
		auto& apps = mFilterInfo.tags;
		apps.clear();
		string textApps;
		for (auto& item : dlg.mItems)
		{
			if (item.second)
			{
				apps[item.first] = true;

				if (!textApps.empty())
				{
					textApps += ",";
				}

				textApps += item.first;
			}
		}

		USES_CONVERSION;
		SetText(IDC_EDIT_TAG, A2T(textApps.c_str()));
	}
}

void LogFilterPage::DisableApp(const string& app)
{
	USES_CONVERSION;
	string apps = T2A(GetText(IDC_EDIT_APP));
	unordered_map<string, bool> mapItems;

	vector<string> items;
	StringTool::ExtractSubString(apps, ',', items);
	for (auto& item : items)
	{
		if (item.empty())
		{
			continue;
		}

		auto pos = item.find('!');
		if (pos == 0)
		{
			mapItems[item.c_str() + 1] = false;
		}
		else
		{
			mapItems[item] = true;
		}
	}

	mapItems[app] = false;
	
	apps.clear();
	for (auto& item : mapItems)
	{
		if (!item.second)
		{
			apps += "!";
		}

		apps += item.first + ",";
	}

	SetText(IDC_EDIT_APP, A2T(apps.c_str()));
	RefreshFilter();
}

void LogFilterPage::ClearApp()
{
	SetText(IDC_EDIT_APP, _T(""));
	RefreshFilter();
}

void LogFilterPage::DisableTag(const string& tag)
{
	USES_CONVERSION;
	string apps = T2A(GetText(IDC_EDIT_TAG));
	unordered_map<string, bool> mapItems;

	vector<string> items;
	StringTool::ExtractSubString(apps, ',', items);
	for (auto& item : items)
	{
		if (item.empty())
		{
			continue;
		}

		auto pos = item.find('!');
		if (pos == 0)
		{
			mapItems[item.c_str() + 1] = false;
		}
		else
		{
			mapItems[item] = true;
		}
	}

	mapItems[tag] = false;

	apps.clear();
	for (auto& item : mapItems)
	{
		if (!item.second)
		{
			apps += "!";
		}

		apps += item.first + ",";
	}

	SetText(IDC_EDIT_TAG, A2T(apps.c_str()));
	RefreshFilter();
}

void LogFilterPage::ClearTag()
{
	SetText(IDC_EDIT_TAG, _T(""));
	RefreshFilter();
}

void LogFilterPage::OnCbnSelchangeCmbLevel()
{
	auto sel = mComboLevel.GetCurSel();
	if (sel != -1 && mFilterInfo.level != gLevel[sel].level)
	{
		mFilterInfo.level = gLevel[sel].level;
		SignalFilterChanged();
	}
}

void LogFilterPage::OnBnClickedChkAutoScroll()
{
	mAutoScroll = IsDlgButtonChecked(IDC_CHK_AUTO_SCROLL);

	if (!mAutoScroll)
	{
		KillTimer(eTimer_countDown);
		mCountDownSeconds = 0;
		UpdateCountDownText();
	}
}

void LogFilterPage::OnRelayout(const CRect& rc)
{
	__super::OnRelayout(rc);

	UINT arr[] = { IDC_EDIT_TAG,IDC_EDIT_APP,};
	for(auto& id:arr)
	{
		auto item = GetDlgItem(id);
		if (!item)
		{
			return;
		}

		CRect rcItem;
		item->GetWindowRect(&rcItem);
		ScreenToClient(rcItem);

		rcItem.right = rc.right;
		item->MoveWindow(rcItem);

	}

}

void LogFilterPage::OnUserAction(ULONGLONG tick)
{
	mUserActionTick = tick;

	if (!mAutoScroll)
	{
		return;
	}

	int seconds = 30;
	if (mCountDownSeconds == 0)
	{
		mCountDownSeconds = seconds;
		SetTimer(eTimer_countDown, 1000);
	}
	else
	{
		mCountDownSeconds = seconds;

	}
	UpdateCountDownText();
}

void LogFilterPage::OnClearList()
{
	mCountDownSeconds = 0;
	KillTimer(eTimer_countDown);
	UpdateCountDownText();

}

bool LogFilterPage::IsAutoScrollEnabled()
{
	if (mAutoScroll)
	{
		return mCountDownSeconds == 0;
	}

	return mAutoScroll;
}

void LogFilterPage::OnTimer(UINT_PTR id)
{
	if (id == eTimer_countDown)
	{
		--mCountDownSeconds;
		if (mCountDownSeconds <= 0)
		{
			KillTimer(id);
		}

		UpdateCountDownText();
		return;
	}

	__super::OnTimer(id);
}

void LogFilterPage::UpdateCountDownText()
{
	if (mCountDownSeconds > 0)
	{
		CString text;
		text.Format(_T("AutoScroll (%d)"),mCountDownSeconds);
		SetDlgItemText(IDC_CHK_AUTO_SCROLL, text);
		CheckDlgButton(IDC_CHK_AUTO_SCROLL, BST_INDETERMINATE | BST_PUSHED);
	}
	else
	{
		SetDlgItemText(IDC_CHK_AUTO_SCROLL, _T("AutoScroll"));
		CheckDlgButton(IDC_CHK_AUTO_SCROLL, mAutoScroll? BST_CHECKED: BST_UNCHECKED);
	}
}
