#include "pch.h"
#include "EditTextPage.h"
#include "afxdialogex.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(EditTextPage, BasePage)

EditTextPage::EditTextPage(CWnd* pParent /*=nullptr*/)
	: BasePage(IDD_EditTextPage, pParent)
{
	mEnableOK = true;
	mEnableCancel = true;
}

EditTextPage::~EditTextPage()
{
}

void EditTextPage::DoDataExchange(CDataExchange* pDX)
{
	BasePage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(EditTextPage, BasePage)
	ON_BN_CLICKED(IDOK, &EditTextPage::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL EditTextPage::OnInitDialog()
{
	BasePage::OnInitDialog();

	SetWindowText(mTitle);
	SetText(IDC_EDIT_TEXT, mText);
	GetDlgItem(IDC_EDIT_TEXT)->SetFocus();


	return FALSE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void EditTextPage::OnBnClickedOk()
{
	GetDlgItemText(IDC_EDIT_TEXT, mText);
	EndDialog(IDOK);
}
