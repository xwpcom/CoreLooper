#include "pch.h"
#include "dt.app.h"
#include "ConsolePage.h"
#include "afxdialogex.h"
#include "LogPage.h"
#include "loginfo.h"
#include "logmanager.h"
#include "EditTextPage.h"

enum
{
	eTimer_DelayActiveTab,
};

IMPLEMENT_DYNAMIC(ConsolePage, BasePage)

ConsolePage::ConsolePage(CWnd* pParent /*=nullptr*/)
	: BasePage(IDD_Console, pParent)
{
	mSection = "consolePage";
}

ConsolePage::~ConsolePage()
{
}

void ConsolePage::DoDataExchange(CDataExchange* pDX)
{
	BasePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, mTabPos);
}


BEGIN_MESSAGE_MAP(ConsolePage, BasePage)
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_TAB_DELETE, &ConsolePage::OnTabDelete)
	ON_COMMAND(ID_TAB_EDIT, &ConsolePage::OnTabEdit)
	ON_COMMAND(ID_TAB_NEW, &ConsolePage::OnTabNew)
END_MESSAGE_MAP()

void ConsolePage::UpdateControlPos()
{
	if (!mTab.GetSafeHwnd())
	{
		return;
	}

	CRect rcClient;
	GetClientRect(rcClient);

	mTab.MoveWindow(rcClient);
}

class CLog2 //for DT2020
{
public:
	CLog2(const char* lpszFile, int nLine, int nLevel)
		:m_lpszFile(lpszFile), m_nLine(nLine), m_nLevel(nLevel)
	{
	}

	int operator()(const char* lpszFormat, ...)
	{
		return 0;
	}
	int operator()(const char* tag, const char* lpszFormat, ...)
	{
		return 0;
	}
protected:
	const char* m_lpszFile;
	int m_nLine;
	int m_nLevel;
};

#define LogV2	(CLog2( __FILE__, __LINE__,DT_VERBOSE))
#define LogD2	(CLog2( __FILE__, __LINE__,DT_DEBUG))
#define LogI2	(CLog2( __FILE__, __LINE__,DT_INFO))
#define LogW2	(CLog2( __FILE__, __LINE__,DT_WARN))
#define LogE2	(CLog2( __FILE__, __LINE__,DT_ERROR))

int ConsolePage::Init()
{
	{
		mMaxLogCount = mIni->GetInt(mSection, "maxLogCount", 1000 * 1000);
		if (mMaxLogCount <= 1000)
		{
			mMaxLogCount = 1000;
		}
	}

	/*
	{
		LogV2("hello", "hello");
		LogV2("hello,%d", 12);
		LogV2("TAG","hello,%d",12);
	}
	*/

	{
		if (!mSinkWnd.GetSafeHwnd())
		{
			mSinkWnd.Create(_T("DT2020 "));
			mSinkWnd.SignalCopyDataReady.connect(this, &ConsolePage::OnCopyDataReady);
		}
	}

	{
		mLogManager = make_shared<LogManager>();
		mLogManager->OnCreate();

		SignalLogItemReady.connect(mLogManager.get(), &LogManager::OnLogItemReady);
	}

	CRect rectTab;

	mTabPos.GetWindowRect(&rectTab);
	ScreenToClient(&rectTab);
	mTabPos.DestroyWindow();

	mTab.Create(CMFCTabCtrl::STYLE_3D_VS2005, rectTab, this, 1, CMFCTabCtrl::LOCATION_TOP);
	mTab.AutoDestroyWindow(FALSE);
	mTab.ModifyStyle(NULL, WS_CLIPCHILDREN);


	{
		string tabIds = mIni->GetString(mSection, "tabIds");
		vector<string> ids;
		StringTool::ExtractSubString(tabIds, ',',ids);
		map<int, bool> existIds;
		string section;
		for (auto& item : ids)
		{
			auto id = atoi(item.c_str());
			if (existIds.find(id) != existIds.end())
			{
				//过滤重复的id
				continue;
			}

			existIds[id] = true;

			section = StringTool::Format("log.%d", id);
			auto title=mIni->GetString(section, "title");
			auto obj=CreateTab(title, id);
		}
	}

	int activeTab = mIni->GetInt(mSection, "activeTab", 0);
	mTab.SetActiveTab(!activeTab);//确保执行切换tab的动作
	SetTimer(eTimer_DelayActiveTab, 1, NULL);

	__super::Init();

	if (mPages.size() == 0)
	{
		OnTabNew();
	}

	return 0;
}

