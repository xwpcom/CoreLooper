#pragma once

namespace Bear {
namespace Core
{
class NameValue;
#include "namevalue.h"
class CORE_EXPORT XmlString
{
public:
	void AddItem(const char *name, const std::string& value, std::shared_ptr<NameValue> props = nullptr);
	void AddItem(const char *name, int value, std::shared_ptr<NameValue> props = nullptr);
	void AddItem(const char *name, long long value, std::shared_ptr<NameValue> props = nullptr);
	void AddItem(const char *name, DWORD value, std::shared_ptr<NameValue> props = nullptr);
	void AddItem(const char *name, long value, std::shared_ptr<NameValue> props = nullptr);

	const std::string& GetString()const
	{
		return mString;
	}
protected:
	std::string mString;
};

}
}
