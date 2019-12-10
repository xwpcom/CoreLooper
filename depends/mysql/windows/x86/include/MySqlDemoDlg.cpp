// MySqlDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MySqlDemo.h"
#include "MySqlDemoDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySqlDemoDlg dialog

CMySqlDemoDlg::CMySqlDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMySqlDemoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMySqlDemoDlg)
	m_nIndex=-1;
	str_PreName=_T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMySqlDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMySqlDemoDlg)
	DDX_Control(pDX, IDC_LIST1, m_list);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMySqlDemoDlg, CDialog)
	//{{AFX_MSG_MAP(CMySqlDemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton_Update)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton_Delete)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton_Add)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnClickList1)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySqlDemoDlg message handlers

BOOL CMySqlDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMySqlDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMySqlDemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMySqlDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMySqlDemoDlg::RefreshList()
{
	m_list.DeleteAllItems();

	//////////////////////////////////////////////////////////////////////////
	char *ch_query;
	ch_query="select * from login";

	if(mysql_real_query(&mysql,ch_query,(UINT)strlen(ch_query))!=0)
	{ 
		AfxMessageBox("数据库中表格出错"); 
	}

	CString str;
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	if(!(result=mysql_use_result(&mysql)))
	{ 
		AfxMessageBox("读取数据集失败"); 		
	}

	int i=0;
	while(row=mysql_fetch_row(result)){
		str.Format("%s",row[0]);
		m_list.InsertItem(i,str);
		
		str.Format("%s",row[1]);
		m_list.SetItemText(i,1,str);
		
		str.Format("%s",row[2]);
		m_list.SetItemText(i,2,str);

		str.Format("%s",row[6]);
		m_list.SetItemText(i,3,str);

		i++;
	}

	mysql_free_result(result);
	
}

void CMySqlDemoDlg::OnButton_Update() 
{
	UpdateData();
	CString strUsername,strList,strRemark;
	
	GetDlgItem(IDC_EDIT_USERNAME)->GetWindowText(strUsername);
	GetDlgItem(IDC_EDIT_VISITELIST)->GetWindowText(strList);
	GetDlgItem(IDC_EDIT_REMARK)->GetWindowText(strRemark);

	CString strSQL;
	strSQL.Format("update mytable set username=\'%s\',visitelist=\'%s\',remark=\'%s\' where username=\'%s\'",strUsername,strList,strRemark,str_PreName);

	if(mysql_real_query(&mysql,(char*)(LPCTSTR)strSQL,(UINT)strSQL.GetLength())!=0)
	{ 
		AfxMessageBox("修改失败"); 
	}

	RefreshList();
	
}

void CMySqlDemoDlg::OnButton_Delete() 
{
	UpdateData(); 
	CString strSQL;
	strSQL.Format("delete from mytable where username=\'%s\'",str_PreName);
	if(mysql_real_query(&mysql,(char*)(LPCTSTR)strSQL,(UINT)strSQL.GetLength())!=0)
	{ 
		AfxMessageBox("删除失败"); 
	}
	
	for(POSITION pos=m_list.GetFirstSelectedItemPosition();pos!=NULL;)
	{
		int nIndex=m_list.GetNextSelectedItem(pos);
	}
	
	RefreshList();
	
}

void CMySqlDemoDlg::OnButton_Add() 
{
	UpdateData();  
	CString strUsername,strList,strRemark;

	GetDlgItem(IDC_EDIT_USERNAME)->GetWindowText(strUsername);
	GetDlgItem(IDC_EDIT_VISITELIST)->GetWindowText(strList);
	GetDlgItem(IDC_EDIT_REMARK)->GetWindowText(strRemark);

	CString strSQL;
	//strSQL.Format("insert into mytable(username,visitelist,remark) values(\'%s\',\'%s\',\'%s\')",strUsername,strList,strRemark);
	strSQL="insert into login(userid,user_pass,email) values('"+strUsername+"','"+strList+"','"+strRemark+"')";
	if(mysql_real_query(&mysql,(char*)(LPCTSTR)strSQL,(UINT)strSQL.GetLength())!=0)
	{ 
		AfxMessageBox("增添失败");
	}

	RefreshList();
}

void CMySqlDemoDlg::OnClickList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	POSITION pos=m_list.GetFirstSelectedItemPosition();
	if(pos!=NULL)
	{
		CString strUsername,strList,strRemark;

		m_nIndex=m_list.GetNextSelectedItem(pos);
		str_PreName=m_list.GetItemText(m_nIndex,0);
		
		strUsername=m_list.GetItemText(m_nIndex,1);
		GetDlgItem(IDC_EDIT_USERNAME)->SetWindowText(strUsername);
	
		strList=m_list.GetItemText(m_nIndex,2);
		GetDlgItem(IDC_EDIT_VISITELIST)->SetWindowText(strList);

		strRemark=m_list.GetItemText(m_nIndex,3);	
		GetDlgItem(IDC_EDIT_REMARK)->SetWindowText(strRemark);

		UpdateData(FALSE);
		
	}
	*pResult = 0;
}

void CMySqlDemoDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	mysql_close(&mysql);

}

void CMySqlDemoDlg::OnButton4() 
{
	m_list.InsertColumn(0,"account_id",LVCFMT_CENTER,75);
	m_list.InsertColumn(1,"userid",LVCFMT_CENTER,100);
	m_list.InsertColumn(2,"user_pass",LVCFMT_CENTER,100);
	m_list.InsertColumn(3,"email",LVCFMT_CENTER,150);
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	mysql_init (&mysql);
	if(!mysql_real_connect(&mysql,"localhost","ragnarok","password","kjathena",3306,NULL,0))
	{ 
		AfxMessageBox("数据库连接失败"); 
	}

	RefreshList();
}


void CMySqlDemoDlg::OnButton5() 
{
	// TODO: Add your control notification handler code here
	int nIndex,t;
	CString k;

	CListBox* listbox2=(CListBox*)GetDlgItem(IDC_LIST2);

	for(POSITION pos=m_list.GetFirstSelectedItemPosition();pos!=NULL;)
	{
		nIndex=m_list.GetNextSelectedItem(pos);
		k=m_list.GetItemText(nIndex,0);
		listbox2->AddString(k);
	}

}
