#pragma once

//XiongWanPing 2013.07.04
namespace Bear {
namespace Core
{
class IParam
{
public:
	virtual ~IParam() {}

	virtual string GetString(string name, string defValue = "") = 0;
	virtual int GetInt(string name, int defValue = 0) = 0;
	virtual bool IsParamExists(string name) = 0;
};
}
}