#include "stdafx.h"
#include "include/basepage.h"
#include "base64ex.h"
#include "core/string/utf8tool.h"

using namespace Bear::Core;

enum
{
	IDT_KICKIDLE = 8000,
	eTimer_DelayCloseDialog,
};

IniFile* BasePage::mIni;

BEGIN_MESSAGE_MAP(BasePage, CDialogEx)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

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
	ASSERT(mIni);
	mHasInit = true;

	LoadConfig();

	if (mSaveAndStoreWindowsPosition)
	{
		USES_CONVERSION;
		ShellTool::LoadWindowPos(m_hWnd, A2T(mSection.c_str()));
	}

	return 0;
}

void BasePage::OnTimer(UINT_PTR id)
{
	switch (id)
	{
	case IDT_KICKIDLE:
	{
		UpdateDialogControls(this, mDisableIfNoHndler);
		break;
	}
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

void BasePage::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);

	UpdateControlPos();

	OnRelayout(CRect(0,0,cx,cy));
}

void BasePage::OnRelayout(const CRect& )
{

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

	if (mAutoSaveConfig)
	{
		SaveConfig();

		if (mSaveAndStoreWindowsPosition)
		{
			USES_CONVERSION;
			ShellTool::SaveWindowPos(m_hWnd, A2T(mSection.c_str()));
		}

	}

	if (mToast)
	{
		mToast->DestroyWindow();
		mToast = nullptr;
	}
}

void BasePage::SaveDlgItemStringBase64(UINT id, CString name)
{
	CString text;
	GetDlgItemText(id, text);
	USES_CONVERSION;
	string data = Base64::Encode((const char*)T2A(text));
	mIni->SetString(mSection, T2A(name), data);
}

void BasePage::LoadDlgItemStringBase64(UINT id, CString name, CString defaultValue)
{
	USES_CONVERSION;
	CString value = A2T(mIni->GetString(mSection, T2A(name), T2A(defaultValue)).c_str());
	auto data = Base64::Decode(T2A((LPCTSTR)value));
	if (data.empty())
	{
		data = T2A(defaultValue);
	}
	SetDlgItemText(id, A2T(data.c_str()));
}

void BasePage::SaveDlgItemString(UINT id, CString name)
{
	CString value;
	GetDlgItemText(id, value);
	mIni->SetStringMFC(mSection, name, value);
}

void BasePage::SaveDlgItemInt(UINT id, CString name)
{
	UINT value = GetDlgItemInt(id);
	mIni->SetInt(mSection, name, value);
}

void BasePage::SaveDlgItemString(UINT id, const string& name)
{
	CString text;
	GetDlgItemText(id, text);
	auto v=Utf8Tool::UNICODE_to_UTF8(text);
	mIni->SetString(mSection,name,v);
}
void BasePage::LoadDlgItemString(UINT id, const string& name, const string& defaultValue)
{
	auto text = mIni->GetString(mSection, name, defaultValue);
	auto v = Utf8Tool::UTF8_to_UNICODE(text);
	SetDlgItemText(id, v);
}

void BasePage::LoadDlgItemString(UINT id, CString name, CString defaultValue)
{
	USES_CONVERSION;
	CString value = mIni->GetStringMFC(mSection, name, defaultValue);
	SetDlgItemText(id, value);
}

void BasePage::SaveCombo(UINT id, const string& name)
{
	USES_CONVERSION;
	SaveCombo(id, A2T(name.c_str()));
}
void BasePage::LoadCombo(UINT id, const string& name, const string& defaultValue)
{
	USES_CONVERSION;
	LoadCombo(id, A2T(name.c_str()), Utf8Tool::UTF8_to_UNICODE(defaultValue));
}

void BasePage::LoadCombo(UINT id, CString name, CString defaultValue)
{
	auto item = (CComboBox*)GetDlgItem(id);
	ASSERT(item);

	auto value = mIni->GetStringMFC(mSection, name, defaultValue);
	item->SelectString(-1, value);
}

void BasePage::SaveCombo(UINT id, CString name)
{
	auto item = (CComboBox*)GetDlgItem(id);
	ASSERT(item);

	auto sel = item->GetCurSel();
	if (sel != -1)
	{
		CString value;
		item->GetLBText(sel, value);
		//mIni->SetStringMFC(mSection, name, value);
		mIni->SetString(mSection, Utf8Tool::UNICODE_to_UTF8(name), Utf8Tool::UNICODE_to_UTF8(value));
	}
}

void BasePage::SaveCheck(UINT id, CString name)
{
	auto checked = IsDlgButtonChecked(id);
	mIni->SetInt(mSection, name, checked);
}

void BasePage::LoadCheck(UINT id, CString name, int defaultValue)
{
	auto checked = mIni->GetInt(mSection, name, defaultValue);
	CheckDlgButton(id, checked);
}

void BasePage::LoadDlgItemInt(UINT id, CString name, int defaultValue)
{
	int value = mIni->GetInt(mSection, name, defaultValue);
	SetDlgItemInt(id, value);
}

void BasePage::SetInt(CString name, int value)
{
	mIni->SetInt(mSection, name, value);
}

void BasePage::SetString(CString name, CString value)
{
	mIni->SetStringMFC(mSection, name, value);
}

int BasePage::GetInt(CString name, int defaultValue)
{
	USES_CONVERSION;
	return mIni->GetInt(mSection, T2A(name), defaultValue);
}

CString BasePage::GetString(CString name, CString defaultValue)
{
	USES_CONVERSION;
	return A2T(mIni->GetString(mSection, T2A(name), T2A(defaultValue)).c_str());
}

CString BasePage::GetItemText(UINT id)
{
	return GetText(id);
}

void BasePage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	//DT("(0x%x)CBasePage::OnShowWindow(bShow=%d)",this,bShow);
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow)
	{
		auto ret = SetTimer(IDT_KICKIDLE, 200, NULL);
		//DW("SetTimer(IDT_KICKIDLE),ret=%d",ret);

		if (mWaitFirstShow)
		{
			mWaitFirstShow = false;
			OnFirstShow();
		}
	}
	else
	{
		KillTimer(IDT_KICKIDLE);
		//DW("KillTimer(IDT_KICKIDLE)");
	}
}

//窗口首次可见时调用本接口
void BasePage::OnFirstShow()
{
	if (!mHasInit)
	{
		Init();
	}
}

void BasePage::PreClose()
{

}
