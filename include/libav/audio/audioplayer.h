#pragma once

#ifdef _MSC_VER
#include <mmsystem.h>  
#endif

namespace Bear {
namespace Core {
namespace Media {

//XiongWanPing 2018.10.07
class AV_EXPORT AudioPlayer:public Handler
{
	SUPER(Handler)
public:
	AudioPlayer();

	void LOOPER_SAFE PlayFile(const string& filePath);
protected:
	void OnCreate();
	void OnDestroy();

	void PlayFile_Impl(const string& filePath);

#ifdef _MSC_VER
	map<string, MCIDEVICEID> mItems;
#endif
};

}
}
}
