#include "stdafx.h"
#include "DebugHelper.h"

#include "DebugHelperDoc.h"
#include "DebugHelperView.h"
#include <Shlwapi.h>

#include "OptionDlg.h"
#include <process.h>
#include "mainfrm.h"

#pragma comment(lib,"Shlwapi.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_SOCKET_MSG	(WM_USER+2)

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperView
//定时保存DebugHelper的输出到DebugHelper.txt
#define IDT_DETECTSAVE	0x3

IMPLEMENT_DYNCREATE(CDebugHelperView, CListView)
static UINT s_uIDThis = 0;
BEGIN_MESSAGE_MAP(CDebugHelperView, CListView)
	//{{AFX_MSG_MAP(CDebugHelperView)
	ON_COMMAND(ID_EDIT_CLEARALL, OnEditClearAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEARALL, OnUpdateEditClearAll)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_LOG, OnLog)
	ON_UPDATE_COMMAND_UI(ID_LOG, OnUpdateLog)
	ON_COMMAND(IDH_TEST, OnTest)
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_OPTION, OnOption)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CListView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CListView::OnFilePrintPreview)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomdrawList)
	ON_MESSAGE(WM_SOCKET_MSG,OnSocketMsg)
	//ON_NOTIFY(NM_CUSTOMDRAW, s_uIDThis, OnCustomdrawList)
END_MESSAGE_MAP()

CDebugHelperView::CDebugHelperView()
{
	m_dwLine = AfxGetApp()->GetProfileInt("Settings","dwLastLine",0);
	m_pViewTraceDlg = NULL;
	m_nTimeSave = 60;
	m_bLog = AfxGetApp()->GetProfileInt("Settings","bLog",0);

	m_hThread = NULL;
	memset(mArrIdx, 0, sizeof(mArrIdx));

}

CDebugHelperView::~CDebugHelperView()
{
	if(m_pViewTraceDlg)
	{
		delete m_pViewTraceDlg;
		m_pViewTraceDlg = NULL;
	}
	AfxGetApp()->WriteProfileInt("Settings","dwLastLine",m_dwLine);
	AfxGetApp()->WriteProfileInt("Settings","bLog",m_bLog);
}

BOOL CDebugHelperView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL bPreCreated = CListView::PreCreateWindow(cs);
	//cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping
	cs.style |= LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS;
	return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperView drawing

void CDebugHelperView::OnDraw(CDC* pDC)
{
	CDebugHelperDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperView printing

BOOL CDebugHelperView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default CListView preparation
	return CListView::OnPreparePrinting(pInfo);
}

void CDebugHelperView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CListView begin printing.
	CListView::OnBeginPrinting(pDC, pInfo);
}

void CDebugHelperView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Default CListView end printing
	CListView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperView diagnostics

#ifdef _DEBUG
void CDebugHelperView::AssertValid() const
{
	CListView::AssertValid();
}

void CDebugHelperView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CDebugHelperDoc* CDebugHelperView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDebugHelperDoc)));
	return (CDebugHelperDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDebugHelperView message handlers
HWND g_hwndListView=NULL;

void CDebugHelperView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();

	g_hwndListView = GetListCtrl().m_hWnd;

	m_font.DeleteObject();
	m_font.CreatePointFont(120,"新宋体");
	
	CListCtrl &lstCtrl = GetListCtrl();
	lstCtrl.DeleteAllItems();
	
	lstCtrl.SetFont(&m_font);	
	lstCtrl.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT
		| LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP);// | LVS_EX_LABELTIP );
	//lstCtrl.ModifyStyle(LVS_SINGLESEL,NULL);
	

	int idx = -1;
	mArrIdx[eIdxMsg] = lstCtrl.InsertColumn(++idx, "Data");
	mArrIdx[eIdxTime] = lstCtrl.InsertColumn(++idx, "Time");
	mArrIdx[eIdxPid] = lstCtrl.InsertColumn(++idx, "Pid/Tid");
	mArrIdx[eIdxFileLine] = lstCtrl.InsertColumn(++idx, "Line");

	VERIFY(SetTimer(IDT_DETECTSAVE,2000,NULL) == IDT_DETECTSAVE);

	unsigned char * pData = NULL;
	UINT nRead = 0;
	AfxGetApp()->GetProfileBinary("Settings","lstCtrlColumnOrderArray",
		&pData,&nRead);

	const int columnCount = lstCtrl.GetHeaderCtrl()->GetItemCount();

	if (sizeof(int)*columnCount == nRead)
	{
		lstCtrl.SetColumnOrderArray(columnCount,(int*)pData);
	}
	delete pData;

	//save column width
	CString szKey;
	for(int i=0;i<columnCount;i++)
	{
		szKey.Format("ColumnWidth%d",i);
		int nWidth = AfxGetApp()->GetProfileInt("Settings",szKey,300);
		if (nWidth < 10)
		{
			nWidth = 10;
		}
		lstCtrl.SetColumnWidth(i,nWidth);
	}

	s_uIDThis = GetDlgCtrlID();

	m_bLog = AfxGetApp()->GetProfileInt("Settings","log",FALSE);

	if(!m_wndMsgSink.GetSafeHwnd())
	{
		m_wndMsgSink.Create("DebugHelper ");
		m_wndMsgSink.SetMsgReceiver(AfxGetMainWnd());
	}

	m_nMaxLine = AfxGetApp()->GetProfileInt("Settings","nMaxLine",0);

	ASSERT(!m_hThread);
	UINT uThreadId=0;
	m_hThread = (HANDLE)_beginthreadex(NULL,0,_RecvThreadCB,this,0,&uThreadId);
}

