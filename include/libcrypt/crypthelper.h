#pragma once
namespace Bear {
namespace Core
{

//crypt helper for wuboss platform
//XiongWanPing 2014.08.29
class CRYPT_EXT_CLASS CryptHelper
{
public:
	static std::string  Encode(std::string  plainText);
	static std::string  Decode(std::string  encodedText);
};
}
}