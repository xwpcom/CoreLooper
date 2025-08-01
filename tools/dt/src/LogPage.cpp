﻿#include "pch.h"
#include "dt.app.h"
#include "LogPage.h"
#include "afxdialogex.h"
#include "LogFilterPage.h"
#include "LogItemPage.h"
#include "logmanager.h"
#include "logwnd.h"
#include "core/string/utf8tool.h"

IMPLEMENT_DYNAMIC(LogPage, BasePage)
static const char* TAG = "LogPage";
enum
{
	eTimer_DelayRefreshItemPage,
	eTimer_Test,
	eTimer_delaySetItemCount,
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

	mSection = "LogPage";
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
#ifdef _CONFIG_VLIST
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, GetDispInfo)
#endif

	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &LogPage::OnNMRClickList)
	ON_COMMAND(ID_OPEN_FILE_GOTO_LINE, &LogPage::OnOpenFileGotoLine)
	ON_COMMAND(ID_CODEPAGE_UTF8, &LogPage::OnCodePageUtf8)
	ON_COMMAND(ID_CODEPAGE_CHINESE, &LogPage::OnCodePageChinese)
	//ON_UPDATE_COMMAND_UI_RANGE(ID_CODEPAGE_UTF8, ID_CODEPAGE_UTF8+100, &LogPage::OnCodePageChinese)
	ON_UPDATE_COMMAND_UI(ID_CODEPAGE_UTF8, &LogPage::OnUpdateCodePageUtf8)
	ON_UPDATE_COMMAND_UI(ID_CODEPAGE_CHINESE, &LogPage::OnUpdateCodePageChinese)
	
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
	ON_BN_CLICKED(IDC_ADD, &LogPage::OnBnClickedAdd)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, &LogPage::OnLvnKeydownList)
	ON_NOTIFY(NM_CLICK, IDC_LIST, &LogPage::OnNMClickList)
END_MESSAGE_MAP()

static COLORREF gColors[] =
{
	RGB(255, 0, 0),//error
	RGB(255, 0, 255),//warning
	RGB(0, 0, 255),//notice
	RGB(0, 0, 0),//trace
	RGB(128, 128, 128),//verbose
};

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
#ifdef _CONFIG_VLIST
			LogPage* mLogPage = nullptr;
#else
			CListCtrl* mList = nullptr;
#endif
			virtual ~LogListCtrlNodeProxy() {}

			virtual LPVOID GetItemData(int row)
			{
#ifdef _CONFIG_VLIST
				return mLogPage->mVirtualItems[row]->item.get();
#else
				return mList->GetItemData(row);
#endif
			}
			virtual COLORREF OnGetCellTextColor(DWORD_PTR context, int row, int col)
			{
				auto item = (LogItem*)context;
				if (!item)
				{
					return 0;
				}

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
				if (!item)
				{
					return 0;
				}
				static COLORREF clrBK = GetSysColor(COLOR_WINDOW);
				return clrBK;
			}
		};

		auto obj = make_shared<LogListCtrlNodeProxy>();
#ifdef _CONFIG_VLIST
		obj->mLogPage = this;
#else
		obj->mList = &mListCtrl;
#endif
		mListCtrl.SetNodeProxy(obj);
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

#ifdef _DEBUG
	SetTimer(eTimer_Test, 1000);
#endif

	{
		//qt在vs2022中编译时__FILE__是相对路径,为了能方便定位日志文件,需要将相对路径转换为绝对路径
		ByteBuffer box;
		File::ReadFile(ShellTool::GetAppPath() + "/dt.path.json", box);
		if (!box.empty())
		{
			DynamicJsonBuffer jBuf;
			auto& jItems = jBuf.parseArray(box.data());
			for (auto& jItem : jItems)
			{
				auto path = jItem.as<string>();
				//if (File::FileExists(path.c_str()))
				{
					mRootPaths.push_back(path);
					//LogV("dt", "rootPaths.add %s", path.c_str());
				}
			}
		}
	}

	return ret;
}