UINT CDebugHelperView::_RecvThreadCB(void *pThis)
{
	CDebugHelperView *p = (CDebugHelperView*)pThis;
	return p->_RecvThread();
}

#include "SockTools.h"
UINT CDebugHelperView::_RecvThread()
{
	CSockTools::InitWSASocket();

	SOCKET s = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	int so_reuseaddr = TRUE;
	int ret = setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&so_reuseaddr,sizeof (so_reuseaddr));
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons (9004);
	sa.sin_addr.s_addr = INADDR_ANY;//inet_addr("192.168.1.48");

	ret = bind(s,(struct sockaddr *)&sa,sizeof(sa));
	if(ret != 0)
	{
		return 0;
	}

	
	char szBuf[32*1024];// = new char [cbBuf];
	int cbBuf = 32*1024;
	while(TRUE)
	{
		struct sockaddr_in sa={0};
		int addrLen = sizeof(sa);
		memset(szBuf,0,cbBuf);

		//DT("等待设备响应广播");
		int ret = recvfrom(s,    /*socket */
			szBuf,    /* receiving buffer */
			cbBuf,    /* max rcv buf size */
			0,    /* flags:no options */
			(struct sockaddr *)&sa,    /* addr */
			&addrLen);    /* addr len */

		if(ret > 0)
		{
			szBuf[ret]=0;
			//DT("recvfrom,len=%d,szBuf=[%s]\n",ret,szBuf);

			/*
			int cbSize = ret + 1;
			char *pszAck = new char[cbSize];
			memcpy(pszAck,szBuf,ret);
			pszAck[ret]=0;

			DT("%s",inet_ntoa(sa.sin_addr));
			//*/

			//DT("send WM_SEARCH_IPCAMERA");
			//如果用SendMessage,而主线程弹出出错或者其它DoModal对话框时,
			//本工作线程也会阻塞住,会导致设备无故超时消失
			//所以改用PostMessage
			//缺点是要用new szBuf.由主线程释放
			//char *pszIP = new char[16];
			//strcpy(pszIP,inet_ntoa(sa.sin_addr));
			SendMessage(WM_SOCKET_MSG,0,(LPARAM)szBuf);
		}
		else
		{
			break;
		}
	}
	
	return 0;
}

LRESULT CDebugHelperView::OnSocketMsg(WPARAM wp, LPARAM lp)
{
	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
	if(pFrame->IsRecvSocketMsg())
	{
		AddString((const char*)lp);
	}

	return 0;
}