void ConsolePage::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case eTimer_DelayActiveTab:
	{
		KillTimer(nIDEvent);

		auto activeTab = mIni->GetInt(mSection, "activeTab");
		if (activeTab < 0 && activeTab >= mTab.GetTabsNum())
		{
			activeTab = 0;
		}
		mTab.SetActiveTab(activeTab);
		break;
	}
	}
}

void ConsolePage::PreClose()
{
	for (auto& item : mPages)
	{
		item->PreClose();
	}
}

void ConsolePage::OnSetFocus(CWnd* pOldWnd)
{
	BasePage::OnSetFocus(pOldWnd);

	for (auto& item : mPages)
	{
		if (item->IsWindowVisible())
		{
			item->SetFocus();
			break;
		}
	}
}

#define CLASS_MSG_SINK		_T("CMsgSink")

MsgSinkWnd::MsgSinkWnd()
{
}

MsgSinkWnd::~MsgSinkWnd()
{
	::DestroyWindow(m_hWnd);
}


BEGIN_MESSAGE_MAP(MsgSinkWnd, CWnd)
	//{{AFX_MSG_MAP(MsgSinkWnd)
	ON_WM_COPYDATA()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// MsgSinkWnd message handlers

BOOL MsgSinkWnd::Create(CString szName)
{
	return CreateEx(NULL, CLASS_MSG_SINK, szName, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0);
}

BOOL MsgSinkWnd::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	COPYDATASTRUCT* pCDS = pCopyDataStruct;
	auto data = (LPBYTE)pCDS->lpData;
	auto bytes = pCDS->cbData;
	SignalCopyDataReady(data, bytes);
	return CWnd::OnCopyData(pWnd, pCopyDataStruct);
}

BOOL MsgSinkWnd::Register()
{
	static BOOL bRegistered = FALSE;
	if (!bRegistered)
	{
		WNDCLASS wc = { 0 };
		wc.style = NULL;
		wc.lpfnWndProc = ::DefWindowProc;
		wc.hInstance = AfxGetInstanceHandle();
		wc.lpszClassName = CLASS_MSG_SINK;
		wc.hbrBackground = NULL;
		wc.hCursor = NULL;
		bRegistered = AfxRegisterClass(&wc);
	}
	return bRegistered;
}

BOOL MsgSinkWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!Register())
		return FALSE;

	return CWnd::PreCreateWindow(cs);
}

void ConsolePage::OnCopyDataReady(LPBYTE data, int bytes)
{
	auto item = LogParser::Input(data, bytes);
	if (item)
	{
		SignalLogItemReady(item);
	}
}

