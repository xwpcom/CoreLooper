#pragma once
#include "core/base/stringtool.h"
#include <string>
namespace Bear {
namespace Core
{
using namespace std;

enum eProcDataFlag
{
	PDF_READ = 1,
	PDF_READ_WRITE = 2,
	PDF_GETTER = 4,
	PDF_SETTER = 8,
	PDF_WRITE_ONLY = 0x10,	//demo: /proc/reboot write 1 to reboot

	PDF_READABLE = PDF_READ | PDF_GETTER,
	PDF_WRITABLE = PDF_READ_WRITE | PDF_SETTER | PDF_WRITE_ONLY,
};

//only update when necessary
class IProcDataGetter
{
public:
	virtual ~IProcDataGetter() {}
	virtual int OnProcDataGetter(const string& name, string& desc) = 0;
};

enum eSetterResult
{
	eSetterResult_InvalidName = -2,
	eSetterResult_InvalidValue = -1,
	eSetterResult_Done = 0,				//handle by setter handler
	eSetterResult_DoDefault = 1,		//handle by ProcNode framework 
};

class IProcDataSetter
{
public:
	virtual ~IProcDataSetter() {}

	virtual int OnProcDataSetter(string name, int value) = 0;
	virtual int OnProcDataSetter(string name, BYTE value) = 0;
	virtual int OnProcDataSetter(string name, bool value) = 0;
	virtual int OnProcDataSetter(string name, WORD value) = 0;
	virtual int OnProcDataSetter(string name, DWORD value) = 0;
	virtual int OnProcDataSetter(string name, double value) = 0;
	virtual int OnProcDataSetter(string name, LONGLONG value) = 0;
	virtual int OnProcDataSetter(string name, ULONGLONG value) = 0;
	virtual int OnProcDataSetter(string name, string value) = 0;
};

//XiongWanPing 2016.07.26
class CORE_EXPORT ProcNode
{
public:
	ProcNode();
	virtual ~ProcNode();

	void DumpData(string& xml);

	int Bind(string name, bool& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);
	int Bind(string name, BYTE& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);
	int Bind(string name, int& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);
	int Bind(string name, WORD& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);
	int Bind(string name, DWORD& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);
	int Bind(string name, double& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);
	int Bind(string name, LONGLONG& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);
	int Bind(string name, ULONGLONG& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);
	int Bind(string name, string& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter);

	bool GetBool(string name);
	BYTE GetByte(string name);
	int GetInt(string name);
	WORD GetWord(string name);
	DWORD GetDword(string name);
	double GetDouble(string name);
	LONGLONG GetLongLong(string name);
	ULONGLONG GetULongLong(string name);
	string GetString(string name);

	int SetBool(string name, bool value);
	int SetByte(string name, BYTE value);
	int SetInt(string name, int value);
	int SetWord(string name, WORD value);
	int SetDword(string name, DWORD value);
	int SetDouble(string name, double value);
	int SetLongLong(string name, LONGLONG value);
	int SetULongLong(string name, ULONGLONG value);
	int SetString(string name, string value);

protected:

	template<class T>
	struct tagItem
	{
		tagItem(const string& name, T& value, const string& desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter)
			:mName(name), mValue(value), mDesc(desc), mFlags(flags), mGetter(getter), mSetter(setter)
		{

		}

		tagItem<T>& operator=(const tagItem<T>& src)
		{
			mName = src.mName;
			mValue = src.mValue;
			mDesc=src.mDesc;
			mFlags=src.mFlags;
			mGetter = src.mGetter;
			mSetter = src.mSetter;
			return *this;
		}

		string						mName;
		T&							mValue;
		string						mDesc;
		DWORD						mFlags;
		weak_ptr<IProcDataGetter>	mGetter;
		weak_ptr<IProcDataSetter>	mSetter;
	};

	template<class T>
	struct tagItems
	{
		//in order to keep order,use list other than map
		list<tagItem<T>> mList;

		int Bind(string name, T& value, string desc, DWORD flags, weak_ptr<IProcDataGetter> getter, weak_ptr<IProcDataSetter> setter)
		{
			tagItem<T> item(name, value, desc, flags, getter, setter);
			mList.push_back(item);
			return 0;
		}

		T Get(string name)
		{
			auto& lst = mList;
			for (auto iter = lst.begin(); iter != lst.end(); ++iter)
			{
				if (iter->mName == name)
				{
					shared_ptr<IProcDataGetter> getter = iter->mGetter.lock();
					if (getter)
					{
						getter->OnProcDataGetter(name, iter->mDesc);
					}

					return iter->mValue;
				}
			}

			ASSERT(FALSE);
			T defaultValue(0);
			return defaultValue;
		}

		int Set(string name, T value)
		{
			auto& lst = mList;
			for (auto iter = lst.begin(); iter != lst.end(); ++iter)
			{
				if (iter->mName == name)
				{
					if (iter->mFlags & PDF_WRITABLE)
					{
						shared_ptr<IProcDataSetter> setter = iter->mSetter.lock();
						eSetterResult result = eSetterResult_DoDefault;
						if (setter)
						{
							result = (eSetterResult)setter->OnProcDataSetter(name, value);
						}

						if (result < 0)
						{
							//fail
						}
						else if (result == eSetterResult_DoDefault)
						{
							iter->mValue = value;
						}
						else if (result == eSetterResult_Done)
						{
							//do nothing
						}
						else
						{
							ASSERT(FALSE);//unknown
						}

						return (int)result;
					}
					else
					{
						DW("%s is NOT writable", name.c_str());
						ASSERT(FALSE);
					}
				}
			}

			ASSERT(FALSE);
			return -1;
		}

		void Dump(string& xml)
		{
			auto& lst = mList;
			if (lst.empty())
			{
				return;
			}

			for (auto iter = lst.begin(); iter != lst.end(); ++iter)
			{
				DumpName(*iter, xml);
				xml += StringTool::Format(mItemFormat.c_str(), iter->mValue);
				xml += StringTool::Format("</%s>", iter->mName.c_str());

			}
		}

		void DumpName(tagItem<T> item, string& xml)
		{
			shared_ptr<IProcDataGetter>	getter = item.mGetter.lock();
			if (getter)
			{
				getter->OnProcDataGetter(item.mName, item.mDesc);
			}

			if (item.mDesc.empty())
			{
				xml += StringTool::Format("<%s>", item.mName.c_str());
			}
			else
			{
				xml += StringTool::Format("<%s desc=\"%s\">", item.mName.c_str(), item.mDesc.c_str());
			}
		}

		//for tagItems<string>
		void DumpString(string& xml)
		{
			auto& lst = mList;
			if (lst.empty())
			{
				return;
			}

			for (auto iter = lst.begin(); iter != lst.end(); ++iter)
			{
				DumpName(*iter, xml);
				//xml+=StringTool::Format("<%s>", iter->mName.c_str());
				xml += StringTool::Format("%s</%s>", iter->mValue.c_str(), iter->mName.c_str());
			}
		}

		static string mItemFormat;
	};

	tagItems<bool> mBools;
	tagItems<BYTE> mBytes;
	tagItems<int> mInts;
	tagItems<WORD> mWords;
	tagItems<DWORD> mDwords;
	tagItems<double> mDoubles;
	tagItems<LONGLONG> mLongLongs;
	tagItems<ULONGLONG> mULongLongs;
	tagItems<string> mStrings;
};

}
}