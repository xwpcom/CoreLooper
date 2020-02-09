#include "pch.h"
#include "dt.app.h"
#include "LogPage.h"
#include "afxdialogex.h"
#include "LogFilterPage.h"
#include "LogItemPage.h"
#include "logmanager.h"
IMPLEMENT_DYNAMIC(LogPage, BasePage)

enum
{
	eTimer_DelayRefreshItemPage,
};

enum
{
	eIdxMsg,
	eIdxTime,
	eIdxFileLine,
	eIdxPid,
	eIdxTag,
	eIdxApp,
};


LogPage::LogPage(CWnd* pParent /*=nullptr*/)
	: BasePage(IDD_LogPage, pParent)
{
	bzero(mArrIdx, sizeof(mArrIdx));
}

LogPage::~LogPage()
{
}

void LogPage::DoDataExchange(CDataExchange* pDX)
{
	BasePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, mListCtrl);
}

BEGIN_MESSAGE_MAP(LogPage, BasePage)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &LogPage::OnNMRClickList)
	ON_COMMAND(ID_OPEN_FILE_GOTO_LINE, &LogPage::OnOpenFileGotoLine)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &LogPage::OnNMDblclkList)
	ON_COMMAND(ID_COPY_FULL_PATH, &LogPage::OnCopyFullPath)
	ON_COMMAND(ID_OPEN_FOLDER, &LogPage::OnOpenFolder)
	ON_COMMAND(ID_APP_DISABLE, &LogPage::OnAppDisable)
	ON_COMMAND(ID_APP_CLEAR, &LogPage::OnAppClear)
	ON_COMMAND(ID_TAG_DISABLE, &LogPage::OnTagDisable)
	ON_COMMAND(ID_TAG_CLEAR, &LogPage::OnTagClear)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &LogPage::OnLvnItemchangedList)
	ON_WM_TIMER()
	ON_COMMAND(ID_CLEAR, &LogPage::OnClear)
	ON_COMMAND(ID_COPY, &LogPage::OnCopy)
	ON_COMMAND(ID_COPY_ALL, &LogPage::OnCopyAll)
END_MESSAGE_MAP()

COLORREF gColors[] =
{
	RGB(255, 0, 0),//error
	RGB(255, 0, 255),//warning
	RGB(0, 0, 255),//notice
	RGB(0, 0, 0),//trace
	RGB(128, 128, 128),//verbose
};

static void test();

int LogPage::Init()
{
	auto ret=__super::Init();
	ASSERT(mLogManager);

	{
		CRect rc(0, 0, 100, 100);
		mFilterPage = make_shared<LogFilterPage>();
		mFilterPage->SetIniSection(mSection+".filterPage");
		mFilterPage->SetLogManager(mLogManager);
		mFilterPage->Create(IDD_LogFilterPage,this);
		mFilterPage->ShowWindow(SW_SHOW);

		mFilterPage->SignalClearLog.connect(this, &LogPage::OnClearLog);
		mFilterPage->SignalFilterChanged.connect(this, &LogPage::OnFilterChanged);
	}
	{
		CRect rc(0, 0, 100, 100);
		mItemPage = make_shared<LogItemPage>();
		mItemPage->SetIniSection(mSection +".itemPage");
		mItemPage->Create(IDD_LogItemPage, this);
		mItemPage->ShowWindow(SW_SHOW);
	}

	mFont.DeleteObject();
	mFont.CreatePointFont(160, _T("新宋体"));

	CListCtrl& list = mListCtrl;
	list.DeleteAllItems();
	{
		mListCtrl.SignalContextMenu.connect(this, &LogPage::OnContextMenu);
	}

	list.SetFont(&mFont);
	list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT
		| LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP);// | LVS_EX_LABELTIP );
	//list.ModifyStyle(LVS_SINGLESEL,NULL);
	{
		class LogListCtrlNodeProxy:public ListCtrlNodeProxy
		{
		public:
			virtual ~LogListCtrlNodeProxy() {}

			virtual COLORREF OnGetCellTextColor(DWORD_PTR context, int row, int col)
			{
				auto item = (LogItem*)context;
				auto level = item->level;
				if (level >= 0 && level < COUNT_OF(gColors))
				{
					return gColors[level];
				}
				return 0;
			}

			virtual COLORREF OnGetCellBkColor(DWORD_PTR context, int row, int col)
			{
				auto item = (LogItem*)context;
				static COLORREF clrBK = GetSysColor(COLOR_WINDOW);
				return clrBK;
			}
		};

		mListCtrl.SetNodeProxy(make_shared<LogListCtrlNodeProxy>());
	}

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

	//VERIFY(SetTimer(IDT_DETECTSAVE, 2000, NULL) == IDT_DETECTSAVE);
	{
		auto cols = mIni->GetString(mSection, "cols");
		if (!cols.empty())
		{
			vector<int> nItems;
			StringTool::ExtractInts(cols, ',', nItems);
			if (nItems.size() > 0)
			{
				list.SetColumnOrderArray(nItems.size(), &nItems[0]);
			}

			auto widths = mIni->GetString(mSection, "widths");
			StringTool::ExtractInts(widths, ',', nItems);
			for (size_t i = 0; i < nItems.size(); ++i)
			{
				list.SetColumnWidth(i, nItems[i]);
			}
		}
	}

	test();

	return ret;
}