void ConsolePage::OnContextMenu(CWnd* pWnd, CPoint point)
{
	__super::OnContextMenu(pWnd, point);

	DV("%s,wnd=%p", __func__, pWnd);

	if (pWnd == &mTab)
	{
		{
			CPoint pt(point);
			mTab.ScreenToClient(&pt);
			int sel = mTab.GetTabFromPoint(pt);
			//DV("tab sel=%d", sel);
			/*
			if (sel == -1 && mTab.GetTabsNum()>0)
			{
				return;
			}
			*/

			if (sel != mTab.GetActiveTab())
			{
				mTab.SetActiveTab(sel);
			}
		}


		CMenu menu;
		menu.LoadMenu(IDR_TAB_CONTEXT_MENU);
		int id = IDR_TAB_CONTEXT_MENU;
#ifdef _CONFIG_SHIP
		id = IDR_TAB_CONTEXT_MENU_SHIP;
#endif
		//theApp.mLang.TransMenu(menu, id);
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

shared_ptr<LogPage> ConsolePage::CreateTab(string title, int id)
{
	CWnd* parent = &mTab;

	CRect rc;
	GetClientRect(&rc);
	auto obj = make_shared<LogPage>();
	obj->SetMaxLogCount(mMaxLogCount);
	obj->SetLogManager(mLogManager);
	SignalLogItemReady.connect(obj.get(), &LogPage::OnLogItemReady);
	obj->SetPageId(id);
	obj->SetIniSection(StringTool::Format("log.%d", id));

	obj->Create(IDD_LogPage, parent);

	int newIndex = mTab.GetTabsNum();
	USES_CONVERSION;
	mTab.InsertTab(obj.get(), A2T(title.c_str()), newIndex);// , 4);// rand() % 5);
	//auto icon = AfxGetApp()->LoadIcon((newIndex%2)?IDI_SHIP_ONLINE:IDI_SHIP_OFFLINE);
	//mTab.SetTabHicon(newIndex,icon);
	int tabId = mTab.GetTabID(newIndex);
	//ASSERT(mGroups.find(tabId) == mGroups.end());
	//mGroups[tabId] = group;
	mPages.push_back(obj);

	{
		//mTab.SetTabTextColor(newIndex, RGB(0, 0,255));
		//mTab.EnableTabDetach(newIndex, TRUE);
		//mTab.SetActiveTabTextColor(RGB(255, 0, 0));
		//mTab.SetActiveTabColor(RGB(255, 0, 255));
	}

	//group->mVideoChannelFrame.ShowChannelPane(mShowChannelPane);

	mTab.SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);//要置底，否则会挡住groups
	mTab.RecalcLayout();
	mTab.RedrawWindow();

	return obj;
}

void ConsolePage::OnTabNew()
{
	int id = GetFreePageId();
	auto obj=CreateTab("log",id);
}

void ConsolePage::LoadConfig()
{
	__super::LoadConfig();
}

void ConsolePage::SaveConfig()
{
	__super::SaveConfig();

	/*
	virtual BOOL GetTabLabel(int iTab, CString& strLabel) const;
	virtual BOOL SetTabLabel(int iTab, const CString& strLabel);
	*/

	CString label;
	auto nc = mTab.GetTabsNum();
	string tabIds;
	USES_CONVERSION;
	for (int i = 0; i < nc; i++)
	{
		mTab.GetTabLabel(i, label);

		auto page = (LogPage*)mTab.GetTabWnd(i);
		tabIds += StringTool::Format("%d,", page->pageId());
		
		mIni->SetString(page->GetIniSection(), "title", T2A(label));
	}

	mIni->SetString(mSection, "tabIds", tabIds);

	auto activeTab = mTab.GetActiveTab();
	mIni->SetInt(mSection, "activeTab", activeTab);
}

int ConsolePage::GetFreePageId()
{
	unordered_map<int, bool> ids;
	const auto nc = mTab.GetTabsNum();
	for (int i = 0; i < nc; i++)
	{
		auto page = (LogPage*)mTab.GetTabWnd(i);
		ids[page->pageId()] = true;
	}

	for (int i = 0; i < nc + 2; i++)
	{
		if (ids.find(i) == ids.end())
		{
			return i;
		}
	}

	return 0;
}

void ConsolePage::OnTabEdit()
{
	auto sel = mTab.GetActiveTab();
	if (sel == -1)
	{
		return;
	}

	CString text;
	mTab.GetTabLabel(sel, text);

	USES_CONVERSION;
	EditTextPage dlg;
	dlg.mTitle = A2T("Edit title");
	dlg.mText = text;
	auto ret=dlg.DoModal();
	if (ret == IDOK)
	{
		mTab.SetTabLabel(sel, dlg.mText);
	}
}


void ConsolePage::OnTabDelete()
{
	auto sel = mTab.GetActiveTab();
	if (sel == -1)
	{
		return;
	}

	auto page = (LogPage*)mTab.GetTabWnd(sel);

	shared_ptr<BasePage> obj;
	for(auto iter= mPages.begin();iter!= mPages.end();++iter)
	{
		auto item = *iter;

		if (item.get() == page)
		{
			obj = item;
			mPages.erase(iter);
			mTab.RemoveTab(sel);
			break;
		}
	}

	page->DestroyWindow();
	page->RemoveSection();
}