void CDebugHelperView::AddString(CString szMsg)
{
	CListCtrl& lstCtrl = GetListCtrl();
	{
		//avoid too many output string
		int nc = GetListCtrl().GetItemCount();
		if(m_nMaxLine != 0 && (UINT)nc >= m_nMaxLine)
		{
			SaveToFile(FALSE);
			OnEditClearAll();
		}
	}
	
	CString szLine;
	char *lpszSplit="$$@@";//定位的分隔符
	int nDTLevel=0;
	CString szPid;

	tagMsgEx *pMsgEx = new tagMsgEx;

	if(szMsg.Find(lpszSplit) == 0)
	{
		int nLenSplit = strlen(lpszSplit);
		int nFileEnd = szMsg.Find(lpszSplit,nLenSplit);
		if(nFileEnd != -1)
		{
			szLine = szMsg.Mid(nLenSplit,nFileEnd - nLenSplit);
			
			nDTLevel = atoi(((LPCTSTR)szMsg)+nFileEnd+nLenSplit);
			
			int nMsgStart = szMsg.Find(lpszSplit,nFileEnd+1);
			int pos = szMsg.Find(lpszSplit, nMsgStart+1);
			if (pos != -1)
			{

				CString sz = (const char*)szMsg + pos + strlen(lpszSplit);
				szPid = sz;
				szMsg = szMsg.Mid(nMsgStart+strlen(lpszSplit), pos - nMsgStart-strlen(lpszSplit));
			}
			else
			{
				szMsg = szMsg.Right(szMsg.GetLength() - nMsgStart - nLenSplit);
			}
			pMsgEx->msg=szMsg;
			szMsg.Replace('\r','^');
			szMsg.Replace('\n','~');
			szMsg.Replace('\t','`');
		}
	}
	
	//szLine.Format("% 8d",++m_dwLine);

	CString szTm;
	if (1)
	{
		SYSTEMTIME sysTime;
		::GetLocalTime(&sysTime);
		char buf[256];
		_snprintf(buf,sizeof(buf)-1, "%02d-%02d %02d:%02d:%02d.%03d",
			sysTime.wMonth,sysTime.wDay, 
			sysTime.wHour, sysTime.wMinute,sysTime.wSecond, sysTime.wMilliseconds);
		szTm = buf;
	}
	else
	{
		CTime tm = CTime::GetCurrentTime();
		DWORD tick = GetTickCount() % 1000;
		szTm.Format("%s.%03d", tm.Format("%m-%d %H:%M:%S"), tick);
	}

	int nIndex = lstCtrl.GetItemCount();
	lstCtrl.InsertItem(nIndex,"");	 
	lstCtrl.SetItemText(nIndex, mArrIdx[eIdxMsg], szMsg);
	lstCtrl.SetItemText(nIndex, mArrIdx[eIdxTime], szTm);
	lstCtrl.SetItemText(nIndex, mArrIdx[eIdxPid], szPid);
	lstCtrl.SetItemText(nIndex, mArrIdx[eIdxFileLine], szLine);
	pMsgEx->level=nDTLevel;
	lstCtrl.SetItemData(nIndex,(DWORD)pMsgEx);
	
	//仅当选中最后一项时才更新选中Item
	if(lstCtrl.GetNextItem(-1,LVNI_SELECTED) == nIndex -1)
	{
		lstCtrl.SetItemState(nIndex,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
		lstCtrl.EnsureVisible(nIndex,FALSE);
	}
	//加入注册表
	if(m_bLog)
	{
		//CString szDate;
		//szDate.Format("%02d月%02d日",tm.GetMonth(),tm.GetDay());
		//AfxGetApp()->WriteProfileString("Settings","lastLine",szMsg);
	}
}

void CDebugHelperView::OnEditClearAll() 
{
	CListCtrl &lstCtrl = GetListCtrl();
	{
		int nc = lstCtrl.GetItemCount();
		for(int i=0;i<nc;i++)
		{
			tagMsgEx *pMsgEx = (tagMsgEx *)lstCtrl.GetItemData(i);
			if(pMsgEx)
			{
				delete pMsgEx;
				pMsgEx=NULL;
			}
		}
	}
	lstCtrl.DeleteAllItems();
}

void CDebugHelperView::OnUpdateEditClearAll(CCmdUI* pCmdUI) 
{
}

void CDebugHelperView::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == IDT_DETECTSAVE)
	{
		CTime tm = CTime::GetCurrentTime();
		CString szTime = tm.Format("%y-%m-%d %H:%M:%S");
		//AfxGetApp()->WriteProfileString("Settings","szLastRunTime",szTime);
		//If you want a instant save ,create "DT.tmp" in exe's direct,
		if(PathFileExists("DT.tmp"))
		{
			DeleteFile("DT.tmp");
			SaveToFile();
		}
		// Notice: CListView::OnTimer(nIDEvent) will kill timer, so we return here
		// directly,but why?
		return;
	}

	CListView::OnTimer(nIDEvent);
}

void CDebugHelperView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CListCtrl &lstCtrl = GetListCtrl();
	int nSel = lstCtrl.GetNextItem(-1,LVNI_SELECTED);

	if(!m_pViewTraceDlg)
	{
		m_pViewTraceDlg = new CViewTraceDlg(this);
		m_pViewTraceDlg->Create(IDD_VIEW_DLG,this);
	}
	m_pViewTraceDlg->SetIndex(nSel);
	m_pViewTraceDlg->ShowWindow(SW_SHOW);
	
	CListView::OnLButtonDblClk(nFlags, point);
}

