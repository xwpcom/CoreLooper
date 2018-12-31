// DebugHelper.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DebugHelper.h"

#include "MainFrm.h"
#include "DebugHelperDoc.h"
#include "DebugHelperView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperApp

BEGIN_MESSAGE_MAP(CDebugHelperApp, CWinApp)
	//{{AFX_MSG_MAP(CDebugHelperApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperApp construction

CDebugHelperApp::CDebugHelperApp()
{
	m_hMutex = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDebugHelperApp object

CDebugHelperApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperApp initialization
#pragma comment(linker,"/section:SharedData,RWS")

#pragma data_seg("SharedData")
	HWND	gs_hWnd = 0; 
#pragma data_seg() 

BOOL CDebugHelperApp::InitInstance()
{
	m_hMutex = ::CreateMutex(NULL,TRUE,"D3F11E49-4E38-4639-9EBB-1BD15DC0A21E");
	if(::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if(gs_hWnd && ::IsWindow(gs_hWnd))
		{
			//让DebugHelper能随时弹出来
			DWORD dwValue = 0;
			SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT,0,&dwValue,0);
			SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,0,0);
			//pop up
			if(!::IsWindowVisible(gs_hWnd) || IsIconic(gs_hWnd))
				::ShowWindow(gs_hWnd,SW_RESTORE);
			BOOL bOK = SetForegroundWindow(gs_hWnd);
			bOK &= BringWindowToTop(gs_hWnd);

			SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT,0,(void*)dwValue,0);
		}
		return FALSE;
	}
	
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
	this->EnableShellOpen();
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDebugHelperDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDebugHelperView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it.
	//m_pMainWnd->ShowWindow(SW_SHOW);
	//m_pMainWnd->UpdateWindow();
	
	unsigned char * pData = NULL;
	UINT nRead = 0;
	WINDOWPLACEMENT wndStatus = {0};
	AfxGetApp()->GetProfileBinary("Settings","WindowPos",
		&pData,&nRead);
	if(nRead == sizeof(wndStatus))
	{
		memcpy((char*)&wndStatus,(char*)pData,nRead);
		VERIFY(m_pMainWnd->SetWindowPlacement(&wndStatus));
	}
	else
		m_pMainWnd->ShowWindow(3);

	delete pData;

	gs_hWnd = m_pMainWnd->m_hWnd;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_szComment;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_szComment = _T("");
	//}}AFX_DATA_INIT
	m_szComment.LoadString(IDS_COMMENT);
	m_szComment.Replace("\n","\r\n");

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_EDIT_COMMENT, m_szComment);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CDebugHelperApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperApp message handlers


int CDebugHelperApp::ExitInstance() 
{
	::CloseHandle(m_hMutex);	
	return CWinApp::ExitInstance();
}


