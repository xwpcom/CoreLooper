#pragma once 
class Derive;
class Bear::Core::Looper;
class Base :public enable_shared_from_this<Base>
{
public:
	Base();
	virtual ~Base();

	weak_ptr<Derive> mObject;
	weak_ptr<Bear::Core::Looper> mLockLooper;
};