//*
static const char* TAG = "LogPage";
void test()
{
	DV("DV");
	DT("DT\r\nline2\r\nline3");
	DG("DG\r\nline2\r\nline3");
	DW("DW\r\nline2\r\nline3");
	DE("DE");

	LogV(TAG, "LogV,hello,%d", 2020);
	LogD(TAG, "LogD\r\nline2\r\nline3"); 
	LogI(TAG, "LogI\r\nline2\r\nline3");
	LogW(TAG, "LogW\r\nline2\r\nline3");
	LogE(TAG,"LogE");

}
//*/

void LogPage::OnLogItemReady(shared_ptr<LogItem> item)
{
	mLogItems.push_back(item);

	if (mFilterPage->Filter(item) == 0)
	{
		AddItem(item);
	}

	//int mMaxLogCount = 1;
	if ((int)mLogItems.size() > mMaxLogCount)
	{
		mLogItems.pop_front();
	}							   

	if ((int)mListCtrl.GetItemCount() > mMaxLogCount)
	{
		auto idx = 0;
		auto item = (LogItem*)mListCtrl.GetItemData(idx);
		mListCtrl.DeleteItem(idx);
		item->mRefs.pop_back();
	}

}

void LogPage::AddItem(shared_ptr<LogItem>& item)
{
	auto& list = mListCtrl;
	string time;
	//仅在hour为0或23时添加date,否则只显示time,hhmmssMMM
	auto hour = item->time / 10000000;
	if (hour==0 || hour==23)
	{
		//date:yymmdd
		auto m = (item->date / 100) % 100;
		auto d = item->date % 100;
		time = StringTool::Format("%02d.%02d ",m,d);
	}

	{
		auto minute = (item->time / 100000)%100;
		auto second = (item->time / 1000) % 100;
		auto ms = item->time % 1000;
		StringTool::AppendFormat(time, "%02d:%02d:%02d.%03d",hour,minute,second,ms);
	}
	
	int nIndex = list.GetItemCount();
	{
		//为简单起见，这里没再严格按date,time排序，绝大部分情况下顺序是正常的

	}
	auto idx=list.InsertItem(nIndex, _T(""));
	item->mRefs.push_back(item);//从list删除时要释放
	list.SetItemData(idx, (DWORD_PTR)item.get());

	USES_CONVERSION;
	list.SetItemText(nIndex, mArrIdx[eIdxApp], A2T(item->appName.c_str()));
	list.SetItemText(nIndex, mArrIdx[eIdxTag], A2T(item->tag.c_str()));
	list.SetItemText(nIndex, mArrIdx[eIdxTime], A2T(time.c_str()));
	list.SetItemText(nIndex, mArrIdx[eIdxPid], A2T(StringTool::Format("%d/%d",item->pid,item->tid).c_str()));
	list.SetItemText(nIndex, mArrIdx[eIdxFileLine], A2T(StringTool::Format("%s(%d)", item->file.c_str(),item->line).c_str()));
	
	{
		auto data = item->msg;
		StringTool::Replace(data,"\r", "^");
		StringTool::Replace(data,"\n", "~");
		StringTool::Replace(data,"\t","`");

		list.SetItemText(nIndex, mArrIdx[eIdxMsg], A2T(data.c_str()));

	}

	//仅当选中最后一项时才更新选中Item
	if (list.GetNextItem(-1, LVNI_SELECTED) == nIndex - 1)
	{
		list.SetItemState(nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		list.EnsureVisible(nIndex, FALSE);
	}
}

void LogPage::OnDestroy()
{
	__super::OnDestroy();
	OnClearLog();
}

void LogPage::OnRelayout(const CRect& rc)
{
	__super::OnRelayout(rc);

	if (!mListCtrl.GetSafeHwnd())
	{
		return;
	}

	CRect rcItem = rc;
	rcItem.bottom = rcItem.top + 32;
	mFilterPage->MoveWindow(rcItem);

	rcItem.top = rcItem.bottom + 1;
	rcItem.bottom = rc.bottom - 200;
	if (rcItem.bottom < rcItem.top)
	{
		rcItem.bottom = rcItem.top;
	}

	mListCtrl.MoveWindow(rcItem);

	rcItem.top = rcItem.bottom;
	rcItem.bottom = rc.bottom;
	mItemPage->MoveWindow(rcItem);
}

void LogPage::OnClearLog()
{
	ClearLogListCtrl();
	mLogItems.clear();
}

void LogPage::ClearLogListCtrl()
{
	for(int i=0;i<(int)mListCtrl.GetItemCount();++i)
	{
		auto item = (LogItem*)mListCtrl.GetItemData(i);
		item->mRefs.pop_back();
	}

	mListCtrl.DeleteAllItems();
}

void LogPage::OnFilterChanged()
{
	ClearLogListCtrl();

	for (auto& item : mLogItems)
	{
		if (mFilterPage->Filter(item) == 0)
		{
			AddItem(item);
		}
	}
}

void LogPage::LoadConfig()
{
	__super::LoadConfig();
}

void LogPage::SaveConfig()
{
	__super::SaveConfig();

	auto& list = mListCtrl;
	const int columnCount = list.GetHeaderCtrl().GetItemCount();
	int nArr[100];
	list.GetColumnOrderArray(nArr, columnCount);
	string cols;
	for (int i = 0; i < columnCount; i++)
	{
		cols += StringTool::Format("%d,", nArr[i]);
	}
	mIni->SetString(mSection, "cols", cols);

	//save column width
	string widths;
	CString szKey;
	for (int i = 0; i < columnCount; i++)
	{
		widths += StringTool::Format("%d,",list.GetColumnWidth(i));
	}

	mIni->SetString(mSection, "widths", widths);
}

void LogPage::RemoveSection()
{
	DisableAutoSaveConfig();
	
	mIni->RemoveSection(mSection);
	
	mFilterPage->DisableAutoSaveConfig();
	mIni->RemoveSection(mFilterPage->GetIniSection());

	mItemPage->DisableAutoSaveConfig();
	mIni->RemoveSection(mItemPage->GetIniSection());
}


void LogPage::OnNMRClickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	DV("%s", __func__);

	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void LogPage::OnContextMenu(CWnd* pwnd, CPoint point)
{
	if (pwnd == &mListCtrl)
	{
		CMenu menu;
		menu.LoadMenu(IDR_LOG);
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	}
}

void LogPage::OnOpenFileGotoLine()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	if (!File::FileExists(item->file))
	{
		ShowToast(_T("file no found"));
		return;
	}

	string cmd = StringTool::Format(
		"%s/OpenFileGotoLine.exe \"%s\" %d"
		, ShellTool::GetAppPath().c_str()
		,item->file.c_str()
		,item->line
	);

	auto ret=WinExec(cmd.c_str(), SW_SHOW);
	DV("ret=%d,%s(%d)", ret,item->file.c_str(), item->line);

}


