#pragma once
#include "handler.h"

#define _CONFIG_MONITOR_HANDLER	//��������object leaks

namespace Bear {
namespace Core
{
using std::unordered_map;
class Loop;
struct tagHandlerInternalData
{
	tagHandlerInternalData(Handler* handler);
	~tagHandlerInternalData();
	
	//ʲôʱ����Ҫ���ñ��ӿ�:
	//��һ��object����û��δ��ҵ��ĸ��������childҲ����û�У�����Ҫ���ñ��ӿ�������
	//���������ٲ���һ��shared_ptr������object
	//active object��ָ��δ��ҵ��Ķ��󣬲��ҵ�����ҵ�����ʱ�Զ�����
	void SetActiveObject()
	{
		mPassive = false;
	}

	string LOOPER_SAFE GetName()const;
	void LOOPER_SAFE SetName(const string& name);

	long SetTimer(UINT interval);
	void KillTimer(long& timerId);

	virtual long SetTimerEx(UINT interval, shared_ptr<tagTimerExtraInfo> info = nullptr);
	long GetLiveChildrenCount();
	virtual int AddChildHelper(weak_ptr<Handler> child, string name);
	shared_ptr<Handler> FindObject_Impl(const string& url);
	shared_ptr<Handler> GetChild_Impl(LONG_PTR id);
	shared_ptr<Handler> Shortcut_Impl(const string& name);
	int RegisterShortcut_Impl(const string& name,weak_ptr<Handler>obj);
	long GetChildCount()const;
	void RemoveChildWeakRef(Handler *);
	void RemoveChildWeakRef_Impl(Handler *handler);
	virtual void Dump(int level, bool includingChild = true);
	void OnPrepareDestructor();

	shared_ptr<Looper> mLooper;//����Create�ɹ�֮�����Ч������Ϊnullptr
	Handler* mHandler = nullptr;
	string mObjectName;
	LONGLONG mTickDestroy = 0;//���������handler���յ�OnDestroy()��������ʱ��

	shared_ptr<Handler> mSelfRef;	//ȷ��������ʱthisָ����Ч,����looper�ͱ�����object
	unordered_map<long*, weak_ptr<Handler>> mChildren;
	shared_ptr<unordered_map<string, weak_ptr<Handler>>> mShortcuts;

	shared_ptr<Handler> mParent;
	LONG_PTR mId = 0;//����������ض�handler,���ߴ��context��Ϣ
	bool mPassive : 1;// = true;//������object,��������ָ�ɶ����ṩ���񣬵�û��δ��ҵ���������,��Ҫ����mSelfRef����
					  //������object��Ҫ����AddChild����parent��������յ�BM_DESTROY������
					  //��������AddChild�ǿ�ѡ��,ҵ�����ʱ�����Զ�����
	bool mCreated : 1;// = false;//c++11��֧��,c++20��֧��
	bool mOnCreateCalled : 1;//=false
	bool mDestroy : 1;// = false;
	bool mDestroyMarkCalled : 1;
	bool mOnDestroyCalled : 1;//=false
	bool mIsLooper : 1;// = false;
	bool mMaybeLongBlock : 1;//=false
	bool mTimerIdRewind : 1;//��mNextTimerId���ƺ������mNextTimerIdʱҪ����ͻ
	long mNextTimerId = 0;

	shared_ptr<unordered_map<long, shared_ptr<tagTimerNode>>> mTimerMap;
	long NextTimerId();
	void RemoveAllTimer();

	shared_ptr<TimerManager> mTimerManager;//������timerʱҪ����looper�е�TimerManager,��Ϊ�˱�֤������ʱTimerManager����Ч��

	void TestContestSleep();

#ifdef _CONFIG_MONITOR_HANDLER
	static void DumpAll();
	static int  GetHandlerCount();
	static void SetRationalHandlerUpperLimit(int count);

	static CriticalSection gCSBaseHandler;
	static unordered_map<long*, long*> gHandlers;
	static int gRationalHandlerUpperLimit;// 1000;
#endif

	unordered_map<UINT, Handler::PFN_OnMessage> mMessageEntries;
	UINT mNextAllocMessageId = BM_ALLOC_MESSAGE_ID_BASE;
};

}
}
