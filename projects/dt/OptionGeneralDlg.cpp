// OptionGeneralDlg.cpp : implementation file
//

#include "stdafx.h"
#include "debughelper.h"
#include "OptionGeneralDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionGeneralDlg property page

IMPLEMENT_DYNCREATE(COptionGeneralDlg, CPropertyPage)

COptionGeneralDlg::COptionGeneralDlg() : CPropertyPage(COptionGeneralDlg::IDD)
{
	//{{AFX_DATA_INIT(COptionGeneralDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COptionGeneralDlg::~COptionGeneralDlg()
{
}

void COptionGeneralDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionGeneralDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionGeneralDlg, CPropertyPage)
	//{{AFX_MSG_MAP(COptionGeneralDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionGeneralDlg message handlers
