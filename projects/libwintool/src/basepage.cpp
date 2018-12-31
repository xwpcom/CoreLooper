#include "stdafx.h"
#include "include/basepage.h"
using namespace Bear::Core;

enum
{
	eTimer_DelayCloseDialog=10000000,
};

CString BasePage::GetString(UINT id)
{
	CString text;
	GetDlgItemText(id, text);
	return text;
}

int BasePage::SetText(UINT id, CString text)
{
	SetDlgItemText(id, text);
	return 0;
}

int BasePage::SetInt(UINT id, int value)
{
	CString text;
	text.Format(_T("%d"), value);
	SetDlgItemText(id, text);
	return 0;
}

CString BasePage::GetText(UINT id)
{
	CString text;
	GetDlgItemText(id, text);
	return text;
}

int BasePage::GetInt(UINT id)
{
	USES_CONVERSION;
	return atoi(T2A(GetText(id)));
}

BOOL BasePage::OnInitDialog()
{
	__super::OnInitDialog();

	Init();

	return FALSE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

int BasePage::Init()
{
	LoadConfig();

	if (mSaveAndStoreWindowsPosition)
	{
		ShellTool::LoadWindowPos(m_hWnd, mSection);
	}

	return 0;
}

void BasePage::OnTimer(UINT_PTR id)
{
	switch (id)
	{
	case eTimer_DelayCloseDialog:
	{
		EndDialog(IDOK);
		KillTimer(id);
		return;
	}
	}

	__super::OnTimer(id);
}

void BasePage::DelayCloseDialog(int ms)
{
	SetTimer(eTimer_DelayCloseDialog, ms,nullptr);
}

void BasePage::ShowToast(CString text,int ms)
{
	GetToast()->Show(this, text, ms);
}

shared_ptr<ToastWnd> BasePage::GetToast()
{
	if (!mToast)
	{
		mToast = make_shared<ToastWnd>();
		mToast->Create(CRect(0, 0, 0, 0),
			//WS_VISIBLE | 
			WS_POPUP);
		mToast->SetAlpha(160);

		mToast->SetWindowPos(nullptr, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

		//DW("toast.Create HWND=0x%08x", mToast->GetSafeHwnd());
	}

	ASSERT(mToast->GetSafeHwnd());
	return mToast;
}

BEGIN_MESSAGE_MAP(BasePage, CDialogEx)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


void BasePage::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	UpdateControlPos();
}

void BasePage::UpdateControlPos()
{

}

void BasePage::LoadConfig()
{

}

void BasePage::SaveConfig()
{

}

void BasePage::OnDestroy()
{
	CDialogEx::OnDestroy();

	SaveConfig();

	if (mSaveAndStoreWindowsPosition)
	{
		ShellTool::SaveWindowPos(m_hWnd, mSection);
	}

	if (mToast)
	{
		mToast->DestroyWindow();
		mToast = nullptr;
	}
}
