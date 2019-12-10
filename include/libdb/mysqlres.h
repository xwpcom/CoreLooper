#pragma once
#include <mysql.h>
namespace Database {

//XiongWanPing 2011.02.22
//封装mysql中的MYSQL_RES
class DB_EXPORT MySqlRes
{
public:
	MySqlRes(MYSQL_RES* pMySQLRes);
	MySqlRes& operator=(MYSQL_RES* pMySQLRes)
	{
		Detach();
		Attach(pMySQLRes);
		return *this;
	}

	~MySqlRes();

	// 获取结果集的行数
	DWORD GetRowNum()const;

	// 获取结果集的列数
	DWORD GetFieldNum()const;

	// 是否结果集末尾
	bool IsEOF()const;

	// 移动到第一行
	bool MoveFirst();

	// 移动到下一行
	bool MoveNext();

	// 根据field名称取数据
	const char* GetField(const char* pszFieldName)const;

	bool GetField(const char* pszFieldName, string& value)const;
	bool GetField(const char* pszFieldName, int& value)const;

	// 获取该字段值 BYTE返回
	bool GetFieldByte(const char* pszFieldName, BYTE& iFieldVal)const;

	// 获取该字段值 整形型返回
	bool GetFieldInt(const char* pszFieldName, int& iFieldVal)const;
	bool GetFieldBool(const char* pszFieldName, bool& iFieldVal)const;

	// 获取该字段值 浮点型返回
	bool GetFieldFloat(const char* pszFieldName, float& fFieldVal)const;

	// 获取该字段值 Double型返回
	bool GetFieldDouble(const char* pszFieldName, double& dFieldVal)const;

	// 获取该字段值 长整型返回
	bool GetFieldLong(const char* pszFieldName, long& lFieldVal)const;

	// 获取该字段值 64位整型返回
	bool GetFieldLongLong(const char* pszFieldName, LONGLONG& llFieldVal)const;

	// 获取该字段值 时间类型返回
	bool GetFieldTime(const char* pszFieldName, CTime& tFiledVal)const;
	//bool GetFieldTime(const char *pszFieldName, time_t & tFiledVal)const;
	bool GetFieldTime(const char* pszFieldName, LONG& tFiledVal)const;

	//返回指定字段的数据长度
	int GetFieldLength(const char* pszFieldName);

	//根据field名称取数据 时间COleDateTime类型返回
	bool GetFieldTime(const char* pszFieldName, COleDateTime& tFiledVal)const;

	// 将数据库查询结果联系到MySqlRes
	bool Attach(MYSQL_RES* pMySQLRes);

	// 释放pMySQLRes
	void Detach();

protected:
	// 获取字段的名称
	const char* GetFieldName(unsigned int iField)const;

	// 释放数据库查询结果
	static void FreeResult(MYSQL_RES* pMySQLRes);


	MYSQL_RES* m_pMySQLRes;
	MYSQL_ROW  m_CurrnetRowVal;
};

}
