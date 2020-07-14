#include "pch.h"
#include "StudyListCtrlPage.h"
#include "afxdialogex.h"
#include "resource.h"

enum
{
	eIdxMsg,
	eIdxTime,
	eIdxFileLine,
	eIdxPid,
	eIdxTag,
	eIdxApp,
};
static const char* TAG = "StudyList";

IMPLEMENT_DYNAMIC(StudyListCtrlPage, CDialogEx)

StudyListCtrlPage::StudyListCtrlPage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_StudyListCtrlPage, pParent)
{

}

StudyListCtrlPage::~StudyListCtrlPage()
{
}

void StudyListCtrlPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, mList);
}


BEGIN_MESSAGE_MAP(StudyListCtrlPage, CDialogEx)
	ON_BN_CLICKED(IDC_ADD_ITEM, &StudyListCtrlPage::OnBnClickedAddItem)
	ON_BN_CLICKED(IDC_SEL_0, &StudyListCtrlPage::OnBnClickedSel0)
	ON_BN_CLICKED(IDC_SEL_1, &StudyListCtrlPage::OnBnClickedSel1)
	ON_BN_CLICKED(IDC_SEL_2, &StudyListCtrlPage::OnBnClickedSel2)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &StudyListCtrlPage::OnLvnItemchangedList)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST, &StudyListCtrlPage::OnLvnItemActivateList)
END_MESSAGE_MAP()


BOOL StudyListCtrlPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	{
		mFont.DeleteObject();
		mFont.CreatePointFont(160, _T("ÐÂËÎÌå"));

		CListCtrl& list = mList;
		list.DeleteAllItems();

		list.SetFont(&mFont);
		list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT
			| LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP);// | LVS_EX_LABELTIP );
		//list.ModifyStyle(LVS_SINGLESEL,NULL);
		int idx = -1;
		mArrIdx[eIdxTag] = list.InsertColumn(++idx, _T("Tag"));
		mArrIdx[eIdxMsg] = list.InsertColumn(++idx, _T("Data"));
		mArrIdx[eIdxTime] = list.InsertColumn(++idx, _T("Time"));
		mArrIdx[eIdxPid] = list.InsertColumn(++idx, _T("Pid/Tid"));
		mArrIdx[eIdxFileLine] = list.InsertColumn(++idx, _T("FileLine"));
		mArrIdx[eIdxApp] = list.InsertColumn(++idx, _T("app"));
		for (int i = 0; i <= idx; i++)
		{
			list.SetColumnWidth(i, 200);
		}

	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void StudyListCtrlPage::OnBnClickedAddItem()
{
	auto& list = mList;

	auto nc = list.GetItemCount();
	auto idx=list.InsertItem(nc, _T(""));

	CString text;
	text.Format(_T("%04d"), nc);
	list.SetItemText(idx, 0, text);
}


void StudyListCtrlPage::OnBnClickedSel0()
{
	Select(0);
}


void StudyListCtrlPage::OnBnClickedSel1()
{
	Select(1);
}


void StudyListCtrlPage::OnBnClickedSel2()
{
	Select(2);
}

void StudyListCtrlPage::Select(int idx)
{
	auto nc = mList.GetItemCount();
	if (idx >= 0 && idx < nc)
	{
		mList.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	}
}

void StudyListCtrlPage::OnLvnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	auto sel = mList.GetNextItem(-1, LVNI_SELECTED);
	LogV(TAG, "Itemchanged,sel=%d", sel);
}

void StudyListCtrlPage::OnLvnItemActivateList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;


	LogV(TAG, "%s", __func__);
}
