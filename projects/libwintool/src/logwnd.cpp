#include "stdafx.h"
#include "../include/logwnd.h"
#include "Scintilla.h"
//#include "SciLexer.h"
#include "core/string/utf8tool.h"


IMPLEMENT_DYNAMIC(LogWnd, ScintillaWnd)

BEGIN_MESSAGE_MAP(LogWnd,ScintillaWnd)
	ON_WM_CREATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

enum
{
	eTimerTest,
	eTimerDump,
};

using namespace Bear::Core;

static const char* TAG = "logWnd";

LogWnd::LogWnd()
{

}

int LogWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;
	SendMessage(SCI_SETREADONLY, 1);
	if (0)
	{
		//set font fore/back color,test ok
		SendMessage(SCI_STYLESETBACK, STYLE_DEFAULT, RGB(0x00, 0x00, 0x00));
		SendMessage(SCI_STYLESETFORE, STYLE_DEFAULT, RGB(0x00, 0xFF, 0x00));
		SendMessage(SCI_STYLECLEARALL);
	}

	//SendMessage(SCI_SETCARETLINEVISIBLEALWAYS, true);

	{
		//SendMessage(SCI_STYLESETBACK, STYLE_LINENUMBER, RGB(0x00, 0x00, 0x00));
		SendMessage(SCI_STYLESETFORE, STYLE_LINENUMBER
			, RGB(0xab, 0xab, 0xab)
			//, RGB(122, 217, 255)
		);
	}

	SendMessage(SCI_SETFONTQUALITY, SC_EFF_QUALITY_LCD_OPTIMIZED);

	//SendMessage(SCI_SETCARETLINEVISIBLEALWAYS, true);
	/*
	https://www.scintilla.org/ScintillaDoc.html#SCI_SETCODEPAGE
	Code page can be set to 65001 (UTF-8), 
	932 (Japanese Shift-JIS), 
	936 (Simplified Chinese GBK), 
	949 (Korean Unified Hangul Code), 
	950 (Traditional Chinese Big5),
	1361 (Korean Johab).
	*/
	//SendMessage(SCI_SETCODEPAGE,SC_CP_UTF8);
	SendMessage(SCI_SETCODEPAGE,936);//解决app为unicode版时中文乱码问题

	/*
	SendMessage(SCI_STYLESETCHARACTERSET, SCE_C_DEFAULT, SC_CHARSET_GB2312);
	SendMessage(SCI_STYLESETCHARACTERSET, SCE_C_STRING, SC_CHARSET_GB2312);
	SendMessage(SCI_STYLESETCHARACTERSET, SCE_C_USERLITERAL, SC_CHARSET_GB2312);
	*/
#ifdef _DEBUG
	//SetTimer(eTimerTest, 1*10, nullptr);
#endif
	/*
	{
		SendMessage(SCI_SETREADONLY, 0);
		string text;
		for (int i = 0; i < 1000; i++)
		{
			text = StringTool::Format("line %04d\r\n", i);
			SendMessage(SCI_ADDTEXT, text.length(), (LPARAM)text.c_str());

			//wp is length to add
			//SendMessage(SCI_ADDTEXT, 4, (LPARAM)"Hilo,world!");
			//SendMessage(SCI_ADDSTYLEDTEXT, 10, (LPARAM)"hellothisisatestsfat;erjtaslfa;sdf");
		}
		//SendMessage(SCI_SETREADONLY, 1);
	}
	*/

	CheckUpdateMarginWidth();

	//SC_WRAP_WORD tries to wrap only between words as indicated by white space or style changes although if a word is longer than a line,
	//it will be wrapped before the line end. 
	//SC_WRAP_CHAR is preferred to SC_WRAP_WORD for Asian languages where there is no white space between words.

	SendMessage(SCI_SETWRAPMODE, SC_WRAP_CHAR);

	//SetTimer(eTimerTest, 3000, nullptr);
	//SetTimer(eTimerDump, 1000,nullptr);

	//SendMessage(SCI_SETYCARETPOLICY);// , CARET_STRICT, CARET_STRICT);// , CARET_JUMPSand CARET_EVEN, );
	SetUtf8();
	return 0;
}

