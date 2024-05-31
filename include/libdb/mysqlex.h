#pragma once

#include <mysql.h>
namespace Database {
class MySqlRes;


struct DB_EXPORT tagDBInfo
{
	tagDBInfo()
	{
		Reset();
	}

	void Reset()
	{
		mIP.clear();
		mPort = 0;
		mUser.clear();
		mPassword.clear();
		mDBName.clear();
	}

	string mIP;
	int mPort;
	string mUser;
	string mPassword;
	string mDBName;
};

enum eMySqlError
{
	eMySqlError_Fail=-1,
	eMySqlError_OK=0,
	eMySqlError_MySqlNoInstall,
	eMySqlError_FileOperationFail,
	eMySqlError_AccessMySqlFail,

};

//XiongWanPing 2011.02.22
//封装mysql中的MYSQL
class DB_EXPORT MySql
{
public:
	MySql();
	virtual ~MySql();

	int Connect(const char * server, const char * user, const char * password, const char * database,int port=0);
	bool IsConnect();
	int ping();
	int autocommit(bool autoMode=true);
	int commit();

	LONGLONG GetLastInsertId(const string& fieldName);
	MYSQL_RES* Query(const string& sql)
	{
		return Query(sql.c_str());
	}
	MYSQL_RES *Query(const char *sql,bool reportError=true);
	void Execute(const char *sql, bool reportError = true);
	void Execute(const std::string& sql, bool reportError = true)
	{
		Execute(sql.c_str(),reportError);
	}
	DWORD GetAffectedRows()const;

	void Disconnect();

	int SetBin(const char *pszSQL,LPBYTE pData,ULONG cbData);
	int GetBin(const char *pszSQL,LPBYTE pData,ULONG cbData);

	static CString getMySqlAppPath(CString server, int port, CString user, CString password);
	static CString GetMySQLAppPath();
	static CString GetMySQLVersion();
	static const char* GetErrorDesc(eMySqlError err);

	static BOOL DropDatabase(const char* server, int port, const char* user, const char* password, const char* database);
	static BOOL CreateDatabase(const char* server, int port, const char* user, const char* password, const char* database);
	static eMySqlError ImportDatabase(const char* server, int port, const char* user, const char* password,
		const char* database, const char* dbSQLFile, const char* TestTable = NULL);

public:
	void SetUtf8();

	MYSQL		*m_pMySql=nullptr;
	DWORD		m_dwTickKeepAlive;

	static CString	m_mysqlAppPath;
};
}
