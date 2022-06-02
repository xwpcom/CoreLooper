#include "stdafx.h"
#include "mysqlex.h"
#include "mysqlres.h"

#ifdef __AFX_H__
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

#pragma comment(lib,"libmysql.lib")
using namespace Bear::Core;
using namespace Bear::Core::FileSystem;

namespace Database {
static const char* TAG = "MySql";

CString	MySql::m_mysqlAppPath;

MySql::MySql()
{
	m_pMySql = NULL;
	m_dwTickKeepAlive = 0;
}

MySql::~MySql()
{
	Disconnect();
}

bool MySql::IsConnect()
{
	if (!m_pMySql)
	{
		return FALSE;
	}

	int nSecond = 30;
#ifdef _DEBUG
	nSecond = 5;
#endif

	DWORD dwTick = GetTickCount();
	if (dwTick >= m_dwTickKeepAlive + nSecond * 1000)
	{
		int ret = mysql_ping(m_pMySql);
		m_dwTickKeepAlive = GetTickCount();
		if (ret)
		{
			return FALSE;
		}
	}

	return (NULL != m_pMySql);
}

int MySql::Connect(const char* server, const char* user, const char* password, const char* database, int port)
{
	ASSERT(m_pMySql == NULL);

	int iReValue = 0;
	m_pMySql = mysql_init(NULL);
	if (!m_pMySql)
	{
		LogW(TAG,"fail mysql_init");
		return -1;
	}


	{
		//https://dev.mysql.com/doc/refman/8.0/en/charset-connection.html

		auto ret = mysql_options(m_pMySql, MYSQL_SET_CHARSET_NAME, "utf8");
		mysql_options(m_pMySql, MYSQL_INIT_COMMAND,"SET NAMES utf8;");
		//SetUtf8();
		//	mysql_set_character_set(m_pMySql,"utf8");

		char reconnectArg = 1;
		mysql_options(m_pMySql, MYSQL_OPT_RECONNECT, (char*)&reconnectArg);
	}

#ifdef _DEBUG
	{
		auto con = m_pMySql;
		//LogV(TAG, "mysql_get_server_info=[%s]", mysql_get_server_info(con));
		//LogV(TAG, "mysql_get_client_info=[%s]", mysql_get_client_info());//5.0.21
	}
#endif

	//http://dev.mysql.com/doc/refman/5.0/en/mysql-affected-rows.html
	//the CLIENT_FOUND_ROWS flag to mysql_real_connect() when connecting to mysqld, 
	//the affected-rows value is the number of rows “found”; that is, matched by the WHERE clause. 
	if (NULL == mysql_real_connect(m_pMySql, server, user, password, database, port, NULL,
		CLIENT_MULTI_RESULTS
		| CLIENT_MULTI_STATEMENTS
		| CLIENT_FOUND_ROWS
	))
	{
		iReValue = mysql_errno(m_pMySql);
		LogW(TAG,"mysql iReValue=%d(%s)", iReValue, mysql_error(m_pMySql));
		Disconnect();
	}
	else
	{
		auto ret = mysql_options(m_pMySql, MYSQL_SET_CHARSET_NAME, "utf8");
		SetUtf8();
		//	mysql_set_character_set(m_pMySql,"utf8");
		mysql_options(m_pMySql, MYSQL_INIT_COMMAND, "SET NAMES utf8;");
		char reconnectArg = 1;
		mysql_options(m_pMySql, MYSQL_OPT_RECONNECT, (char*)& reconnectArg);
	}

	return iReValue;
}

/*
https://blog.csdn.net/kakarot5/article/details/40088137
set names utf8之前，
character_set_client    | gbk
character_set_connection| gbk
character_set_results   | gbk

set names utf8之后，
character_set_client    | utf8
character_set_connection| utf8
character_set_results   | utf8
---------------------
作者：兴_业
来源：CSDN
原文：https://blog.csdn.net/kakarot5/article/details/40088137
版权声明：本文为博主原创文章，转载请附上博文链接！

*/
//在CMySql::Connect成功之后调用SetUtf8()
void MySql::SetUtf8()
{
	ASSERT(m_pMySql);
	
	if (m_pMySql)
	{
		mysql_query(m_pMySql, "SET NAMES UTF8;");
	}
}
//调用者负责释放返回的数据
//一般采用MySqlRes rs=MySql::Query()在MySqlRes析构时自动释放
MYSQL_RES* MySql::Query(const char* sql)
{
	if (!m_pMySql)
	{
		//ASSERT(FALSE);
		return NULL;
	}

	try
	{
		int ret = mysql_query(m_pMySql, sql);
		if (ret == 0)
		{
			MYSQL_RES* rs = mysql_store_result(m_pMySql);
			return rs;
		}
		else
		{
			LogW(TAG,"mysql error=[%s],sql=[%s]", mysql_error(m_pMySql), sql);
		}
	}
	catch (...)
	{
	}

	int err = mysql_errno(m_pMySql);
	SetLastError(err);

	return NULL;
}

//Execute不返回数据,Query返回数据
void MySql::Execute(const char* sql)
{
	MySqlRes rs = Query(sql);
}

DWORD MySql::GetAffectedRows()
{
	if (!m_pMySql)
	{
		ASSERT(FALSE);
		return 0;
	}

	long nc = (long)mysql_affected_rows(m_pMySql);
	if (nc < 0)
	{
		nc = 0;
	}
	return nc;
}

// **************************************************************
// Function	  : Disconnect
// Description: 关闭数据库连接
// Return	  : void
// Author	  : Yuncai Yan
// Date		  : 2011-2-10
// Revisions  : 
// **************************************************************
void MySql::Disconnect()
{
	if (m_pMySql)
	{
		mysql_close(m_pMySql);
		m_pMySql = NULL;
	}
}

int MySql::SetBin(const char* pszSQL, LPBYTE pData, ULONG cbData)
{
	MYSQL_STMT* pStmt = mysql_stmt_init(m_pMySql);
	if (pStmt)
	{
		if (0 == mysql_stmt_prepare(pStmt, pszSQL, (long)strlen(pszSQL)))
		{
			MYSQL_BIND bind[1];
			memset(bind, 0, sizeof(bind));
			bind[0].buffer = (void*)pData;
			bind[0].buffer_type = MYSQL_TYPE_BLOB;
			bind[0].is_null = 0;
			bind[0].length = &cbData;

			if (0 == mysql_stmt_bind_param(pStmt, bind))
			{
				if (0 == mysql_stmt_send_long_data(pStmt, 0, (const char*)pData, cbData))
				{
					if (0 == mysql_stmt_execute(pStmt))
					{
						mysql_stmt_close(pStmt);
						return 0;
					}
					int ret;
					ret = mysql_stmt_errno(pStmt);
				}
			}
		}

		mysql_stmt_close(pStmt);
	}

	return -1;
}

int MySql::GetBin(const char* pszSQL, LPBYTE pData, ULONG cbData)
{
	ASSERT(FALSE);
	return -1;
}

//返回当前安装的mysql版本
CString MySql::GetMySQLVersion()
{
	CString mysqlPath;
	{
		CRegKey reg;
		LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MySQL AB\\MySQL Server 5.0"));
		if (ret == 0)
		{
			TCHAR buf[MAX_PATH];
			memset(buf, 0, sizeof(buf));
			ULONG cbSize = sizeof(buf);
			reg.QueryStringValue(_T("Version"), buf, &cbSize);
			mysqlPath = buf;
		}
	}

	return mysqlPath;
}

//返回mysql.exe的全路径，可用来粗略判断是否安装了mysql
CString MySql::GetMySQLAppPath()
{
	CString mysqlPath;
	{
		CRegKey reg;
		LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\MySQL AB\\MySQL Server 5.0"));
		if (ret == 0)
		{
			TCHAR buf[MAX_PATH];
			memset(buf, 0, sizeof(buf));
			ULONG cbSize = sizeof(buf);
			reg.QueryStringValue(_T("Location"), buf, &cbSize);

			mysqlPath = buf;
			if (mysqlPath.IsEmpty())
			{
				return _T("");
			}

			mysqlPath += "\\bin\\mysql.exe";
		}
	}

	return mysqlPath;
}

BOOL MySql::DropDatabase(const char* server, int port, const char* user, const char* password, const char* database)
{
	MySql mysql;
	int ret = mysql.Connect(server, user, password, NULL, port);
	if (ret == 0)
	{
		CString sql;
		sql.Format(_T("Drop Database  If Exists %s;"), database);
		USES_CONVERSION;
		MySqlRes rs = mysql.Query(T2A(sql));
		int nc = mysql.GetAffectedRows();
		return TRUE;
	}

	return FALSE;
}

BOOL MySql::CreateDatabase(const char* server, int port, const char* user, const char* password, const char* database)
{
	MySql mysql;
	int ret = mysql.Connect(server, user, password, NULL, port);
	if (ret == 0)
	{
		CString sql;
		sql.Format(_T("Create Database If Not Exists %s;"), database);
		USES_CONVERSION;
		MySqlRes rs = mysql.Query(T2A(sql));
		int nc = mysql.GetAffectedRows();
		return nc == 1;
	}

	return FALSE;
}

//导入数据库
//TestTable是用来检测table是否创建成功
eMySqlError MySql::ImportDatabase(const char* svr, int port, const char* user, const char* password,
	const char* database, const char* SqlFile, const char* TestTable)
{
	BOOL bCreate = FALSE;
	BOOL bDrop = MySql::DropDatabase(svr, port, user, password, database);
	bCreate = MySql::CreateDatabase(svr, port, user, password, database);
	LogV(TAG,"bDrop=%d,bCreate=%d", bDrop, bCreate);

	if (!bDrop && !bCreate)
	{
		return eMySqlError_AccessMySqlFail;
	}


	//由于用户可能没有添加mysql.exe路径到path环境变量中,所以不能直接运行mysql.exe
	//下面自动获取mysql.exe的全路径
	CString mysqlPath = MySql::GetMySQLAppPath();
	if (mysqlPath.IsEmpty())
	{
		return eMySqlError_MySqlNoInstall;
	}

	USES_CONVERSION;
	CString dbSQLFile = A2T(SqlFile);

	//mysqldump导出的.sql不带use xxx;语句，导致直接导入时会报no database selected.
	//所以下面采用临时文件添加此行
	{
		CString tmpSQLFile;
		tmpSQLFile.Format(_T("%s.sql"), dbSQLFile);

		FILE* hSrc = File::fopen(T2A(dbSQLFile), "rb");
		FILE* hDst = File::fopen(T2A(tmpSQLFile), "wb");
		if (!hSrc || !hDst)
		{
			fclose(hSrc);
			fclose(hDst);
			return eMySqlError_FileOperationFail;
		}

		CString szFirstLine;
		szFirstLine.Format(_T("use %s;\r\n"), database);
		fwrite(szFirstLine, 1, szFirstLine.GetLength(), hDst);
		while (1)
		{
			BYTE buf[4096];
			auto ret = fread(buf, 1, sizeof(buf), hSrc);
			if (ret > 0)
			{
				fwrite(buf, 1, ret, hDst);
			}
			else
			{
				break;
			}
		}
		fclose(hSrc);
		fclose(hDst);

		dbSQLFile = tmpSQLFile;
	}

	CString batFile;
	batFile.Format(_T("%s\\mysql.bat"), ShellTool::GetAppPath().c_str());
	{
		CFile file;
		BOOL bOK = file.Open(batFile, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite);
		if (bOK)
		{
			CString bat;
			//注意下面-u -p与%s之间不能有空格，否则会失败
			//对于mysql.exe的这种参数判断方式无语,相当于所有-uXXX 和-pXXX都不能做其他用途了
			bat.Format(_T("\"%s\" -u%s -p%s\r\n"), mysqlPath, user, password);
			//bat="mysql.exe -uroot -pxxxxxx\r\n";
			file.Write(bat, bat.GetLength());
		}
	}

	CString cmd;
	cmd.Format(_T("\"%s\" -u %s -p %s %s < \"%s\""), batFile, user, password, database, dbSQLFile);
	LogV(TAG,"cmd=[%s]", cmd);
	//cmd.Format("mysql.bat -u %s -p %s %s < %s",user,password,database,dbSQLFile);
	//UINT ret = WinExec("mysql.bat -u root -p xxxxxx game < game.sql",SW_HIDE);
	//经测试，可以成功导入带中文数据的game.sql
	//用CreateProcess测试失败

	UINT ret = WinExec(T2A(cmd), SW_HIDE);
	//说明:不能马上删除batFile,否则导入会失败
	Sleep(2000);
	DeleteFile(batFile);
	DeleteFile(dbSQLFile);
	/*
	return -1;
	char CommandLine[MAX_PATH];
	_snprintf(CommandLine,sizeof(CommandLine)-1,
		//" -uroot -pxxxxxx game2 < c:\\game.sql"
		" -u root -p xxxxxx game < c:\\game.sql"
		);

	STARTUPINFO si={0};
	si.cb=sizeof(si);
	si.wShowWindow=SW_SHOW;
	si.dwFlags=STARTF_USESHOWWINDOW;

	PROCESS_INFORMATION pi={0};
	BOOL bOK=CreateProcess(
		app,
		CommandLine,
		NULL,//__in          LPSECURITY_ATTRIBUTES lpProcessAttributes,
		NULL,//__in          LPSECURITY_ATTRIBUTES lpThreadAttributes,
		FALSE,//__in          BOOL bInheritHandles,
		0,//__in          DWORD dwCreationFlags,
		NULL,//__in          LPVOID lpEnvironment,
		GetAppPath(),//NULL,//__in          LPCTSTR lpCurrentDirectory,
		&si,//__in          LPSTARTUPINFO lpStartupInfo,
		&pi//__out         LPPROCESS_INFORMATION lpProcessInformation
	);

	if(bOK)
	{
		WaitForSingleObject(pi.hProcess,INFINITE);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	//*/

	if (TestTable)
	{
		MySql mysql;
		int ret = mysql.Connect(svr, user, password, database, port);
		if (ret)
		{
			return eMySqlError_Fail;
		}

		USES_CONVERSION;

		CString sql;
		sql.Format(_T("SELECT * FROM %s"), TestTable);
		MySqlRes rs = mysql.Query(T2A(sql));
		LogV(TAG,"TestTable[%s] rows=%d", TestTable, rs.GetRowNum());
		if (rs.GetRowNum() > 0)
		{
			return eMySqlError_OK;
		}
		else
		{
			return eMySqlError_Fail;
		}
	}

	return eMySqlError_OK;
}

const char* MySql::GetErrorDesc(eMySqlError err)
{
	switch (err)
	{
	case eMySqlError_Fail:				return "失败";
	case eMySqlError_OK:				return "成功";
	case eMySqlError_MySqlNoInstall:	return "没有安装MYSQL数据库";
	case eMySqlError_FileOperationFail:	return "文件操作失败";
	case eMySqlError_AccessMySqlFail:	return "访问失败，请确保MySQL服务器正在运行，并提供正确的用户名和密码";
	default:							return "未知错误";
	}
}

CString MySql::getMySqlAppPath(CString server, int port, CString user, CString password)
{
	if (!m_mysqlAppPath.IsEmpty())
	{
		return m_mysqlAppPath;
	}

	USES_CONVERSION;
	MySql mysql;
	int ret = mysql.Connect(T2A(server), T2A(user), T2A(password), NULL, port);
	if (ret == 0)
	{
		CString sql = _T("select @@basedir as basePath from dual;");
		MySqlRes rs = mysql.Query(T2A(sql));
		CString path = A2T(rs.GetField("basedir"));
		LogV(TAG,"mysql path=%s", path);
		if (!path.IsEmpty())
		{
			path += "bin\\";
		}

		m_mysqlAppPath = path;
	}

	return m_mysqlAppPath;
}

LONGLONG MySql::GetLastInsertId(const string& fieldName)
{
	LONGLONG id = -1;

	string sql="SELECT last_insert_id()";////取加新增加的记录
	MySqlRes res = Query(sql);
	if (!res.IsEOF())
	{
		res.GetFieldLongLong(fieldName.c_str(), id);
	}

	return id;
}


}
