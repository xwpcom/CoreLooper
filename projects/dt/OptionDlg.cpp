// OptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "debughelper.h"
#include "OptionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionDlg dialog


COptionDlg::COptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionDlg, CDialog)
	//{{AFX_MSG_MAP(COptionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionDlg message handlers

BOOL COptionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	

	int nMaxLine = AfxGetApp()->GetProfileInt("Settings","nMaxLine",0);
	SetDlgItemInt(IDC_EDIT_MAX_LINE,nMaxLine);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionDlg::OnOK() 
{
	BOOL bOK = FALSE;
	int nMaxLine = GetDlgItemInt(IDC_EDIT_MAX_LINE,&bOK,FALSE);
	if(!bOK || nMaxLine<0)
	{
		AfxMessageBox("无效行数");
		GetDlgItem(IDC_EDIT_MAX_LINE)->SetFocus();
		return;
	}

	AfxGetApp()->WriteProfileInt("Settings","nMaxLine",nMaxLine);
	
	CDialog::OnOK();
}