void LogPage::OnLogItemReady(shared_ptr<LogItem> item)
{
	mLogItems.push_back(item);

	if (mFilterPage->Filter(item) == 0)
	{
		AddItem(item);
	}
	
	/*
	2020.12.16
	为方便用户查看日志,最近有用户操作时，不删除过旧日志, 
	是为了避免日志达到上限后，用户在查看日志时，日志不断上翻，影响体验
	*/
	if (ShellTool::GetTickCount64() - mUserActionTick > 5 * 60 * 1000)
	{

		//int mMaxLogCount = 1;
		if ((int)mLogItems.size() > mMaxLogCount)
		{
			mLogItems.pop_front();
		}

		if ((int)mListCtrl.GetItemCount() > mMaxLogCount)
		{
#ifdef _CONFIG_VLIST
			mVirtualItems.erase(mVirtualItems.begin());
#else
			auto idx = 0;
			auto item = (LogItem*)mListCtrl.GetItemData(idx);
			mListCtrl.DeleteItem(idx);
			item->mRefs.pop_back();
#endif
		}
	}

#ifdef _CONFIG_VLIST
	SetTimer(eTimer_delaySetItemCount,100);
#endif
}

/*
* 发现条数较多时,在AddItem内部每次都调用SetItemCountEx会比较慢，所以由调用方负责主动调用SetItemCountEx
*/

void LogPage::AddItem(shared_ptr<LogItem>& item)
{
	auto& list = mListCtrl;

#ifdef _CONFIG_VLIST
	{
		auto obj = make_shared<VirtualItem>();
		obj->item = item;
		mVirtualItems.push_back(obj);
	}

#else
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
	item->mRefs.push_back(item);//从list删除时要释放 */
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
		list.SetItemState(nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		list.EnsureVisible(nIndex, FALSE);
		dump("after set select");
	}
#endif

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

	int headerHeight = 0;
	/*
	{
		CRect rcItem;
		GetDlgItem(IDC_ADD)->GetWindowRect(rcItem);
		ScreenToClient(rcItem);
		headerHeight = rcItem.Height();
	}
	*/

	CRect rcItem = rc;
	rcItem.OffsetRect(0, headerHeight);
	rcItem.bottom = rcItem.top + 56;
	mFilterPage->MoveWindow(rcItem);

	int itemHeight = rc.Height() / 5;
	itemHeight = MAX(itemHeight, 240);

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

void LogPage::OnClearLog()
{
	ClearLogListCtrl();
	mLogItems.clear();
}

void LogPage::ClearLogListCtrl()
{
#ifdef _CONFIG_VLIST
	mVirtualItems.clear();
	mListCtrl.SetItemCountEx((int)mVirtualItems.size());
	mListCtrl.Invalidate();
#else
	for(int i=0;i<(int)mListCtrl.GetItemCount();++i)
	{
		auto item = (LogItem*)mListCtrl.GetItemData(i);
		item->mRefs.pop_back();
	}

	mListCtrl.DeleteAllItems();
#endif
}

void LogPage::OnFilterChanged()
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

	int count = (int)mVirtualItems.size();
	mListCtrl.SetItemCountEx(count);

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
	//DV("%s", __func__);

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

	auto filePath = item->file;
	tryConvertToFullPath(filePath);

	if (!File::FileExists(filePath))
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
		,filePath.c_str()
		,item->line
	);

	auto ret=WinExec(cmd.c_str(), SW_SHOW);
	//DV("ret=%d,%s(%d)", ret,item->file.c_str(), item->line);

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

#ifdef _CONFIG_VLIST
	return mVirtualItems[sel]->item.get();
#else
	auto item = (LogItem*)list.GetItemData(sel);
	return item;
#endif
}

void LogPage::OnCopyFullPath()
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

	auto filePath = item->file;
	tryConvertToFullPath(filePath);

	USES_CONVERSION;
	CString text = A2T(filePath.c_str());
	ShellTool::CopyTextToClipboard(GetSafeHwnd(), text);

	text.Format(_T("Copy full path OK,line = %d"), item->line);
	ShowToast(text);
}

void LogPage::OnOpenFolder()
{
	auto item = GetCurrentLogItem();
	if (!item)
	{
		return;
	}

	auto filePath = item->file;
	tryConvertToFullPath(filePath);

	if (!File::FileExists(filePath))
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

	int sel = mListCtrl.GetNextItem(-1, LVNI_SELECTED);
	if (sel != -1)
	{
		mListSelectIndex = sel;
	}

	KillTimer(eTimer_DelayRefreshItemPage);
	//delay refresh to avoid too much unnecessary actions
	SetTimer(eTimer_DelayRefreshItemPage, 10, nullptr);
}

