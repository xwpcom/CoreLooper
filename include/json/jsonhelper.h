#pragma once
namespace Bear {
namespace Core
{

//XiongWanPing 2018.08.02
class CORE_EXPORT JsonHelper
{
public:
	NameValue mParams;
	string mHeader;
	string mTail;
	string GetJsonText()
	{
		string ack = mHeader;
		int idx = -1;
		auto& items = mParams.GetItemsX();
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			++idx;
			if (idx > 0)
			{
				ack += ",\r\n";
			}

			ack += Core::StringTool::Format("\"%s\": \"%s\"", iter->first.c_str(), iter->second.c_str());
		}

		ack += "\r\n";

		ack += mTail;
		return ack;
	}

	string GetJsonTextNoCRLF()
	{
		string ack = mHeader;
		int idx = -1;
		auto& items = mParams.GetItemsX();
		for (auto iter = items.begin(); iter != items.end(); ++iter)
		{
			++idx;
			if (idx > 0)
			{
				ack += ",";// \r\n";
			}

			ack += Core::StringTool::Format("\"%s\": \"%s\"", iter->first.c_str(), iter->second.c_str());
		}

		//ack += "\r\n";

		ack += mTail;
		return ack;
	}
};

}
}