BOOL CDebugHelperView::SaveToFile(BOOL bShowMsgBox)
{
	char szBuf[MAX_PATH];
	::GetModuleFileName(NULL,szBuf,MAX_PATH);
	CTime time = CTime::GetCurrentTime();
	CString szTime = time.Format("%Y.%m.%d_%H_%M_%S");
	CString szFile(szBuf);
	szFile = szFile.Left(szFile.ReverseFind('\\'));
	szFile += "\\DT.dump\\";
	CreateDirectory(szFile,NULL);
	szFile += szTime+".txt";
	CStdioFile file;
	if(file.Open(szFile,CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		CString szText;
		CListCtrl& lstCtrl = GetListCtrl();
		for(int i=0;i<lstCtrl.GetItemCount();i++)
		{
			
			/*
			szText = lstCtrl.GetItemText(i,0) + "  ";
			szText += lstCtrl.GetItemText(i,1) + "  ";
			szText += lstCtrl.GetItemText(i,2)+ "\n";
			file.WriteString(szText);
			//*/
			
			szText = lstCtrl.GetItemText(i,1) + "  ";
			file.WriteString(szText);

			szText = lstCtrl.GetItemText(i,0);
			file.WriteString(szText);

			file.WriteString("\n");

		}
		file.Close();

		if(bShowMsgBox)
		{
			//AfxMessageBox("Save log file to :"+szFile);
			ShellExecute(NULL, _T("open"), _T("Explorer.exe"), _T(" /select,") + szFile, NULL, SW_SHOWDEFAULT);	
		}
	}
	else
	{
		if(bShowMsgBox)
		{
			AfxMessageBox("Save fail!");
		}
	}
	
	return TRUE;
}

void CDebugHelperView::OnFileSave() 
{
	SaveToFile();	
}



void CDebugHelperView::OnLog() 
{
	m_bLog = !m_bLog;	
}

void CDebugHelperView::OnUpdateLog(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bLog);	
}

void CDebugHelperView::OnTest() 
{
	this->GetListCtrl().SetColumnWidth(0,100);	
	int i = this->GetListCtrl().GetItemCount();
}

void CDebugHelperView::OnDestroy() 
{
	//save column layout,and resotre the next time when app startup
	CListCtrl &lstCtrl = GetListCtrl();
	const int columnCount = lstCtrl.GetHeaderCtrl()->GetItemCount();
	int nArr[100];
	lstCtrl.GetColumnOrderArray(nArr, columnCount);

	AfxGetApp()->WriteProfileBinary("Settings","lstCtrlColumnOrderArray",
		(LPBYTE)nArr,sizeof(nArr[0])*columnCount);

	//save column width
	CString szKey;
	for(int i=0;i<columnCount;i++)
	{
		szKey.Format("ColumnWidth%d",i);
		AfxGetApp()->WriteProfileInt("Settings",szKey,lstCtrl.GetColumnWidth(i));
	}

	//
	//AfxGetApp()->WriteProfileInt("Settings","log",m_bLog);

	CListView::OnDestroy();
}

void CDebugHelperView::OnEditCopy() 
{
	CString szLine;
	{
		CListCtrl &lstCtrl = GetListCtrl();		
		POSITION pos = lstCtrl.GetFirstSelectedItemPosition();
		int i=0;
		while(pos)
		{
			if(i++>0)
				szLine +="\n";
			int nIndex = lstCtrl.GetNextSelectedItem(pos);
			szLine += lstCtrl.GetItemText(nIndex,0);
		}
	}
	if(szLine.IsEmpty())
		return;
	
	CString source(szLine); 
	//put your text in source
	if(OpenClipboard())
	{
		HGLOBAL clipbuffer=NULL;
		char * buffer=NULL;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, source.GetLength()+1);
		buffer = (char*)GlobalLock(clipbuffer);
		strcpy(buffer, LPCSTR(source));
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT,clipbuffer);
		CloseClipboard();
	}
}

void CDebugHelperView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	CListCtrl &lstCtrl = GetListCtrl();		
	POSITION pos = lstCtrl.GetFirstSelectedItemPosition();
	pCmdUI->Enable(pos!=NULL);	
}

