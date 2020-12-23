#pragma once
#include "core/string/textprotocol.h"
#include "ToastWnd.h"

using namespace std;
using namespace Bear::Core::FileSystem;

class WIN_CLASS BasePage :public CDialogEx
{
public:
	BasePage(UINT idd=0, CWnd* pParent = NULL)
		: CDialogEx(idd, pParent)
	{
		mEnableCancel = false;
	}

	void SetIniSection(const string& section)
	{
		mSection = section;
	}

	const string& GetIniSection()const
	{
		return mSection;
	}

	void EnableAutoSaveConfig()
	{
		mAutoSaveConfig = true;
	}

	void DisableAutoSaveConfig()
	{
		mAutoSaveConfig = false;
	}

	virtual void PreClose();
	static void SetIni(IniFile *ini)
	{
		mIni = ini;
	}
	static IniFile *mIni;

	void DelayCloseDialog(int ms=500);
	void EnableAutoSavePosition()
	{
		ASSERT(mSection != "BasePage");
		mSaveAndStoreWindowsPosition = true;
	}

	int GetCurSel(CListCtrl& list, bool enableFocus = true);
	void SetReadOnly(UINT id, bool readOnly = true);
	CString GetString(UINT id);
	CString GetText(UINT id);
	int SetText(UINT id, CString text);
	int SetInt(UINT id, int value);
	int GetInt(UINT id);
	CString GetItemText(UINT id);

	void SaveDlgItemStringBase64(UINT id, CString name);
	void LoadDlgItemStringBase64(UINT id, CString name, CString defaultValue = _T(""));
	void SaveDlgItemString(UINT id, CString name);
	void SaveDlgItemInt(UINT id, CString name);
	void LoadDlgItemString(UINT id, CString name, CString defaultValue = _T(""));
	void LoadDlgItemInt(UINT id, CString name, int defaultValue = 0);
	void SetInt(CString name, int value);
	void SetString(CString name, CString value);
	void SaveCheck(UINT id, CString name);
	void SaveCheck(UINT id, string name)
	{
		USES_CONVERSION;
		SaveCheck(id,A2T(name.c_str()));
	}
	void LoadCheck(UINT id, CString name, int defaultValue = 0);
	void LoadCheck(UINT id, const string& name, int defaultValue = 0)
	{
		USES_CONVERSION;
		LoadCheck(id, A2T(name.c_str()), defaultValue);
	}
	void SaveCombo(UINT id, CString name);
	void SaveCombo(UINT id, string name)
	{
		USES_CONVERSION;
		SaveCombo(id, A2T(name.c_str()));
	}
	void LoadCombo(UINT id, CString name, CString defaultValue = _T(""));
	void LoadCombo(UINT id, const string& name, const string& defaultValue = "")
	{
		USES_CONVERSION;
		LoadCombo(id, A2T(name.c_str()), A2T(defaultValue.c_str()));
	}

	int GetInt(CString name, int defaultValue = 0);
	CString GetString(CString name, CString DefaultValue = _T(""));

	void EnableItem(UINT id);
	void DisableItem(UINT id);
	void ShowItem(UINT id);
	void HideItem(UINT id);

	void OnOK()
	{
		if (mEnableOK)
		{
			__super::OnOK();
		}
	
	}
	void OnCancel()
	{
		if (mEnableCancel)
		{
			__super::OnCancel();
		}
	}

	void ShowToast(CString text, int ms=1000);
	virtual UINT_PTR SetTimer(UINT_PTR nIDEvent, UINT nElapse,
		void (CALLBACK* lpfnTimer)(HWND, UINT, UINT_PTR, DWORD)=nullptr)
	{
		
		return __super::SetTimer(nIDEvent, nElapse, lpfnTimer);
	}
protected:
	virtual BOOL OnInitDialog();
	virtual int Init();
	virtual void UpdateControlPos();
	virtual void OnRelayout(const CRect& rc);
	virtual void OnTimer(UINT_PTR id);
	void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnFirstShow();

	virtual void LoadConfig();
	virtual void SaveConfig();

	string mSection;
	bool mEnableOK=false;
	bool mEnableCancel;
	bool	mAutoSaveConfig = true;
	bool	mSaveAndStoreWindowsPosition = false;
	bool	mWaitFirstShow = true;
	bool	mDisableIfNoHndler = true;
	bool mHasInit = false;
	shared_ptr<ToastWnd> mToast;
	shared_ptr<ToastWnd> GetToast();

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
};
