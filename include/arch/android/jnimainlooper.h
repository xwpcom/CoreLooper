#pragma once

namespace Bear {
using namespace Bear::Core;
namespace Android {

/*
XiongWanPing 2019.08.30
JniMainLooper�÷�:
��java��ͨ��jni�ӿ�
.ͨ��JniMainLooper::CheckCreateJniMainLooper()������
.JniMainLooper��Ϊc++ main looper,һֱ���У����ṩ�˳�����
.java native�����ӳ�䵽c++ handler����,��handler�������JniMainLooper�У�����create,destroy
*/
class CORE_EXPORT JniMainLooper :public MainLooper_
{
	SUPER(MainLooper_);
public:
	JniMainLooper();

	static void CheckCreateJniMainLooper();
	static shared_ptr<Handler> gInstance;
protected:
	void OnCreate();
};
}
}
