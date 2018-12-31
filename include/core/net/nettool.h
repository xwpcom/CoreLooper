#pragma once
namespace Bear {
namespace Core
{
namespace Net {

class CORE_EXPORT NetTool
{
public:
	static bool IsValidMac(const char *mac);
	static std::string GetLanIP(std::string eth = GetInterfaceName());
	static std::string GetNetSection(std::string ip, std::string mask);
	static std::string GetMac(std::string eth = GetInterfaceName());
	static std::string GetGateWay(std::string ifName = GetInterfaceName());
	static std::string GetInterfaceName();
	static std::vector<std::string> GetMacs();
	static bool MacExists(const std::string& mac);
};
}
}
}