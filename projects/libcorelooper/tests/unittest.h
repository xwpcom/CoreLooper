#pragma once
using namespace std;
class TestHandler :public Bear::Core::Handler
{
public:
	//test fail,can not deduce type T
	template<class T> std::shared_ptr<T> ShortcutEx(
		//std::shared_ptr<T> & sp, 
		const string& name)
	{
		//std::shared_ptr<Handler> Handler::Shortcut(const string& name)
		auto obj = dynamic_pointer_cast<T>(Shortcut(name));
		return obj;
	}
};
