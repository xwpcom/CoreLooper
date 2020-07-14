#include "pch.h"
#include "dt.app.h"
#include "LogPageEx.h"
#include "afxdialogex.h"
#include "LogFilterPage.h"
#include "LogItemPage.h"
#include "logmanager.h"
#include "logwnd.h"
#include "core/string/utf8tool.h"

IMPLEMENT_DYNAMIC(LogPageEx, BasePage)

static const char* TAG = "LogPageEx";

enum
{
	eTimer_DelayRefreshItemPage,
	eTimer_DelayRelayout,
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


LogPageEx::LogPageEx(CWnd* pParent /*=nullptr*/)
	: BasePage(IDD_LogPageEx, pParent)
{
	bzero(mArrIdx, sizeof(mArrIdx));

	mSection = "LogPageEx";
	mEnableOK = true;
	mEnableCancel = true;
	EnableAutoSavePosition();
}

LogPageEx::~LogPageEx()
{
}

void LogPageEx::DoDataExchange(CDataExchange* pDX)
{
	BasePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, mListCtrl);
}

BEGIN_MESSAGE_MAP(LogPageEx, BasePage)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &LogPageEx::OnNMRClickList)
	ON_COMMAND(ID_OPEN_FILE_GOTO_LINE, &LogPageEx::OnOpenFileGotoLine)
	ON_COMMAND(ID_CODEPAGE_UTF8, &LogPageEx::OnCodePageUtf8)
	ON_COMMAND(ID_CODEPAGE_CHINESE, &LogPageEx::OnCodePageChinese)
	//ON_UPDATE_COMMAND_UI_RANGE(ID_CODEPAGE_UTF8, ID_CODEPAGE_UTF8+100, &LogPageEx::OnCodePageChinese)
	ON_UPDATE_COMMAND_UI(ID_CODEPAGE_UTF8, &LogPageEx::OnUpdateCodePageUtf8)
	ON_UPDATE_COMMAND_UI(ID_CODEPAGE_CHINESE, &LogPageEx::OnUpdateCodePageChinese)
	
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &LogPageEx::OnNMDblclkList)
	ON_COMMAND(ID_COPY_FULL_PATH, &LogPageEx::OnCopyFullPath)
	ON_COMMAND(ID_OPEN_FOLDER, &LogPageEx::OnOpenFolder)
	ON_COMMAND(ID_APP_DISABLE, &LogPageEx::OnAppDisable)
	ON_COMMAND(ID_APP_CLEAR, &LogPageEx::OnAppClear)
	ON_COMMAND(ID_TAG_DISABLE, &LogPageEx::OnTagDisable)
	ON_COMMAND(ID_TAG_CLEAR, &LogPageEx::OnTagClear)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &LogPageEx::OnLvnItemchangedList)
	ON_WM_TIMER()
	ON_COMMAND(ID_CLEAR, &LogPageEx::OnClear)
	ON_COMMAND(ID_COPY, &LogPageEx::OnCopy)
	ON_COMMAND(ID_COPY_ALL, &LogPageEx::OnCopyAll)
	ON_BN_CLICKED(IDC_ADD, &LogPageEx::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_ADD_ITEM, &LogPageEx::OnBnClickedAddItem)
	ON_BN_CLICKED(IDC_SEL_0, &LogPageEx::OnBnClickedSel0)
	ON_BN_CLICKED(IDC_SEL_1, &LogPageEx::OnBnClickedSel1)
	ON_BN_CLICKED(IDC_SEL_2, &LogPageEx::OnBnClickedSel2)
END_MESSAGE_MAP()

COLORREF gColors[] =
{
	RGB(255, 0, 0),//error
	RGB(255, 0, 255),//warning
	RGB(0, 0, 255),//notice
	RGB(0, 0, 0),//trace
	RGB(128, 128, 128),//verbose
};

int LogPageEx::Init()
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

		mFilterPage->SignalClearLog.connect(this, &LogPageEx::OnClearLog);
		mFilterPage->SignalFilterChanged.connect(this, &LogPageEx::OnFilterChanged);
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
		mListCtrl.SignalContextMenu.connect(this, &LogPageEx::OnContextMenu);
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

	{
		auto code=mIni->GetInt(mSection, "codepage", SC_CP_UTF8);
		auto edit = mItemPage->GetEdit();
		edit->SetCodePage(code);
	}

	SetTimer(eTimer_DelayRelayout,10);
	return ret;
}

void LogPageEx::OnLogItemReady(shared_ptr<LogItem> item)
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

