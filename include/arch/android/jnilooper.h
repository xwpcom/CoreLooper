#pragma once
namespace Bear {
namespace Core {
//XiongWanPing 2018.07.16
//说明:
//为避免java和looper互相调用导致死锁,规定c++调用java接口全部在JniLooper中进行
//java绝不sendMessage给JniLooper
//c++其他looper要发消息给java时,通过postRunnable转给JniLooper执行
class CORE_EXPORT JniLooper :public Looper
{
	SUPER(Looper)
public:
	JniLooper();

protected:
	void OnCreate();
	void OnDestroy();
	void OnBMQuit();
};

}
}
