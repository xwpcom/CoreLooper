#include "stdafx.h"
#include "DebugHelper.h"
#include "ViewTraceDlg.h"
#include <afxcview.h>
#include "DebugHelperView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewTraceDlg dialog

CViewTraceDlg::CViewTraceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CViewTraceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewTraceDlg)
	m_szTime = _T("");
	m_szData = _T("");
	m_szLine = _T("");
	//}}AFX_DATA_INIT
}


void CViewTraceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewTraceDlg)
	DDX_Text(pDX, IDC_TIME, m_szTime);
	DDX_Text(pDX, IDC_EDIT_DATA, m_szData);
	DDX_Text(pDX, IDC_EDIT_LINE, m_szLine);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewTraceDlg, CDialog)
	//{{AFX_MSG_MAP(CViewTraceDlg)
	ON_BN_CLICKED(IDC_BTN_UP, OnBtnUp)
	ON_BN_CLICKED(IDC_BTN_DOWN, OnBtnDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewTraceDlg message handlers

void CViewTraceDlg::OnBtnUp() 
{
	CListView * pView = static_cast<CListView*>(m_pParentWnd);
	ASSERT(pView);
	CListCtrl &lstCtrl = pView->GetListCtrl();
	if(m_nLine > 0)
	{
		m_nLine--;
		SetIndex(m_nLine);
	}
}

void CViewTraceDlg::OnBtnDown() 
{
	CListView * pView = static_cast<CListView*>(m_pParentWnd);
	ASSERT(pView);
	CListCtrl &lstCtrl = pView->GetListCtrl();
	if(m_nLine < lstCtrl.GetItemCount() - 1)
	{
		m_nLine++;
		SetIndex(m_nLine);
	}
}

void CViewTraceDlg::SetIndex(int nIndex)
{
	CDebugHelperView * pView = static_cast<CDebugHelperView*>(m_pParentWnd);
	ASSERT(pView);
	m_nLine = nIndex;
	CListCtrl &lstCtrl = pView->GetListCtrl();
	m_szLine = lstCtrl.GetItemText(m_nLine, pView->mArrIdx[eIdxFileLine]);
	m_szTime = lstCtrl.GetItemText(m_nLine, pView->mArrIdx[eIdxTime]);
	//m_szData = lstCtrl.GetItemText(m_nLine,0);
	tagMsgEx *pMsgEx = (tagMsgEx *)lstCtrl.GetItemData(nIndex);
	if(pMsgEx)
	{
		m_szData=pMsgEx->msg;
	}
	else
	{
		m_szData="";
	}

	UpdateData(FALSE);
}

void CViewTraceDlg::OnOK() 
{
	ShowWindow(SW_HIDE);
	return;
}
void CViewTraceDlg::OnCancel() 
{
	ShowWindow(SW_HIDE);
	return;
}
