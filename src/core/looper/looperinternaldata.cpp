#include "stdafx.h"
#include "handlerinternaldata.h"
#include "looperinternaldata.h"
#include "looper.h"
#include "core/looper/teststate.h"

using namespace std;

namespace Bear {
namespace Core
{

tagLooperInternalData::tagLooperInternalData(LooperImpl* looper)
{
	mLooperImpl = looper;
	mBMQuit = false;
	mLooperRunning = false;
	mAttachThread = false;
}

tagLooperInternalData::~tagLooperInternalData()
{

}
//�����ʱ��Handlers
//������:
//��weak_ptr����mDestroyedHandlers�л����shared_ptr
//���mDestroyedHandlers�е�shared_ptr
//Ȼ����weak_ptr.lock(),���lockΪnull��ʾhandler������������Ҫ������ӵ�mDestroyedHandlers��
void tagLooperInternalData::gc()
{
	//ASSERT(IsMyselfThread());
	++mSeqGC;

	auto& items = mDestroyedHandlers;
	for (auto iter = items.begin(); iter != items.end();)
	{
		weak_ptr<Handler> handler = iter->second;
		{
			void *key = iter->first;
			{
				auto item = iter;
				++iter;

				if (item->second.use_count() != 1)//����������looper�о���Ҳû�й�ϵ�������lock�������ж�
				{
					continue;
				}
			}

			//XiongWanPing 2018.07.27
			//�����о�����ϵ�����ܵ���������thread������handler,��������:
			//�������use_count=1ʱ���е��ˣ�Ȼ��cpu�л�����һthread,����һthread��weakptr lock�õ�shared_ptr
			//Ȼ���л�����threadִ��items[key] = nullptr;
			//Ȼ�����л�����һthread����shared_ptr�����лص���thread
			//�о�����ʱ��ֻ�������ϴ��ڣ�ʵ���ϻ�����������
			//��test state���⹹��ʱ��
#ifdef _CONFIG_TEST_CROSS_LOOPER_WEAK_PTR_LOCK
			LooperImpl::SetTestState(eTestState_0);

			//��TestLooper weak_ptr.lock()
			//Ȼ����TestLooper��SetTestState(eTestState_1)
			while (LooperImpl::GetTestState() == eTestState_0)
			{
				ShellTool::Sleep(1);
			}
#endif
			
			auto handlerData = items[key]->mInternalData;//��֤handler data��ԭ��looper������
			items[key] = nullptr;//���û���ⲿ���ã���ʱhandler������

#ifdef _CONFIG_TEST_CROSS_LOOPER_WEAK_PTR_LOCK
			LooperImpl::SetTestState(eTestState_2);
			while (LooperImpl::GetTestState() == eTestState_2)
			{
				ShellTool::Sleep(1);
			}
#endif
			auto obj = handler.lock();
			if (obj)
			{
				//���ⲿ����
				items[key] = obj;
				obj->OnPrepareDestructor();
			}
			else
			{
				//handler������
				items.erase(key);
				handlerData->mHandler = nullptr;
				handlerData->RemoveAllTimer();//��ʽ�������timer
			}
		}
	}

	//DW("this=%p,gc size=%d",this, items.size());
	if (items.size() == 0)
	{
		mLooperImpl->KillTimer(mTimerGC);
		mSeqGC = 0;
	}
}


}
}
