#pragma once
#include <memory>
namespace Bear {
namespace Windows {
using namespace std;

class CORE_EXPORT UIProxy:public enable_shared_from_this<UIProxy>
{
public:
	virtual ~UIProxy() {}
};

struct tagUIRunnable
{
	shared_ptr<tagUIRunnable> mSelfRef;
	weak_ptr<UIProxy>	mProxy;
	std::function<void()> fn;
	std::function<void(const string&)> fn2;
	string body;
};

}
}
