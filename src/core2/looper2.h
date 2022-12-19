#pragma once
#include "handler2.h"

namespace Bear {
namespace Core2 {

class Looper:public Handler
{
public:
	Looper();

	virtual void asyncRun();
	virtual void syncRun();
};

}
}
