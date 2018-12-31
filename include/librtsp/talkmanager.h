#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Rtsp {

class RtspServerHandler;
class Bear::Core::Media::AudioRender;

//XiongWanPing 2016.02.17
//设备端的对讲管理器
class RTSP_CLASS TalkManager :public Handler
{
	SUPER(Handler);
public:
	TalkManager();
	~TalkManager();

	int RequestTalkId(std::shared_ptr<RtspServerHandler> handler);
	int OnRecvAudio(int talkId, LPBYTE audio, int audioBytes);
	void StopTalk(std::shared_ptr<RtspServerHandler> handler, int talkId);
protected:
	void OnCreate();

	int mNextTalkId;
	ByteBuffer mBox;

	bool mSingleTalk = true;//同一时刻只支持一路对讲(mtk6572支持多个)
	std::shared_ptr<Bear::Core::Media::AudioRender> mSingleAudioRender;//仅在mSingleTalk为true时使用
	std::weak_ptr<RtspServerHandler> mTalkHandler;

};

}
}
}
}
