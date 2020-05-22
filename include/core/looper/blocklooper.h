#pragma once
#include "looper.h"
namespace Bear {
namespace Core
{

class BlockLooper :public Looper
{
	SUPER(Looper)
public:
	BlockLooper()
	{
		mThreadName = "BlockLooper";
	}

protected:
	virtual bool MaybeLongBlock()const
	{
		return true;
	}

	void OnCreate()
	{
		__super::OnCreate();

		if (mEnableProxy)
		{
			auto parent = GetParent();
			if (parent)
			{
				//主要是为方便调试，在mainlooper中感知有block looper存在
				auto proxy = std::make_shared<Handler>();
				proxy->SetObjectName(GetObjectName() + ".BlockLooper");
				parent->AddChild(proxy);
				mProxyBuddy = proxy;
			}
		}
	}

	std::shared_ptr<Handler> mProxyBuddy;
	bool mEnableProxy = false;
};
}
}