void LogPageEx::AddItem(shared_ptr<LogItem>& item)
{
	mAddItemBusying = true;
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
	
	const int nIndex = list.GetItemCount();
	{
		//为简单起见，这里没再严格按date,time排序，绝大部分情况下顺序是正常的
	}
	const auto idx=list.InsertItem(nIndex, _T(""));
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

		CString text= Utf8Tool::UTF8_to_UNICODE(data.c_str(),data.length());
		list.SetItemText(nIndex, mArrIdx[eIdxMsg], text);

	}

	//仅当选中最后一项时才更新选中Item
	const auto sel = list.GetNextItem(-1, LVNI_SELECTED);
	if (sel == nIndex - 1)
	{
		dump("before set select");
		TRACE("%s#1\r\n", __func__);
		list.SetItemState(nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		TRACE("%s#2\r\n", __func__);
		list.EnsureVisible(nIndex, FALSE);
		TRACE("%s#3\r\n", __func__);
		dump("after set select");
	}

	mAddItemBusying = false;
}

void LogPageEx::OnDestroy()
{
	__super::OnDestroy();
	OnClearLog();
}

void LogPageEx::OnRelayout(const CRect& rc)
{
	__super::OnRelayout(rc);

	if (!mListCtrl.GetSafeHwnd() || !mFilterPage)
	{
		return;
	}

	int headerHeight = 0;
	{
		CRect rcItem;
		GetDlgItem(IDC_ADD)->GetWindowRect(rcItem);
		ScreenToClient(rcItem);
		headerHeight = rcItem.Height();
	}

	CRect rcItem = rc;
	rcItem.OffsetRect(0, headerHeight);
	rcItem.bottom = rcItem.top + 32;
	mFilterPage->MoveWindow(rcItem);

	int itemHeight = rc.Height() / 5;
	itemHeight = MAX(itemHeight, 320);

	rcItem.top = rcItem.bottom + 1;
	rcItem.bottom = rc.bottom - itemHeight;
	if (rcItem.bottom < rcItem.top)
	{
		rcItem.bottom = rcItem.top;
	}

	mListCtrl.MoveWindow(rcItem);

	rcItem.top = rcItem.bottom;
	rcItem.bottom = rc.bottom;
	mItemPage->MoveWindow(rcItem);
}

void LogPageEx::OnClearLog()
{
	ClearLogListCtrl();
	mLogItems.clear();
}

void LogPageEx::ClearLogListCtrl()
{
	for(int i=0;i<(int)mListCtrl.GetItemCount();++i)
	{
		auto item = (LogItem*)mListCtrl.GetItemData(i);
		item->mRefs.pop_back();
	}

	mListCtrl.DeleteAllItems();
}

void LogPageEx::OnFilterChanged()
{
	//TRACE("%s\r\n", __func__);

	ClearLogListCtrl();

	for (auto& item : mLogItems)
	{
		if (mFilterPage->Filter(item) == 0)
		{
			AddItem(item);
		}
	}
}

void LogPageEx::LoadConfig()
{
	__super::LoadConfig();
}

void LogPageEx::SaveConfig()
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

void LogPageEx::RemoveSection()
{
	DisableAutoSaveConfig();
	
	mIni->RemoveSection(mSection);
	
	mFilterPage->DisableAutoSaveConfig();
	mIni->RemoveSection(mFilterPage->GetIniSection());

	mItemPage->DisableAutoSaveConfig();
	mIni->RemoveSection(mItemPage->GetIniSection());
}


void LogPageEx::OnNMRClickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	//DV("%s", __func__);

	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void LogPageEx::OnContextMenu(CWnd* pwnd, CPoint point)
{
	if (pwnd == &mListCtrl)
	{
		CMenu menu;
		menu.LoadMenu(IDR_LOG);
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	}
}

void LogPageEx::OnOpenFileGotoLine()
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

	string exe = StringTool::Format("%s/%s"
		, ShellTool::GetAppPath().c_str()
		,"OpenFileGotoLine.exe"
	);

	if (!File::FileExists(exe))
	{
		USES_CONVERSION;
		CString text = _T("no found OpenFileGotoLine.exe");
		ShowToast(text);
		return;
	}

	string cmd = StringTool::Format(
		"%s \"%s\" %d"
		,exe.c_str()
		,item->file.c_str()
		,item->line
	);

	auto ret=WinExec(cmd.c_str(), SW_SHOW);
	//DV("ret=%d,%s(%d)", ret,item->file.c_str(), item->line);

}


void LogPageEx::OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	OnOpenFileGotoLine();
}


LogItem* LogPageEx::GetCurrentLogItem()
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

