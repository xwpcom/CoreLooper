#include "stdafx.h"
#include "include/CategoryPage.h"
#include "afxdialogex.h"

enum
{
	eTimer_DelayActiveTab = 1000,
};

IMPLEMENT_DYNAMIC(CategoryPage, BasePage)

CategoryPage::CategoryPage(UINT idd,CWnd* pParent /*=NULL*/)
	: BasePage(idd, pParent)
{

}

CategoryPage::~CategoryPage()
{
}

void CategoryPage::DoDataExchange(CDataExchange* pDX)
{
	BasePage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CategoryPage, BasePage)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CategoryPage message handlers
int CategoryPage::Init()
{
	__super::Init();

	CRect rc(0, 0, 100, 100);
	mTab.Create(CMFCTabCtrl::STYLE_3D_VS2005, rc, this, 1, CMFCTabCtrl::LOCATION_TOP);
	mTab.ModifyStyle(NULL, WS_CLIPCHILDREN);

	{

		CWnd *parent = &mTab;
		/*
#if _MSC_VER >=1900
		//vs2015
		parent = this;
#elif _MSC_VER >=1800
		//vs2013
#else
#endif
*/
		auto arr = GetPages();
		for (int i = 0; i < (int)arr.size(); i++)
		{
			arr[i].mPage->Create(arr[i].mIdd, parent);
			mTab.AddTab(arr[i].mPage.get(), arr[i].mTitle);

			int tabId = mTab.GetTabID(i);
			ASSERT(mGroups.find(tabId) == mGroups.end());
			mGroups[tabId] = arr[i].mPage;
		}

		if (mTab.GetTabsNum() > 1)
		{
			int activeTab = mIni->GetInt(mSection, "activeTab", 0);
			mTab.SetActiveTab(!activeTab);//确保执行切换tab的动作
			SetTimer(eTimer_DelayActiveTab, 1, NULL);

		}
		int x = 0;
	}

	UpdateControlPos();
	return 0;
}

void CategoryPage::UpdateControlPos()
{
	if (!mTab.GetSafeHwnd())
	{
		return;
	}

	CRect rc;
	GetClientRect(rc);
	rc.DeflateRect(mTabDeflateRect);
	mTab.MoveWindow(rc);
}

void CategoryPage::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case eTimer_DelayActiveTab:
	{
		KillTimer(nIDEvent);

		int activeTab = mIni->GetInt(mSection, "activeTab", 0);
		if (activeTab < 0 && activeTab >= mTab.GetTabsNum())
		{
			activeTab = 0;
		}
		mTab.SetActiveTab(activeTab);
		break;
	}
	}

	__super::OnTimer(nIDEvent);
}

void CategoryPage::OnDestroy()
{
	__super::OnDestroy();

	int activeTab = mTab.GetActiveTab();
	if (mIni)
	{
		mIni->SetInt(mSection, "activeTab", activeTab);
	}
}