void LogWnd::OnTimer(UINT_PTR nIDEvent)
{
	switch (nIDEvent)
	{
	case eTimerDump:
	{
		auto pos = SendMessage(SCI_GETCURRENTPOS);
		auto currentLine = SendMessage(SCI_LINEFROMPOSITION, pos);
		auto lineCount = SendMessage(SCI_GETLINECOUNT);
		auto len = SendMessage(SCI_GETLENGTH);
		auto totalLines = SendMessage(SCI_LINEFROMPOSITION, len);
		auto anchor=SendMessage(SCI_GETANCHOR);

		bool hasSelect = (anchor != pos);
		//LogV(TAG,"line:%d/%d",currentLine,lineCount);
		//LogV(TAG,"anchor:%d,pos=%d,len=%d", anchor,pos,len);

		/*
		//SCI_MOVECARETINSIDEVIEW
		//If the caret is off the top or bottom of the view, it is moved to the nearest line that is visible to its current position.Any selection is lost.
		//SCI_SETSEL(position anchor, position caret)
		SCI_SETCURRENTPOS(position caret)

		*/

		/*
		if (currentLine == lineCount - 1)
		{
			SendMessage(SCI_SETCURRENTPOS, len);// SCI_GETLENGTH
			SendMessage(SCI_SETSEL, len, len);
			SendMessage(SCI_MOVECARETINSIDEVIEW);
		}
		*/

		break;
	}
	case eTimerTest:
	{
		KillTimer(eTimerTest);

#if 0
		enum
		{
			styleNotice = 88,
			styleGray,
			styleError,
			styleAnnotation,
		};

		SendMessage(SCI_STYLESETFORE, styleNotice, RGB(0, 0, 255));
		SendMessage(SCI_STYLESETFORE, styleGray, RGB(0x80, 0x80, 0x80));
		//SendMessage(SCI_STYLESETBACK, styleGray, RGB(0xff, 0x0, 0xff));//test ok
		SendMessage(SCI_STYLESETFORE, styleError, RGB(255, 0, 0));

		SendMessage(SCI_STYLESETFORE, styleAnnotation, RGB(128, 128, 128));
		//SendMessage(SCI_STYLESETBACK, styleAnnotation, RGB(255, 0, 255));
		SendMessage(SCI_STYLESETSIZE, styleAnnotation, 16);
		//SendMessage(SCI_STYLESETSIZE, styleAnnotation, 24);
		SendMessage(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD);
		{
			for (int i = 0; i < 10; i++)
			{
				auto text = StringTool::Format("this is  a test line to show style demo,idx=%04d\r\n", i);
				auto line = AddLog(text);

				if (line == 3)
				{
					SendMessage(SCI_ANNOTATIONSETSTYLE, line, styleAnnotation);

					auto msg = StringTool::Format(u8"标注测试 %04d", line);
					auto text = Utf8Tool::UTF_8ToGB2312(msg);
					SendMessage(SCI_ANNOTATIONSETTEXT, line, (LPARAM)text.c_str());
				}
			}


			SendMessage(SCI_STARTSTYLING, 0, 0);
			SendMessage(SCI_SETSTYLING, 10, styleNotice);

			SendMessage(SCI_STARTSTYLING, 20, 0);
			SendMessage(SCI_SETSTYLING, 30, styleError);

			auto v = SendMessage(SCI_STYLEGETSIZE, STYLE_LINENUMBER);
		}

		//SendMessage(SCI_STYLESETSIZE, STYLE_LINENUMBER,v*10);//not work
		//SendMessage(SCI_STYLESETBOLD, STYLE_LINENUMBER,true);//not work

		/*
		{
			int margin = 3;
			int pixelWidth = 200;
			SendMessage(SCI_SETMARGINWIDTHN, margin, pixelWidth);

			auto msg = u8"时间 2021.06.18 09:37:00.345";
			auto text = Utf8Tool::UTF_8ToGB2312(msg);
			SendMessage(SCI_MARGINSETTEXT,line, (LPARAM)text.c_str());
		}
		*/

		/*
		AddLog("item1\r\n");
		
		AddLog("item2\r\n");
		break;
		*/

		{
			/*
			SCI_APPENDTEXT(position length, const char *text)
				This adds the first length characters from the string text to the end of the document. 
				This will include any 0's in the string that you might have expected to stop the operation. 
				The current selection is not changed and the new text is not scrolled into view.

			*/

			SendMessage(SCI_SETREADONLY, 0);
			string text;
			static int idx = 0;
			for (int i = 0; i < 1; i++)
			{
				text = StringTool::Format("line %04d\r\n", idx);
				++idx;
				SendMessage(SCI_APPENDTEXT, text.length(), (LPARAM)text.c_str());

				//wp is length to add
				//SendMessage(SCI_ADDTEXT, 4, (LPARAM)"Hilo,world!");
				//SendMessage(SCI_ADDSTYLEDTEXT, 10, (LPARAM)"hellothisisatestsfat;erjtaslfa;sdf");
			}
			SendMessage(SCI_SETREADONLY, 1);
		}
#endif
		break;
	}
	}

	CWnd::OnTimer(nIDEvent);
}

