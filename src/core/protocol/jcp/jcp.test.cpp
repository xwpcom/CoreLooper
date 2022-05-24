#include "stdafx.h"

#ifdef _MSC_VER
#include "CppUnitTest.h"
#include "core/protocol/jcp/jcp.client.h"
#include "core/protocol/jcp/jcp.protocol.h"

using namespace Bear::Core;
using namespace Bear::Core::FileSystem;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace Bear {
namespace Jcp {

static const char* TAG = "jco";

TEST_CLASS(JcpTests)
{
public:
	TEST_METHOD(jcpDemo)
	{
		class MainLooper :public MainLooper_
		{
			long mTimer_checkCreateClient = 0;
			weak_ptr<JcpClient> mClient;

			void OnCreate()
			{
				__super::OnCreate();

				SetTimer(mTimer_checkCreateClient, 2000);
			}

			void CheckCreateClient()
			{
				auto obj = mClient.lock();
				if (obj)
				{
					return;
				}

				obj = make_shared<JcpClient>();
				AddChild(obj);
				mClient = obj;

				Bundle bundle;
				bundle.Set("address", "127.0.0.1");
				bundle.Set("port", 8081);
				obj->StartConnect(bundle);
			}

			void OnTimer(long id)
			{
				if (id == mTimer_checkCreateClient)
				{
					CheckCreateClient();

					return;
				}

				__super::OnTimer(id);

			}
		};

		make_shared<MainLooper>()->StartRun();

	}
};

}
}
#endif
