#pragma once
#include "base/stringtool.h"
namespace Bear {
namespace Core
{

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
	virtual int OnProcDataGetter(const std::string& name, std::string& desc) = 0;
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

	virtual int OnProcDataSetter(std::string name, int value) = 0;
	virtual int OnProcDataSetter(std::string name, BYTE value) = 0;
	virtual int OnProcDataSetter(std::string name, bool value) = 0;
	virtual int OnProcDataSetter(std::string name, WORD value) = 0;
	virtual int OnProcDataSetter(std::string name, DWORD value) = 0;
	virtual int OnProcDataSetter(std::string name, double value) = 0;
	virtual int OnProcDataSetter(std::string name, LONGLONG value) = 0;
	virtual int OnProcDataSetter(std::string name, ULONGLONG value) = 0;
	virtual int OnProcDataSetter(std::string name, std::string value) = 0;
};

//XiongWanPing 2016.07.26
class CORE_EXPORT ProcNode
{
public:
	ProcNode();
	virtual ~ProcNode();

	void DumpData(std::string& xml);

	int Bind(std::string name, bool& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);
	int Bind(std::string name, BYTE& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);
	int Bind(std::string name, int& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);
	int Bind(std::string name, WORD& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);
	int Bind(std::string name, DWORD& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);
	int Bind(std::string name, double& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);
	int Bind(std::string name, LONGLONG& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);
	int Bind(std::string name, ULONGLONG& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);
	int Bind(std::string name, std::string& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter);

	bool GetBool(std::string name);
	BYTE GetByte(std::string name);
	int GetInt(std::string name);
	WORD GetWord(std::string name);
	DWORD GetDword(std::string name);
	double GetDouble(std::string name);
	LONGLONG GetLongLong(std::string name);
	ULONGLONG GetULongLong(std::string name);
	std::string GetString(std::string name);

	int SetBool(std::string name, bool value);
	int SetByte(std::string name, BYTE value);
	int SetInt(std::string name, int value);
	int SetWord(std::string name, WORD value);
	int SetDword(std::string name, DWORD value);
	int SetDouble(std::string name, double value);
	int SetLongLong(std::string name, LONGLONG value);
	int SetULongLong(std::string name, ULONGLONG value);
	int SetString(std::string name, std::string value);

protected:

	template<class T>
	struct tagItem
	{
		tagItem(const std::string& name, T& value, const std::string& desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter)
			:mName(name), mValue(value), mDesc(desc), mFlags(flags), mGetter(getter), mSetter(setter)
		{

		}

		std::string					mName;
		T&							mValue;
		std::string					mDesc;
		DWORD						mFlags;
		std::weak_ptr<IProcDataGetter>	mGetter;
		std::weak_ptr<IProcDataSetter>	mSetter;
	};

	template<class T>
	struct tagItems
	{
		//in order to keep order,use list other than map
		std::list<tagItem<T>> mList;

		int Bind(std::string name, T& value, std::string desc, DWORD flags, std::weak_ptr<IProcDataGetter> getter, std::weak_ptr<IProcDataSetter> setter)
		{
			tagItem<T> item(name, value, desc, flags, getter, setter);
			mList.push_back(item);
			return 0;
		}

		T Get(std::string name)
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

		int Set(std::string name, T value)
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

		void Dump(std::string& xml)
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

		void DumpName(tagItem<T> item, std::string& xml)
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

		//for tagItems<std::string>
		void DumpString(std::string& xml)
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

		static std::string mItemFormat;
	};

	tagItems<bool> mBools;
	tagItems<BYTE> mBytes;
	tagItems<int> mInts;
	tagItems<WORD> mWords;
	tagItems<DWORD> mDwords;
	tagItems<double> mDoubles;
	tagItems<LONGLONG> mLongLongs;
	tagItems<ULONGLONG> mULongLongs;
	tagItems<std::string> mStrings;
};

}
}