void LogPage::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case eTimer_delaySetItemCount:
	{
		KillTimer(nIDEvent);
#ifdef _CONFIG_VLIST
		/*
		* 发现CListCtrl virtual list的SetItemCountEx很慢，凡是可能频繁调用到SetItemCountEx的地方都需要优化
		*/

		auto count = (int)mVirtualItems.size();
		mListCtrl.SetItemCountEx(count);

		if (mFilterPage->IsAutoScrollEnabled() && count>0)
		{
			auto index = count - 1;
			auto& list = mListCtrl;
			list.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			list.EnsureVisible(index, FALSE);
		}
#endif
		return;
	}
	case eTimer_Test:
	{
		static int idx = -1;

		auto tick = ShellTool::GetTickCount64();
		for (int i = 0; i < 1; i++)
		{
			++idx;

			LogV(TAG, "idx=%04d", idx);
			LogW(TAG, u8"idx=%04d 中文", idx);
		}
		tick = ShellTool::GetTickCount64()-tick;
		//TRACE("tick=%lld **********************\r\n", tick);
		//PostQuitMessage(0);
		//KillTimer(eTimer_Test);
		return;
	}
	case eTimer_DelayRefreshItemPage:
	{
		int sel = mListCtrl.GetNextItem(-1, LVNI_SELECTED);
		bool invalid = (mListSelectIndex >= mListCtrl.GetItemCount());
		if (sel!=-1 && (invalid ||  mListSelectIndex == sel))
		{
			KillTimer(nIDEvent);

			static int idx = -1;
			++idx;
			TRACE("SetLogItem[%04d].sel=%d\r\n",idx, sel);

#ifdef _CONFIG_VLIST
			auto item = mVirtualItems[sel]->item.get();
#else
			auto item = (LogItem*)mListCtrl.GetItemData(sel);
#endif
			//auto item = GetCurrentLogItem();
			mItemPage->SetLogItem(item);
		}

		return;
	}
	}

	__super::OnTimer(nIDEvent);
}


void LogPage::OnClear()
{
	OnClearLog();
	mItemPage->clear();
	mFilterPage->OnClearList();
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
#ifdef _CONFIG_VLIST
	char level[] = 
	{
		'E',
		'W',
		'I',
		'D',
		'V',
	};

	USES_CONVERSION;
	for (auto& itemA : mVirtualItems)
	{
		itemA->makeReady();
		auto& item = itemA->item;

		StringTool::AppendFormat(text,"%s[%c][%s]%s\r\n"
			, itemA->time.c_str()
			, level[item->level]
			,item->tag.c_str()
			,item->msg.c_str()
			);
	}
#else
	auto& list = mListCtrl;
	int count = list.GetItemCount();
	for (int i = 0; i < count; i++)
	{
		auto item = (LogItem*)list.GetItemData(i);

		text += item->msg + "\r\n";
	}
#endif

	ShellTool::CopyTextToClipboard(GetSafeHwnd(), text);
}

void LogPage::OnCodePageUtf8()
{
	int code = SC_CP_UTF8;
	mIni->SetInt(mSection, "codepage", code);

	auto edit = mItemPage->GetEdit();
	edit->SetCodePage(code);
}

void LogPage::OnCodePageChinese()
{
	//936 (Simplified Chinese GBK), 
	//解决app为unicode版时中文乱码问题

	int code = 936;
	mIni->SetInt(mSection, "codepage", code);

	auto edit = mItemPage->GetEdit();
	edit->SetCodePage(code);
}

void LogPage::OnUpdateCodePageUtf8(CCmdUI* pCmdUI)
{
	//pCmdUI->SetCheck(true);
}

void LogPage::OnUpdateCodePageChinese(CCmdUI* pCmdUI)
{

}

void LogPage::dump(string desc)
{
	auto& list = mListCtrl;
	const auto sel = list.GetNextItem(-1, LVNI_SELECTED);

	static int idx = -1;
	++idx;
	//TRACE("%s,%s[%04d].sel=%04d,\r\n",desc.c_str(), __func__, idx, sel);
}


void LogPage::OnBnClickedAdd()
{
	static int idx = -1;
	++idx;
	//LogV(TAG, "item %04d", idx);
}