void LogPageEx::OnCopyFullPath()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	//if (!File::FileExists(item->file))
	{
		//ShowToast(_T("file no found"));
		//return;
	}

	USES_CONVERSION;
	CString text = A2T(item->file.c_str());
	ShellTool::CopyTextToClipboard(GetSafeHwnd(), text);

	ShowToast(_T("Copy full path OK"));
}

void LogPageEx::OnOpenFolder()
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


void LogPageEx::OnAppDisable()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	mFilterPage->DisableApp(item->appName);
}

void LogPageEx::OnAppClear()
{
	mFilterPage->ClearApp();
}

void LogPageEx::OnTagDisable()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	mFilterPage->DisableTag(item->tag);
}

void LogPageEx::OnTagClear()
{
	mFilterPage->ClearTag();
}

void LogPageEx::OnLvnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	auto p = pNMLV;
	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVIS_SELECTED))
	{
		//TRACE("select changed###################################\r\n");
	}

	*pResult = 0;

	/*
	TRACE("iItem=%d,uNewState=%d,uOldState=%d,uChanged=0x%04x\r\n"
		,p->iItem
		,p->uNewState
		,p->uOldState
		,p->uChanged
	);
	*/
	//delay refresh to avoid too much unnecessary actions
	//SetTimer(eTimer_DelayRefreshItemPage, 200, nullptr);

	if (!mAddItemBusying)
	{
		auto sel = mListCtrl.GetNextItem(-1, LVNI_SELECTED);
		if (sel == -1)
		{
			return;
		}
		
		static int idx = -1;
		++idx;
		TRACE("Itemchanged[%04d].sel=%d\r\n",idx, sel);

		if (sel == -1)
		{
			int x = 0;
		}
	}
}

void LogPageEx::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case eTimer_DelayRefreshItemPage:
	{
		KillTimer(nIDEvent);
		{
			dump("in timer");
		}

		auto item = GetCurrentLogItem();
		mItemPage->SetLogItem(item);
		return;
	}
	case eTimer_DelayRelayout:
	{
		KillTimer(eTimer_DelayRelayout);
		CRect rc;
		GetClientRect(rc);
		OnRelayout(rc);
		return;
	}
	}

	__super::OnTimer(nIDEvent);
}


void LogPageEx::OnClear()
{
	OnClearLog();
	mItemPage->clear();
}

void LogPageEx::OnCopy()
{
	auto item = GetCurrentLogItem();
	if (item)
	{
		ShellTool::CopyTextToClipboard(GetSafeHwnd(), item->msg);
	}
}

void LogPageEx::OnCopyAll()
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

void LogPageEx::OnCodePageUtf8()
{
	int code = SC_CP_UTF8;
	mIni->SetInt(mSection, "codepage", code);

	auto edit = mItemPage->GetEdit();
	edit->SetCodePage(code);
}

void LogPageEx::OnCodePageChinese()
{
	//936 (Simplified Chinese GBK), 
	//解决app为unicode版时中文乱码问题

	int code = 936;
	mIni->SetInt(mSection, "codepage", code);

	auto edit = mItemPage->GetEdit();
	edit->SetCodePage(code);
}

void LogPageEx::OnUpdateCodePageUtf8(CCmdUI* pCmdUI)
{
	//pCmdUI->SetCheck(true);
}

void LogPageEx::OnUpdateCodePageChinese(CCmdUI* pCmdUI)
{

}

void LogPageEx::dump(string desc)
{
	auto& list = mListCtrl;
	const auto sel = list.GetNextItem(-1, LVNI_SELECTED);

	static int idx = -1;
	++idx;
	TRACE("%s,%s[%04d].sel=%04d,\r\n",desc.c_str(), __func__, idx, sel);
}


void LogPageEx::OnBnClickedAdd()
{
	static int idx = -1;
	++idx;
	//LogV(TAG, "item %04d", idx);
}


void LogPageEx::OnBnClickedAddItem()
{
	auto& list = mListCtrl;

	auto item = make_shared<LogItem>();

	auto nc = list.GetItemCount();
	//auto idx = list.InsertItem(nc, _T(""));

	item->msg = StringTool::Format("%04d", nc);
	//CString text;
	//text.Format(_T("%04d"), nc);
	//list.SetItemText(idx, 0, text);
	OnLogItemReady(item);
}

void LogPageEx::OnBnClickedSel0()
{
	Select(0);
}


void LogPageEx::OnBnClickedSel1()
{
	Select(1);
}


void LogPageEx::OnBnClickedSel2()
{
	Select(2);
}

void LogPageEx::Select(int idx)
{
	auto nc = mListCtrl.GetItemCount();
	if (idx >= 0 && idx < nc)
	{
		mListCtrl.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	}
}
