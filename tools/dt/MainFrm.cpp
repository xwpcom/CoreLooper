﻿#include "pch.h"
#include "framework.h"
#include "dt.app.h"
#include "resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum
{
	eTimer_CheckSaveIni,
};

IMPLEMENT_DYNAMIC(MainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(MainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &MainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &MainFrame::OnUpdateApplicationLook)
	ON_COMMAND(ID_KEEP_TOP,OnKeepTop)
	ON_UPDATE_COMMAND_UI(ID_KEEP_TOP,OnUpdateKeepTop)
	ON_COMMAND(ID_ENABLE_IE_PROXY, OnEnableIEProxy)
	ON_UPDATE_COMMAND_UI(ID_ENABLE_IE_PROXY, OnUpdateIEProxy)
	
	ON_COMMAND(ID_REFRESH_TASK_ICON,OnRefreshTaskIcon)
	ON_COMMAND(ID_BACKUP_CFG, OnBackupCfg)
	ON_WM_TIMER()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

// MainFrame construction/destruction

MainFrame::MainFrame() noexcept
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);
}

MainFrame::~MainFrame()
{
}

int MainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	{
		HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		SetIcon(hIcon, TRUE);
		SetIcon(hIcon, FALSE);
	}

	SetTimer(eTimer_CheckSaveIni, 2000,nullptr);

	BOOL bNameValid;

	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, nullptr))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CString strTitlePane1;
	CString strTitlePane2;
	bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), strTitlePane1);
	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, strTitlePane2, TRUE), strTitlePane2);

	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	{
		auto app = (App*)AfxGetApp();
		auto& ini = app->mIni;
		mKeepTop=ini.GetBool(mSection.c_str(), "keepTop", true);
		mEnableIEProxy = ini.GetBool(mSection.c_str(), "enableIEProxy", false);

		ApplyKeepTop();
		ApplyIEProxy();
	}

	return 0;
}

BOOL MainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// MainFrame diagnostics

#ifdef _DEBUG
void MainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void MainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// MainFrame message handlers

void MainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL MainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void MainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(TRUE);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
	}

	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void MainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}



void MainFrame::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case eTimer_CheckSaveIni:
	{
		auto app = (App*)AfxGetApp();
		auto& ini = app->mIni;
		if (ini.IsModified())
		{
			ini.Save();
		}

		return;
	}
	}

	CFrameWndEx::OnTimer(nIDEvent);
}

#include <WinInet.h>

void MainFrame::ApplyIEProxy()
{
	string mTag = "IEProxy";

	/* 测试没成功，disable可以生效，但enable不能生效
	* 
	if (mEnableIEProxy)
	{
		DWORD flags = 0;
		LPWSTR xx;
		WCHAR connectionName[256];

		InternetGetConnectedStateEx(&flags,connectionName,sizeof(connectionName),0);

		auto ok = IEHttpProxy::SetConnectionOptions(connectionName, _T("127.0.0.1:1080")); LogV(mTag, "enable proxy ok = %d", ok);
	}
	else
	{
		auto ok = IEHttpProxy::DisableConnectionProxy(NULL);LogV(mTag, "disable proxy,return = %d", ok);
	}
	//*/
}

void MainFrame::OnEnableIEProxy()
{
	mEnableIEProxy = !mEnableIEProxy;

	auto app = (App*)AfxGetApp();
	auto& ini = app->mIni;
	ini.SetBool(mSection.c_str(), "enableIEProxy", mEnableIEProxy);

	ApplyIEProxy();
	
}
void MainFrame::OnKeepTop()
{
	mKeepTop = !mKeepTop;

	auto app = (App*)AfxGetApp();
	auto& ini = app->mIni;
	ini.SetBool(mSection.c_str(), "keepTop", mKeepTop);

	ApplyKeepTop();
}

void MainFrame::ApplyKeepTop()
{
	if (mKeepTop)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else
	{
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

void MainFrame::OnUpdateKeepTop(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(mKeepTop);
}

void MainFrame::OnUpdateIEProxy(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(mEnableIEProxy);
}

void MainFrame::OnKillFocus(CWnd* pNewWnd)
{
	CFrameWndEx::OnKillFocus(pNewWnd);

	if (mKeepTop)
	{
		//https://docs.microsoft.com/en-us/windows/win32/winmsg/extended-window-styles
		//有时SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);没有起作用
		//在此重设一下

		auto v = GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE);
		if (!(v & WS_EX_TOPMOST))
		{
			ApplyKeepTop();
		}
	}

}

//XiongWanPing 2019.11.07
//https://blog.csdn.net/mfcing/article/details/50345193
//有些固定在任务栏的app是安装在bitlocker加密盘里面，当开机时windows无法获取app图标
//导致任务栏上app图标为空白,不方便使用
//所以在DT上增加刷新图标的功能
void MainFrame::OnRefreshTaskIcon()
{
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);//刷新任务栏icon,test ok

	{
		//starcraft title
		auto wnd = ::FindWindow(_T("OsWindow"), nullptr);
		if (wnd)
		{
			::SetWindowText(wnd, _T("c++ test"));
			int x = 0;
		}
	}
}

void MainFrame::OnBackupCfg()
{
	auto folder = ShellTool::GetAppPath();
	auto t = ShellTool::GetCurrentTimeMs();
	auto time = StringTool::Format("%06d.%06d", t.date(), t.time());
	auto filePath = StringTool::Format("%s/backup/%s.ini", folder.c_str(),time.c_str());
	theApp.mIni.Dump(filePath);
}
