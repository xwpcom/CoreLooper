#ifndef _DEBUG_TRACE_2004
#define _DEBUG_TRACE_2004

#include <Windows.h>

#define DEBUG_TRACE
//只在release下并且也定义了DEBUG_TRACE的情况下输出到文件
#ifndef _DEBUG
	#define	DEBUG_TRACE_FILE
#endif

#ifdef DEBUG_TRACE
typedef int BOOL;
	BOOL DebugTrace(char* lpszFormat,...);
	#define DT	DebugTrace
	#define _DT DEBUG_TRACE
#else
	#define DT
	#undef _DT
#endif


#endif