#ifdef _CONFIG_VLIST
void LogPage::VirtualItem::makeReady()
{
	if (virtualReady)
	{
		return;
	}

	virtualReady = true;

	USES_CONVERSION;
	{
		auto data = item->msg;

		StringTool::Replace(data, "\r", "^");
		StringTool::Replace(data, "\n", "~");
		StringTool::Replace(data, "\t", "`");

		msg = data;// Utf8Tool::UTF8_to_UNICODE(data.c_str(), data.length());
	}
	{
		string time;
		//仅在hour为0或23时添加date,否则只显示time,hhmmssMMM
		auto hour = item->time / 10000000;
		if (hour == 0 || hour == 23)
		{
			//date:yymmdd
			auto m = (item->date / 100) % 100;
			auto d = item->date % 100;
			time = StringTool::Format("%02d.%02d ", m, d);
		}

		{
			auto minute = (item->time / 100000) % 100;
			auto second = (item->time / 1000) % 100;
			auto ms = item->time % 1000;
			StringTool::AppendFormat(time, "%02d:%02d:%02d.%03d", hour, minute, second, ms);
		}

		this->time = time;// A2T(time.c_str());
	}
	{
		auto text = StringTool::Format("%s(%d)", item->file.c_str(), item->line);
		fileLine = text;// A2T(text.c_str());
	}
	
	{
		auto text = StringTool::Format("%d/%d", item->pid, item->tid);
		pid = text;// A2T(text.c_str());
	}
	
	tag = item->tag;// A2T(item->tag.c_str());
	app = item->appName;// A2T(item->appName.c_str());
}

void LogPage::GetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM* pItem = &(pDispInfo)->item;
	auto item = mVirtualItems[pItem->iItem];
	if (!item->virtualReady)
	{
		item->makeReady();
	}

	auto& dst = pItem->pszText;
	//LPSTR_TEXTCALLBACK;
	if (item && pItem->mask & LVIF_TEXT) //valid text buffer?
	{
		auto& text = item->CStringCache;
		const auto col = pItem->iSubItem;
		USES_CONVERSION;
		if (col == mArrIdx[eIdxMsg])
		{
			text = Utf8Tool::UTF8_to_UNICODE(item->msg.c_str(), item->msg.length());
			//dst = (LPWSTR)(LPCTSTR)item->msg;
		}
		else if (col == mArrIdx[eIdxTime])
		{
			text = A2T(item->time.c_str());
			//dst = (LPWSTR)(LPCTSTR)item->time;
		}
		else if (col == mArrIdx[eIdxFileLine])
		{
			text = A2T(item->fileLine.c_str());
			//dst = (LPWSTR)(LPCTSTR)item->fileLine;
		}
		else if (col == mArrIdx[eIdxPid])
		{
			text = A2T(item->pid.c_str());
			//dst = (LPWSTR)(LPCTSTR)item->pid;
		}
		else if (col == mArrIdx[eIdxTag])
		{
			text = A2T(item->tag.c_str());
			//dst = (LPWSTR)(LPCTSTR)item->tag;
		}
		else if (col == mArrIdx[eIdxApp])
		{
			text = A2T(item->app.c_str());
			//dst = (LPWSTR)(LPCTSTR)item->app;
		}
		else
		{
			ASSERT(FALSE);
		}

		dst = (LPWSTR)(LPCTSTR)item->CStringCache;

	}

	*pResult = 0;
	//wcscpy(dst, rLabel.m_szText[0]);
}
#endif


void LogPage::OnLvnKeydownList(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LogV(TAG, "%s", __func__);

	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	*pResult = 0;

	UpdateUserActionTick();
}

/*
* 有用户动作时，在一定的时间段内不删除过旧的日志
*/
void LogPage::UpdateUserActionTick()
{
	mUserActionTick = ShellTool::GetTickCount64();
	//LogV(TAG, "%s", __func__);
	mFilterPage->OnUserAction(mUserActionTick);
}

void LogPage::OnNMClickList(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LogV(TAG, "%s", __func__);

	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	*pResult = 0;

	UpdateUserActionTick();
}

int LogPage::tryConvertToFullPath(string& filePath)
{
	if (filePath.length() > 2 && filePath.at(1) != ':')
	{
		//可能是相对路径
		for (auto& root : mRootPaths)
		{
			auto fullPath = root + "/" + filePath;
			if (File::FileExists(fullPath.c_str()))
			{
				filePath = fullPath;

				File::PathMakePretty(filePath);
				StringTool::Replace(filePath, "/", "\\");
				return 0;
			}
		}
	}

	return 0;
}
