#include "stdafx.h"
#include "MySqlRes.h"

#ifdef __AFX_H__
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#endif

using namespace Bear::Core;
namespace Database {
static const char* TAG = "sqlRes";

MySqlRes::MySqlRes(MYSQL_RES* pMySQLRes)
{
	m_pMySQLRes = pMySQLRes;
	MoveFirst();
}

MySqlRes::~MySqlRes()
{
	Detach();
}

// **************************************************************
// Description: 获取结果集的行数
// Parameters : void
// Return	  : 
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// **************************************************************
DWORD MySqlRes::GetRowNum()const
{
	if (m_pMySQLRes)
	{
		return (DWORD)mysql_num_rows(m_pMySQLRes);
	}

	ASSERT(FALSE);
	return 0;
}

// **************************************************************
// Description: 获取结果集的列数
// Parameters : void
// Return	  : 
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// **************************************************************
DWORD MySqlRes::GetFieldNum()const
{
	if (m_pMySQLRes)
	{
		return mysql_num_fields(m_pMySQLRes);
	}

	ASSERT(FALSE);
	return 0;
}

// **************************************************************
// Description: 
// Parameters : void
// Return	  : 结果集莫返回TRUE
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// **************************************************************
bool MySqlRes::IsEOF()const
{
	if (m_pMySQLRes)
	{
		return (NULL == m_CurrnetRowVal);
	}

	return TRUE;
}

// **************************************************************
// Description: 移动到第一行
// Parameters : void
// Return	  : 当前行
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// **************************************************************
bool MySqlRes::MoveFirst()
{
	if (m_pMySQLRes)
	{
		mysql_data_seek(m_pMySQLRes, 0);
		m_CurrnetRowVal = mysql_fetch_row(m_pMySQLRes);
		return m_CurrnetRowVal != NULL;
	}

	return false;
}

// **************************************************************
// Description: 移动到下一行
// Parameters : 
// Return	  : 当前行
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// **************************************************************
bool MySqlRes::MoveNext()
{
	if (m_pMySQLRes)
	{
		m_CurrnetRowVal = mysql_fetch_row(m_pMySQLRes);
		return (NULL != m_CurrnetRowVal);
	}

	return false;
}

// **************************************************************
// Description: 获取字段的名称
// Parameters : 
// Return	  : 字段名
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// **************************************************************
const char* MySqlRes::GetFieldName(unsigned int iField)const
{
	if (iField >= GetFieldNum() || iField < 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	if (m_pMySQLRes)
	{
		MYSQL_FIELD* fields = NULL;
		mysql_field_seek(m_pMySQLRes, 0);
		fields = mysql_fetch_fields(m_pMySQLRes);
		if (fields)
			return fields[iField].name;
	}

	ASSERT(FALSE);
	return NULL;
}

// **************************************************************
// Description: 根据field名称取数据
// Parameters : pszFieldName 字段名
// Return	  : 获取错误返回NULL
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
const char* MySqlRes::GetField(const char* pszFieldName)const
{
	if (NULL == m_pMySQLRes)
	{
		ASSERT(FALSE);
		return "";
	}

	unsigned int index = 0;
	MYSQL_FIELD* field = NULL;

	mysql_field_seek(m_pMySQLRes, 0);
	bool found = false;
	while ((field = mysql_fetch_field(m_pMySQLRes)))
	{
		index++;
		if (_stricmp(field->name, "last_insert_id()") == 0 || _stricmp(field->name, pszFieldName) == 0)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		ASSERT(FALSE);
		return "";
	}

	if (index > 0 && m_CurrnetRowVal)
	{
		auto ret = m_CurrnetRowVal[index - 1];
		if (!ret)
		{
			ret = "";
		}

		return ret;
	}

	ASSERT(FALSE);
	return "";
}

// **************************************************************
// Function	  : GetFieldInt
// Description: 根据field名称取数据 整形型返回
// Parameters : pszFieldName 字段名
// Parameters : [out] iFieldVal 字段值
// Return	  : 获取错误返回false
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::GetFieldByte(const char* pszFieldName, BYTE& iFieldVal)const
{
	const char* pSqlRes = GetField(pszFieldName);
	if (pSqlRes)
	{
		iFieldVal = (BYTE)atoi(pSqlRes);
		return true;
	}
	else
	{
		DW("no find field value[%s]", pszFieldName);
		iFieldVal = -1;
	}

	//ASSERT(FALSE);
	return false;
}

bool MySqlRes::GetFieldBool(const char* pszFieldName, bool& iFieldVal)const
{
	const char* pSqlRes = GetField(pszFieldName);
	if (pSqlRes)
	{
		iFieldVal = !!atoi(pSqlRes);
		return true;
	}
	else
	{
		DW("no find field value[%s]", pszFieldName);
		iFieldVal = false;
	}

	//ASSERT(FALSE);
	return false;
}

bool MySqlRes::GetField(const char* pszFieldName, int& value)const
{
	const char* v = GetField(pszFieldName);
	if (v)
	{
		value = atoi(v);
		return true;
	}
	else
	{
		DW("no find field value[%s]", pszFieldName);
		value = 0;
	}

	//ASSERT(FALSE);
	return false;
}

bool MySqlRes::GetField(const char* pszFieldName, string& value)const
{
	const char* v = GetField(pszFieldName);
	if (v)
	{
		value = v;
		return true;
	}
	else
	{
		DW("no find field value[%s]", pszFieldName);
		value.clear();
	}

	//ASSERT(FALSE);
	return false;
}
// **************************************************************
// Function	  : GetFieldInt
// Description: 根据field名称取数据 整形型返回
// Parameters : pszFieldName 字段名
// Parameters : [out] iFieldVal 字段值
// Return	  : 获取错误返回false
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::GetFieldInt(const char* pszFieldName, int& iFieldVal)const
{
	const char* pSqlRes = GetField(pszFieldName);
	if (pSqlRes)
	{
		iFieldVal = atoi(pSqlRes);
		return true;
	}
	else
	{
		DW("no find field value[%s]", pszFieldName);
		iFieldVal = 0;
	}

	//ASSERT(FALSE);
	return false;
}

// **************************************************************
// Function	  : GetFieldFloat
// Description: 根据field名称取数据 浮点型返回
// Parameters : pszFieldName 字段名
// Parameters : [out] fFieldVal 字段值
// Return	  : 获取错误返回false
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::GetFieldFloat(const char* pszFieldName, float& fFieldVal)const
{
	const char* pSqlRes = GetField(pszFieldName);
	if (pSqlRes)
	{
		fFieldVal = (float)atof(pSqlRes);
		return true;
	}

	ASSERT(FALSE);
	return false;
}

// **************************************************************
// Function	  : GetFieldDouble
// Description: 根据field名称取数据 Double型返回
// Parameters : pszFieldName 字段名
// Parameters : [out] dFieldVal 字段值
// Return	  : 获取错误返回false
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::GetFieldDouble(const char* pszFieldName, double& dFieldVal)const
{
	const char* pSqlRes = GetField(pszFieldName);
	if (pSqlRes)
	{
		dFieldVal = atof(pSqlRes);
		return true;
	}

	ASSERT(FALSE);
	return false;
}

// **************************************************************
// Function	  : GetFieldLong
// Description: 根据field名称取数据 长整型返回
// Parameters : pszFieldName 字段名
// Parameters : [out] lFieldVal 该字段的值
// Return	  : 获取错误返回false
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::GetFieldLong(const char* pszFieldName, long& lFieldVal)const
{
	const char* pSqlRes = GetField(pszFieldName);
	if (pSqlRes)
	{
		lFieldVal = atol(pSqlRes);
		return true;
	}
	else
	{
		DW("no find field value[%s]", pszFieldName);
		lFieldVal = 0;
	}
	return false;
}

// **************************************************************
// Function	  : GetFieldLongLong
// Description: 获取该字段值 64位整型返回
// Parameters : pszFieldName 字段名
// Parameters : [out] llFieldVal 该字段的值
// Return	  : 获取错误返回false
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::GetFieldLongLong(const char* pszFieldName, LONGLONG& llFieldVal)const
{
	llFieldVal = 0;
	const char* pSqlRes = GetField(pszFieldName);
	if (pSqlRes)
	{
		llFieldVal = _atoi64(pSqlRes);
		return true;
	}

	//ASSERT(FALSE);
	return false;
}

// **************************************************************
// Function	  : GetFieldTime
// Description: 根据field名称取数据 时间类型返回
// Parameters : pszFieldName 字段名
// Parameters : [out] tFiledVal 该字段的值
// Return	  : 获取错误返回false
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::GetFieldTime(const char* pszFieldName, CTime& tFiledVal)const
{
	long value = 0;
	BOOL bOK = GetFieldLong(pszFieldName, value);
	if (bOK)
	{
		try
		{
			CTime tm((time_t)value);
			tFiledVal = tm;
			return true;
		}
		catch (...)
		{
			DW("invalid time format for field[%s]", pszFieldName);
		}
	}
	return false;
}

/*
bool MySqlRes::GetFieldTime(const char *pszFieldName, time_t & tm)const
{
	long value=0;
	BOOL bOK = GetFieldLong(pszFieldName,value);
	if(bOK)
	{
		tm=(time_t)value;
		return true;
	}
	return false;
}
//*/

bool MySqlRes::GetFieldTime(const char* pszFieldName, LONG& tm)const
{
	long value = 0;
	BOOL bOK = GetFieldLong(pszFieldName, value);
	if (bOK)
	{
		tm = value;
		return true;
	}
	return false;
}

// **************************************************************
// Function	  : GetFieldTime
// Description: 根据field名称取数据 时间COleDateTime类型返回
// Parameters : pszFieldName 字段名
// Parameters : [out] tFiledVal 该字段的值
// Return	  : 获取错误返回false
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::GetFieldTime(const char* pszFieldName, COleDateTime& tm)const
{
	const char* pSqlRes = GetField(pszFieldName);

	if (pSqlRes)
	{
		try
		{
			LONGLONG llTime = _atoi64(pSqlRes);
			DATE* pDate = (DATE*)& llTime;
			tm = *pDate;
			return true;
		}
		catch (...)
		{
			DW("invalid COleDateTime");
		}
	}

	ASSERT(FALSE);
	return false;
}


// **************************************************************
// Function	  : FreeResult
// Description: 释放数据库查询结果
// Parameters : pMySQLRes 要被释放的结果
// Return	  : void
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
void MySqlRes::FreeResult(MYSQL_RES* pMySQLRes)
{
	if (pMySQLRes)
	{
		mysql_free_result(pMySQLRes);
		pMySQLRes = NULL;
	}
}

// **************************************************************
// Function	  : Attach
// Description: 将数据库查询结果联系到MySqlRes
// Parameters : pMySQLRes MYSql查询后返回的结果
// Return	  : void
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
bool MySqlRes::Attach(MYSQL_RES* pMySQLRes)
{
	ASSERT(m_pMySQLRes == NULL);
	if (NULL != m_pMySQLRes)
		return false;

	m_pMySQLRes = pMySQLRes;
	MoveFirst();
	return true;
}

// **************************************************************
// Function	  : Detach
// Description: 释放pMySQLRes
// Parameters : void
// Return	  : void
// Author	  : Yuncai Yan
// Date		  : 2011-2-17
// Revisions  : 
// **************************************************************
void MySqlRes::Detach()
{
	if (m_pMySQLRes)
	{
		mysql_free_result(m_pMySQLRes);
		m_pMySQLRes = NULL;
		m_CurrnetRowVal = NULL;
	}
}

//返回指定字段的数据长度
int MySqlRes::GetFieldLength(const char* pszFieldName)
{
	if (NULL == m_pMySQLRes)
	{
		ASSERT(FALSE);
		return 0;
	}

	int index = -1;
	MYSQL_FIELD* field = NULL;

	mysql_field_seek(m_pMySQLRes, 0);
	while ((field = mysql_fetch_field(m_pMySQLRes)))
	{
		index++;
		if (0 == _stricmp(field->name, pszFieldName))
		{
			break;
		}
	}

	if (index >= 0)
	{
		unsigned long* lengths = mysql_fetch_lengths(m_pMySQLRes);
		return lengths[index];
	}

	return 0;
}

void MySqlRes::dump()
{
	auto row = mysql_num_rows(m_pMySQLRes);
	auto col = mysql_num_fields(m_pMySQLRes);

	vector<string> fieldNames;
	{
		MYSQL_FIELD* fields = NULL;
		mysql_field_seek(m_pMySQLRes, 0);
		fields = mysql_fetch_fields(m_pMySQLRes);

		if (fields)
		{
			for (int i = 0; i < col; i++)
			{
				auto item = fields[i];
				fieldNames.push_back(item.name);
			}
		}
	}

	MoveFirst();


	while (!IsEOF())
	{
		string text;
		for (auto& name : fieldNames)
		{
			text += GetField(name.c_str());
			text += ",";
		}

		LogV(TAG,"%s",text.c_str());

		MoveNext();
	}

	MoveFirst();
}

}
