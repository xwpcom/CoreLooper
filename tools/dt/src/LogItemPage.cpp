#include "pch.h"
#include "LogItemPage.h"
#include "afxdialogex.h"
#include "resource.h"
#include "logwnd.h"

IMPLEMENT_DYNAMIC(LogItemPage, BasePage)

LogItemPage::LogItemPage(CWnd* pParent /*=nullptr*/)
	: BasePage(IDD_LogItemPage, pParent)
{

}

LogItemPage::~LogItemPage()
{
}

void LogItemPage::DoDataExchange(CDataExchange* pDX)
{
	BasePage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(LogItemPage, BasePage)
	ON_BN_CLICKED(IDC_COPY, &LogItemPage::OnBnClickedCopy)
END_MESSAGE_MAP()


int LogItemPage::Init()
{
	{
		mEdit = make_shared<LogWnd>();
		CRect rc(0, 0, 100, 100);
		mEdit->Create(0, WS_VISIBLE | WS_CHILD, rc, this, 1000);
	}

	return __super::Init();
}

void LogItemPage::OnRelayout(const CRect& rc)
{
	__super::OnRelayout(rc);

	if (!mEdit->GetSafeHwnd())
	{
		return;
	}

	if (0)
	{
		CRect rcItem;
		GetDlgItem(IDC_COPY)->GetWindowRect(rcItem);
		ScreenToClient(rcItem);

		CRect rcEdit(rc);
		rcEdit.left = rcItem.right + 1;
		mEdit->MoveWindow(rcEdit);
	}
	else
	{
		GetDlgItem(IDC_COPY)->ShowWindow(SW_HIDE);
		mEdit->MoveWindow(rc);
	}
}

void LogItemPage::clear()
{
	SetLogItem(nullptr);
}

extern COLORREF gColors[];

void LogItemPage::SetLogItem(LogItem* item)
{
	mEdit->clear();

	if (!item)
	{
		return;
	}

	mEdit->SetTextColor(gColors[item->level]);

	/*
	string text = StringTool::Format(
		"%s\r\n"
		"\r\n"
		"app:%s\r\n"
		"tag:%s\r\n"
		"pid:%d\r\n"
		"tid:%d\r\n"
		"file:%s\r\n"
		"line:%d\r\n"
		"time:%d %d\r\n"
		,item->msg.c_str()
		,item->appName.c_str()
		,item->tag.c_str()
		,item->pid
		,item->tid
		,item->file.c_str()
		,item->line
		,item->date,item->time

	);
	mEdit->AddLog(text);
	*/

	mEdit->AddLog(item->msg);
}


void LogItemPage::OnBnClickedCopy()
{
	mEdit->CopyAll();
}
