#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "dt.app.h"
#include "MainFrm.h"
#include "StudyListCtrlPage.h"
#include "src/LogPageEx.h"
#include "src/logmanager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// App

BEGIN_MESSAGE_MAP(App, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &App::OnAppAbout)
END_MESSAGE_MAP()


// App construction

App::App() noexcept
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("dt.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only App object

App theApp;


// App initialization

BOOL App::InitInstance()
{
	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	SetRegistryKey(_T("Bear"));

	mIni.Load(ShellTool::GetAppPath() + "/dt.ini");
	BasePage::SetIni(&mIni);

	if(0)
	{
		auto obj = make_shared<LogManager>();
		LogPageEx dlg;
		dlg.SetLogManager(obj);

		//StudyListCtrlPage dlg;
		dlg.DoModal();
		return FALSE;
	}


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CFrameWnd* pFrame = new MainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, nullptr,
		nullptr);





	// The one and only window has been initialized, so show and update it
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	return TRUE;
}

int App::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);
	if (mIni.IsModified())
	{
		mIni.Save();
	}
	return CWinAppEx::ExitInstance();
}

// App message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// App customization load/save methods

void App::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void App::LoadCustomState()
{
}

void App::SaveCustomState()
{
}

// App message handlers





BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString text = _T("DT is a open source log tool,like Android Studio Logcat\r\n\r\n"
		"Author:xwpcom@163.com\r\n\r\nQQ:117620974\r\n\r\n"
		"source code https://github.com/xwpcom/CoreLooper\r\n"
	);
	SetDlgItemText(IDC_EDIT_COMMENT, text);

	CString version;
	USES_CONVERSION;
	version.Format(_T("DT           Build on %s %s"),A2T(__DATE__),A2T(__TIME__));
	SetDlgItemText(IDC_STATIC_VERSION, version);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
