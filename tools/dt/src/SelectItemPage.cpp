#include "pch.h"
#include "SelectItemPage.h"
#include "afxdialogex.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(SelectItemPage, BasePage)

SelectItemPage::SelectItemPage(CWnd* pParent /*=nullptr*/)
	: BasePage(IDD_SelectItemPage, pParent)
{

}

SelectItemPage::~SelectItemPage()
{														    
}

void SelectItemPage::DoDataExchange(CDataExchange* pDX)
{
	BasePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, mListCtrl);
}


BEGIN_MESSAGE_MAP(SelectItemPage, BasePage)
	ON_COMMAND(IDOK,OnBnClickedOk)
	ON_BN_CLICKED(IDC_SELECT_ALL, &SelectItemPage::OnBnClickedSelectAll)
	ON_BN_CLICKED(IDC_SELECT_INVERT, &SelectItemPage::OnBnClickedSelectInvert)
END_MESSAGE_MAP()


int SelectItemPage::Init()
{
	mEnableOK = true;
	mEnableCancel = true;

	USES_CONVERSION;

	SetWindowText(A2T(mTitle.c_str()));

	CListCtrl& list = mListCtrl;
	{
		//mFont.DeleteObject();
		//mFont.CreatePointFont(160, _T("ÐÂËÎÌå"));

		list.DeleteAllItems();

		//list.SetFont(&mFont);
		list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT
			| LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP | LVS_EX_CHECKBOXES);// | LVS_EX_LABELTIP );
	
		int idx = -1;
		list.InsertColumn(++idx, A2T(mItemTitle.c_str()),0,300);
	}

	{
		int idx = -1;
		for (auto& item : mItems)
		{
			++idx;
			auto i=list.InsertItem(idx, A2T(item.first.c_str()));
			list.SetCheck(i, item.second);
		}
	}

	return __super::Init();
}

void SelectItemPage::OnBnClickedOk()
{
	CListCtrl& list = mListCtrl;
	auto nc = list.GetItemCount();
	USES_CONVERSION;
	for (int i = 0; i < nc; i++)
	{
		string text=T2A(list.GetItemText(i, 0));
		mItems[text] = list.GetCheck(i);
	}

	EndDialog(IDOK);
}


void SelectItemPage::OnBnClickedSelectAll()
{
	CListCtrl& list = mListCtrl;
	auto nc = list.GetItemCount();
	for (int i = 0; i < nc; i++)
	{
		list.SetCheck(i, true);
	}
}

void SelectItemPage::OnBnClickedSelectInvert()
{
	CListCtrl& list = mListCtrl;
	auto nc = list.GetItemCount();
	for (int i = 0; i < nc; i++)
	{
		list.SetCheck(i, !list.GetCheck(i));
	}
}
