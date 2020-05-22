#pragma once

class MainLooper :public Bear::Core::MainLooper_
{
	SUPER(MainLooper_)
public:
	MainLooper();

protected:
	void OnCreate();

};