#include <windowsx.h>
int test(LPARAM lParam)
{
			LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;
		
		/*
		CDDS_PREPAINT is sent when the control is about to paint itself. You 
		implement custom draw by returning the proper value to this 
		notification.
		*/
		if(lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
		{
			//tell the control we want pre-paint notifications for each item
			return CDRF_NOTIFYITEMDRAW;
			//tell the control that we won't be doing any custom drawing
			return CDRF_DODEFAULT;
		}
		
		/*
		CDDS_ITEMPREPAINT is sent when the control is about to paint an item. 
		You will only get these if you returned CDRF_NOTIFYITEMDRAW in 
		response to CDDS_PREPAINT.
		*/
		if(lplvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
		{
			LRESULT  lReturn = CDRF_DODEFAULT;
			/*
			For the ListView, the index of the item being drawn is stored in the 
			dwItemSpec member of the NMCUSTOMDRAW structure. In this example, only 
			the odd items will be drawn using the bold font.
			*/
			if(lplvcd->nmcd.dwItemSpec & 0x01)
			{
				HFONT    hFont;
				LOGFONT  lf;
				
				//get the existing font
				hFont = (HFONT)SendMessage(g_hwndListView, WM_GETFONT, 0, 0);
				
				//now get the font's information
				GetObject(hFont, sizeof(lf), &lf);
				
				//make this font bold
				lf.lfWeight = FW_BOLD;
				
				//create the new font
				hFont = CreateFontIndirect(&lf);
				
				/*
				To change the font, just select the desired font into the HDC 
				provided.
				*/
				SelectObject(lplvcd->nmcd.hdc, hFont);
				
				/*
				To change the text and background colors in a ListView, set the 
				clrText and clrTextBk members of the NMLVCUSTOMDRAW structure to 
				the desired color. This is different than most other controls that 
				support custom draw. To change the text and background colors in 
				the others, you just call SetTextColor and SetBkColor on the HDC 
				provided. 
				*/
				lplvcd->clrText = RGB(255, 0, 0);
				lplvcd->clrTextBk = RGB(255, 255, 255);
				
				/*
				If you change the font, return CDRF_NEWFONT so the control can 
				recalculate the extent of the text. Returning CDRF_NOTIFYPOSTPAINT 
				causes the control to send us notifications with CDDS_ITEMPOSTPAINT.
				*/
				lReturn = CDRF_NEWFONT | CDRF_NOTIFYPOSTPAINT;
			}
			else
			{
				lplvcd->clrText = RGB(0, 0, 255);
				lplvcd->clrTextBk = RGB(255, 255, 255);
			}
			
			return lReturn;
		}
		
		if(lplvcd->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
		{
			HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			
			//clean up stuff here
			hFont = (HFONT)SelectObject(lplvcd->nmcd.hdc, hFont);
			
			DeleteFont(hFont);
			
			return CDRF_DODEFAULT;
		}
	return CDRF_DODEFAULT;
}

//错误级别
enum eDTLevel
{
	DT_MIN		=0,
	DT_DISABLE	=0,
	DT_FATAL,
	DT_ERROR,
	DT_WARNING,
	DT_NOTICE,
	DT_TRACE,
	DT_VERBOSE,
	DT_MAX,
};

void CDebugHelperView::OnCustomdrawList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	if (lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_DODEFAULT| CDRF_NOTIFYITEMDRAW| CDRF_NEWFONT;
	}
    else if (lplvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
    else if (lplvcd->nmcd.dwDrawStage & CDDS_ITEMPREPAINT)
    {
		CListCtrl &lstCtrl = GetListCtrl();
		int nDTLevel = 0;
		tagMsgEx *pMsgEx = (tagMsgEx *)lstCtrl.GetItemData(lplvcd->nmcd.dwItemSpec);
		if(pMsgEx)
		{
			nDTLevel = pMsgEx->level;
		}
		if(nDTLevel !=0)
		{
			if(nDTLevel == DT_WARNING)
				lplvcd->clrText = 	RGB(255,0,255);
			else if(nDTLevel == DT_FATAL)
			{
				lplvcd->clrText		= RGB(255,255,0);
				lplvcd->clrTextBk	= RGB(255,0,0);
			}
			else if(nDTLevel == DT_ERROR)
			{
				lplvcd->clrText		= RGB(255,0,0);
			}
			else if(nDTLevel == DT_VERBOSE)
				lplvcd->clrText = 	RGB(128,128,128);
			else if(nDTLevel == DT_NOTICE)
			{
				lplvcd->clrText = 	RGB(0,0,255);
			}

			*pResult = CDRF_DODEFAULT| CDRF_NEWFONT;
		}
		else
			*pResult = CDRF_DODEFAULT;
	}
	return;	
	
}

void CDebugHelperView::OnOption() 
{
	COptionDlg dlg;
	if(dlg.DoModal() == IDOK)
	{
		m_nMaxLine = AfxGetApp()->GetProfileInt("Settings","nMaxLine",0);
	}
}