void LogWnd::clear()
{
	SendMessage(SCI_SETREADONLY, 0);
	SendMessage(SCI_CLEARALL);
	SendMessage(SCI_SETREADONLY, 1);

	CheckUpdateMarginWidth();
}

void LogWnd::CopyAll()
{
	auto len=SendMessage(SCI_GETLENGTH);
	SendMessage(SCI_COPYRANGE, 0, len);
}

int LogWnd::AddLog(const string& text)
{
	bool scroll = false;

	auto pos = SendMessage(SCI_GETCURRENTPOS);
	//auto currentLine = SendMessage(SCI_LINEFROMPOSITION, pos);
	//auto lineCount = SendMessage(SCI_GETLINECOUNT);
	auto len = SendMessage(SCI_GETLENGTH);
	//auto totalLines = SendMessage(SCI_LINEFROMPOSITION, len);
	auto anchor = SendMessage(SCI_GETANCHOR);
	//if (!hasSelect && (lineCount == 0 || currentLine == lineCount - 1))
	if(anchor == pos && pos==len)
	{
		scroll = true;
	}

	SendMessage(SCI_SETREADONLY, 0);
	SendMessage(SCI_APPENDTEXT, text.length(), (LPARAM)text.c_str());
	SendMessage(SCI_SETREADONLY, 1);

	CheckUpdateMarginWidth();

	if (scroll)
	{
		SendMessage(SCI_DOCUMENTEND);
	}

	{
		auto len = SendMessage(SCI_GETLENGTH);
		auto line=SendMessage(SCI_LINEFROMPOSITION, len);
		auto pos2 = SendMessage(SCI_GETCURRENTPOS);
		//LogV(TAG, "pos=%d,pos2=%d,line=%d", pos, pos2,line);
	
		return line;

	}
}

void LogWnd::CheckUpdateMarginWidth()
{
	int lineNumbersWidth = 2;
	auto lineCount = SendMessage(SCI_GETLINECOUNT);
	lineCount = MAX(lineCount, 100);
	int lineNumWidth = 1;
	while (lineCount >= 10) {
		lineCount /= 10;
		++lineNumWidth;
	}

	if (lineNumWidth < lineNumbersWidth) {
		lineNumWidth = lineNumbersWidth;
	}

	if (lineNumWidth < 0)
		lineNumWidth = 0;
	// The 4 here allows for spacing: 1 pixel on left and 3 on right.
	std::string nNines(lineNumWidth, '9');
	const WPARAM pixelWidth = 4 + SendMessage(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)nNines.c_str());
	SendMessage(SCI_SETMARGINWIDTHN, 0, pixelWidth);
}

void LogWnd::SetUtf8()
{
	LogV(TAG, "%s", __func__);
	SendMessage(SCI_SETCODEPAGE, SC_CP_UTF8);
}

void LogWnd::SetGB()
{
	LogV(TAG, "%s", __func__);
	SendMessage(SCI_SETCODEPAGE, 936);//解决app为unicode版时中文乱码问题
}


