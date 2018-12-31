#pragma once
#include "string/textprotocol.h"
#include "ToastWnd.h"

using namespace std;

class WIN_CLASS BasePage :public CDialogEx
{
public:
	BasePage(UINT idd=0, CWnd* pParent = NULL)
		: CDialogEx(idd, pParent)
	{
		mEnableCancel = false;
	}

	void DelayCloseDialog(int ms=500);
	void EnableAutoSavePosition()
	{
		ASSERT(mSection != "BasePage");
		mSaveAndStoreWindowsPosition = true;
	}

	CString GetString(UINT id);
	CString GetText(UINT id);
	int SetText(UINT id, CString text);
	int SetInt(UINT id, int value);
	int GetInt(UINT id);
	void OnOK(){}
	void OnCancel()
	{
		if (mEnableCancel)
		{
			__super::OnCancel();
		}
	}

	CString mSection;
	bool mEnableCancel;
	bool	mSaveAndStoreWindowsPosition=false;
	shared_ptr<ToastWnd> mToast;
	shared_ptr<ToastWnd> GetToast();
	void ShowToast(CString text, int ms=1000);

protected:
	virtual BOOL OnInitDialog();
	virtual int Init();
	virtual void UpdateControlPos();
	void OnTimer(UINT_PTR id);

	virtual void LoadConfig();
	virtual void SaveConfig();

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
};
