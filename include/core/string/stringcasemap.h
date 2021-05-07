#pragma once
#include <map>
#include <string>

namespace Bear {
namespace Core
{
using namespace std;

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

struct StrCaseCompare {
	bool operator()(const string& __x, const string& __y) const {
		return strcasecmp(__x.data(), __y.data()) < 0;
	}
};

//XiongWanPing 2019.10.16
//from zlmediakit
//key大小写不敏感map,比如用来存储http,rtsp header fields
//应用场合:websocket协议规定upgrade时header字段是大小写不敏感

/*
2021.05.07,发现君正gcc4.7.2不支持Super::emplace
*/

#ifndef _CONFIG_INGENIC
class StringCaseMap : public multimap<string, string, StrCaseCompare> 
{
public:
	typedef multimap<string, string, StrCaseCompare> Super;
	StringCaseMap() = default;
	~StringCaseMap() = default;

	template <class K>
	string& operator[](K&& k) 
	{
		auto it = find(std::forward<K>(k));
		if (it == end()) 
		{
			it = Super::emplace(std::forward<K>(k), "");
		}
		return it->second;
	}

	template <class K, class V>
	void emplace(K&& k, V&& v) 
	{
		auto it = find(std::forward<K>(k));
		if (it != end()) 
		{
			it->second = v;
			return;
		}
		Super::emplace(std::forward<K>(k), std::forward<V>(v));
	}

	template <class K, class V>
	void emplace_force(K&& k, V&& v) 
	{
		Super::emplace(std::forward<K>(k), std::forward<V>(v));
	}
};

#endif

}
}

