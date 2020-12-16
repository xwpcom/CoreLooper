#pragma once

#include "basepage.h"
#include "loginfo.h"

class CMFCMenuButtonExProxy
{
public:
	virtual ~CMFCMenuButtonExProxy() {}
	virtual void OnUpdateItem(CCmdUI* pCmdUI)=0;
};

//CMFCMenuButton的扩展性不好，所以只能采用扩展类来补救
class CMFCMenuButtonEx :public CMFCMenuButton
{
public:
	CMFCMenuButtonEx(){}
	
	CMFCMenuButtonExProxy* mProxy = nullptr;
protected:
	DECLARE_MESSAGE_MAP()
	void OnUpdateItem(CCmdUI* pCmdUI);
	void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
};

class LogManager;

class LogFilterPage : public BasePage,public CMFCMenuButtonExProxy
{
	DECLARE_DYNAMIC(LogFilterPage)

public:
	LogFilterPage(CWnd* pParent = nullptr);   // standard constructor
	virtual ~LogFilterPage();
	void SetLogManager(shared_ptr<LogManager> obj)
	{
		mLogManager = obj;
	}

	int Filter(shared_ptr<LogItem> item);
	sigslot::signal0<> SignalClearLog;
	sigslot::signal0<> SignalFilterChanged;

	void DisableApp(const string& app);
	void ClearApp();

	void DisableTag(const string& tag);
	void ClearTag();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LogFilterPage };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	void OnRelayout(const CRect&);
	DECLARE_MESSAGE_MAP()
	int Init();
	virtual void LoadConfig();
	virtual void SaveConfig();

	struct tagFilter
	{
		unordered_map<string, bool>	apps;
		unordered_map<string, bool>	tags;
		unordered_map<DWORD, bool>	pids;
		unordered_map<DWORD, bool>	tids;
		eDTLevel level = DT_VERBOSE;
		//bool						defaultOn=true;
		bool onlyDisableApp = true;//仅apps中有项目，并且都是disabled项目时，可显示其他app
		bool onlyDisableTag = true;

		void clear()
		{
			apps.clear();
			tags.clear();
			pids.clear();
			tids.clear();
			//defaultOn = true;
			onlyDisableApp = true;
			onlyDisableTag = true;
		}

		void dump();
		int Filter(shared_ptr<LogItem> item);
	}mFilterInfo;

	//CMenu mMenu;

	void RefreshFilter();
	void OnApp(UINT id);
	void OnUpdateApp(CCmdUI* pCmdUI);
	virtual void OnUpdateItem(CCmdUI* pCmdUI);
public:
	afx_msg void OnEnChangeEditApp();
	afx_msg void OnEnChangeEditTag();
	afx_msg void OnEnChangeEditProcess();


	afx_msg void OnBnClickedClearFilter();
	afx_msg void OnBnClickedCheckDefaultOn();
	afx_msg void OnBnClickedClear();
	void OnBtnApp();
	//CMFCMenuButtonEx mBtnApp;
	CButton mBtnApp;
	bool mAutoScroll = true;

	shared_ptr<LogManager> mLogManager;

	afx_msg void OnBnClickedTag();
	CComboBox mComboLevel;
	afx_msg void OnCbnSelchangeCmbLevel();
	afx_msg void OnBnClickedChkAutoScroll();
};
