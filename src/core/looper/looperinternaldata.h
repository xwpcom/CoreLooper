#pragma once
namespace Bear {
namespace Core
{
class LooperImpl;
struct tagLooperInternalData
{
	tagLooperInternalData(LooperImpl*);
	~tagLooperInternalData();
	void gc();

	LooperImpl			*mLooperImpl=nullptr;
	bool				mLooperRunning = false;//looper��������ܶ࣬����û�б�Ҫ��bit field
	bool				mAttachThread = false;//������_WorkThreadCB�̣߳�����attach�������߳���
	bool				mBMQuit = false;//�Ƿ����յ�BM_QUIT��Ϣ
	std::shared_ptr<Event>	mExitEvent;//�˳��߳�ʱ�ɴ������¼�

	CriticalSection					mMessageLock;
	std::list<tagLoopMessageInternal> 	mMessageList;

#ifdef _CONFIG_CALC_EVER_MAX_SIZE
	//ͳ���������ֹ��������Ϣ��������������ϵͳ����
	long mCurrentMessages;
	long mEverMaxMessages;
#endif

	std::vector<std::shared_ptr<Event>>	mExitEvents;
	std::weak_ptr<Looper>		mOwnerLooper;
	long						mExitCode = 0;
	long						mTimerCheckQuitLooper = 0;
	LONGLONG					mTickStartQuit = 0;//���������looper����ò������˳�

	//private:
	std::map<void*, std::shared_ptr<Handler>> mDestroyedHandlers;
	long	mTimerGC = 0;
};

}
}
