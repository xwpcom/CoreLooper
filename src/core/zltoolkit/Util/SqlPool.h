﻿/*
 * MIT License
 *
 * Copyright (c) 2016-2019 xiongziliang <771730766@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SQL_SQLPOOL_H_
#define SQL_SQLPOOL_H_

#include <deque>
#include <mutex>
#include <memory>
#include <sstream>
#include <functional>
#include "Poller/Timer.h"
#include "logger.h"
#include "SqlConnection.h"
#include "Thread/WorkThreadPool.h"
#include "Util/ResourcePool.h"

using namespace std;

namespace toolkit {

class SqlPool : public std::enable_shared_from_this<SqlPool> {
public:
	typedef std::shared_ptr<SqlPool> Ptr;
	typedef ResourcePool<SqlConnection> PoolType;
	typedef vector<vector<string> > SqlRetType;

	static SqlPool &Instance();

	~SqlPool() {
		_timer.reset();
		flushError();
		_threadPool.reset();
		_pool.reset();
		InfoL;
	}

	/**
	 * 设置循环池对象个数
	 * @param size
	 */
	void setSize(int size) {
		checkInited();
		_pool->setSize(size);
	}

	/**
	 * 初始化循环池，设置数据库连接参数
	 * @tparam Args
	 * @param arg
	 */
	template<typename ...Args>
	void Init(Args && ...arg) {
		_pool.reset(new PoolType(std::forward<Args>(arg)...));
		_pool->obtain();
	}


	/**
	 * 异步执行sql
	 * @param str sql语句
	 * @param tryCnt 重试次数
	 */
	template<typename ...Args>
	void asyncQuery(Args &&...args) {
		asyncQuery_l(SqlConnection::queryString(std::forward<Args>(args)...));
	}


	/**
	 * 同步执行sql
	 * @tparam Args 可变参数类型列表
	 * @param arg 可变参数列表
	 * @return 影响行数
	 */

	template<typename ...Args>
	int64_t syncQuery(Args &&...arg) {
		checkInited();
		typename PoolType::ValuePtr mysql;
		try {
			//捕获执行异常
			mysql = _pool->obtain();
			return mysql->query(std::forward<Args>(arg)...);
		} catch (exception &e) {
			mysql.quit();
			throw;
		}
	}


	/**
	 * sql转义
	 * @param str
	 * @return
	 */
	string escape(const string &str) {
		checkInited();
		return _pool->obtain()->escape(const_cast<string &>(str));
	}

private:
	SqlPool()  {
		_threadPool = WorkThreadPool::Instance().getExecutor();
		_timer = std::make_shared<Timer>(30,[this](){
			flushError();
			return true;
		}, nullptr);
	}

	/**
	 * 异步执行sql
	 * @param sql sql语句
	 * @param tryCnt 重试次数
	 */
	void asyncQuery_l(const string &sql,int tryCnt = 3) {
		auto lam = [this,sql,tryCnt]() {
			int64_t rowID;
			auto cnt = tryCnt - 1;
			try {
				syncQuery(rowID,sql);
			}catch(exception &ex) {
				if( cnt > 0) {
					//失败重试
					lock_guard<mutex> lk(_error_query_mutex);
					sqlQuery query(sql,cnt);
					_error_query.push_back(query);
				}else{
					WarnL <<  ex.what();
				}
			}
		};
		_threadPool->async(lam);
	}

	/**
	 * 定时重试失败的sql
	 */
	void flushError() {
		decltype(_error_query) query_copy;
		{
			lock_guard<mutex> lck(_error_query_mutex);
			query_copy.swap(_error_query);
		}
		for (auto &query : query_copy) {
			asyncQuery(query.sql_str,query.tryCnt);
		}
	}

	/**
	 * 检查数据库连接池是否初始化
	 */
	void checkInited(){
		if(!_pool){
			throw SqlException("SqlPool::checkInited","数据库连接池未初始化");
		}
	}
private:
	struct sqlQuery {
		sqlQuery(const string &sql,int cnt):sql_str(sql),tryCnt(cnt){}
		string sql_str;
		int tryCnt = 0;
	} ;
private:
	deque<sqlQuery> _error_query;
	TaskExecutor::Ptr _threadPool;
	mutex _error_query_mutex;
	std::shared_ptr<PoolType> _pool;
	Timer::Ptr _timer;
};

/**
 * Sql语句生成器，通过占位符'？'的方式生成sql语句
 */
class SqlStream {
public:
	SqlStream(const char *sql) : _sql(sql){}
	~SqlStream() {}

	template<typename T>
	SqlStream& operator <<(T &&data) {
		auto pos = _sql.find('?', _startPos);
		if (pos == string::npos) {
			return *this;
		}
		_str_tmp.str("");
		_str_tmp << std::forward<T>(data);
		string str = SqlPool::Instance().escape(_str_tmp.str());
		_startPos = pos + str.size();
		_sql.replace(pos, 1, str);
		return *this;
	}
	const string& operator <<(std::ostream&(*f)(std::ostream&)) const {
		return _sql;
	}
	operator string (){
		return _sql;
	}
private:
	stringstream _str_tmp;
	string _sql;
	string::size_type _startPos = 0;
};


/**
 * sql查询器
 */
class SqlWriter {
public:
	/**
	 * 构造函数
	 * @param sql 带'？'占位符的sql模板
	 * @param throwAble 是否抛异常
	 */
	SqlWriter(const char *sql,bool throwAble = true) : _sqlstream(sql),_throwAble(throwAble) {}
	~SqlWriter() {}

	/**
	 * 输入参数替换占位符'？'以便生成sql语句；可能抛异常
	 * @tparam T 参数类型
	 * @param data 参数
	 * @return 本身引用
	 */
	template<typename T>
	SqlWriter& operator <<(T &&data) {
		try {
			_sqlstream << std::forward<T>(data);
		}catch (std::exception &ex){
			//在转义sql时可能抛异常
			if(!_throwAble){
				WarnL << ex.what();
			}else{
				throw;
			}
		}
		return *this;
	}

	/**
	 * 异步执行sql，不会抛异常
	 * @param f std::endl
	 */
	void operator <<(std::ostream&(*f)(std::ostream&)) {
		//异步执行sql不会抛异常
		SqlPool::Instance().asyncQuery((string)_sqlstream);
	}

	/**
	 * 同步执行sql，可能抛异常
	 * @tparam Row 数据行类型，可以是vector<string>/list<string>等支持 obj.emplace_back("value")操作的数据类型
	 * 			   也可以是map<string,string>/Json::Value 等支持 obj["key"] = "value"操作的数据类型
	 * @param ret 数据存放对象
	 * @return 影响行数
	 */
	template <typename Row>
	int64_t operator <<(vector<Row> &ret) {
		try {
			_affectedRows = SqlPool::Instance().syncQuery(_rowId,ret, (string)_sqlstream);
		}catch (std::exception &ex){
			if(!_throwAble){
				WarnL << ex.what();
			} else {
				throw;
			}
		}
		return _affectedRows;
	}

	/**
	 * 在insert数据库时返回插入的rowid
	 * @return
	 */
	int64_t getRowID() const {
		return _rowId;
	}

	/**
	 * 返回影响数据库数据行数
	 * @return
	 */
	int64_t getAffectedRows() const {
		return _affectedRows;
	}
private:
	SqlStream _sqlstream;
	int64_t _rowId = -1;
	int64_t _affectedRows = -1;
	bool _throwAble = true;
};

} /* namespace toolkit */

#endif /* SQL_SQLPOOL_H_ */
