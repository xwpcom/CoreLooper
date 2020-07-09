#pragma once

namespace Bear {
namespace Core {
namespace Net {
namespace Http {

//XiongWanPing 2018.07.26
//解析http chunk块数据
class HTTP_EXPORT HttpChunkParser
{
public:
	HttpChunkParser();
	virtual ~HttpChunkParser();

	int Parse(const string& httpAck, ByteBuffer& box);
};

}
}
}
}
