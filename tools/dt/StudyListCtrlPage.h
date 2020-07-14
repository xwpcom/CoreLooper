#pragma once


#include "LogListCtrl.h"

class StudyListCtrlPage : public CDialogEx
{
	DECLARE_DYNAMIC(StudyListCtrlPage)

public:
	StudyListCtrlPage(CWnd* pParent = nullptr);   // standard constructor
	virtual ~StudyListCtrlPage();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_StudyListCtrlPage };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CFont mFont;
	int mArrIdx[10] = {0};
	void Select(int idx);
public:
	afx_msg void OnBnClickedAddItem();
	afx_msg void OnBnClickedSel0();
	afx_msg void OnBnClickedSel1();
	afx_msg void OnBnClickedSel2();
	
	//CListCtrl mList;
	LogListCtrl mList;

	virtual BOOL OnInitDialog();
	afx_msg void OnLvnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemActivateList(NMHDR* pNMHDR, LRESULT* pResult);
};
