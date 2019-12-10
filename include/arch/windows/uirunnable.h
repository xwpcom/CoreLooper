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
	/*
	tagUIRunnable()
	{
		DG("%s(%p)", __func__, this);
	}
	virtual ~tagUIRunnable()
	{
		DG("%s(%p)", __func__, this);
	}
	*/

	shared_ptr<tagUIRunnable> mSelfRef;
	weak_ptr<UIProxy>	mProxy;
	std::function<void()> fn;
};

}
}