void LogPage::OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	OnOpenFileGotoLine();
}


LogItem* LogPage::GetCurrentLogItem()
{
	auto& list = mListCtrl;
	int sel = list.GetNextItem(-1, LVNI_SELECTED);
	if (sel == -1)
	{
		return nullptr;
	}

	auto item = (LogItem*)list.GetItemData(sel);
	return item;
}

void LogPage::OnCopyFullPath()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	if (!File::FileExists(item->file))
	{
		ShowToast(_T("file no found"));
		return;
	}

	USES_CONVERSION;
	CString text = A2T(item->file.c_str());
	ShellTool::CopyTextToClipboard(GetSafeHwnd(), text);
}

void LogPage::OnOpenFolder()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	if (!File::FileExists(item->file))
	{
		ShowToast(_T("file no found"));
		return;
	}

	ShellTool::ShowInFolder(item->file);
}


void LogPage::OnAppDisable()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	mFilterPage->DisableApp(item->appName);
}

void LogPage::OnAppClear()
{
	mFilterPage->ClearApp();
}

void LogPage::OnTagDisable()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	mFilterPage->DisableTag(item->tag);
}

void LogPage::OnTagClear()
{
	mFilterPage->ClearTag();
}

void LogPage::OnLvnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	*pResult = 0;

	//delay refresh to avoid too much unnecessary actions
	SetTimer(eTimer_DelayRefreshItemPage, 100, nullptr);
}

void LogPage::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case eTimer_DelayRefreshItemPage:
	{
		KillTimer(nIDEvent);

		auto item = GetCurrentLogItem();
		mItemPage->SetLogItem(item);
		return;
	}
	}

	__super::OnTimer(nIDEvent);
}


void LogPage::OnClear()
{
	OnClearLog();
}

void LogPage::OnCopy()
{
	auto item = GetCurrentLogItem();
	if (item)
	{
		ShellTool::CopyTextToClipboard(GetSafeHwnd(), item->msg);
	}
}

void LogPage::OnCopyAll()
{
	string text;
	auto& list = mListCtrl;
	int count = list.GetItemCount();
	for (int i = 0; i < count; i++)
	{
		auto item = (LogItem*)list.GetItemData(i);

		text += item->msg + "\r\n";

	}

	ShellTool::CopyTextToClipboard(GetSafeHwnd(), text);
}
