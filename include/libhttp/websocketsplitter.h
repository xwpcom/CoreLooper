﻿#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
using namespace std;


namespace Bear {
namespace Core {
namespace Net {
namespace Http {
class HTTP_EXPORT WebSocketHeader {
public:
	typedef std::shared_ptr<WebSocketHeader> Ptr;
	typedef enum {
		CONTINUATION = 0x0,
		TEXT = 0x1,
		BINARY = 0x2,
		RSV3 = 0x3,
		RSV4 = 0x4,
		RSV5 = 0x5,
		RSV6 = 0x6,
		RSV7 = 0x7,
		CLOSE = 0x8,
		PING = 0x9,
		PONG = 0xA,
		CONTROL_RSVB = 0xB,
		CONTROL_RSVC = 0xC,
		CONTROL_RSVD = 0xD,
		CONTROL_RSVE = 0xE,
		CONTROL_RSVF = 0xF
	} Type;
public:
	WebSocketHeader() : _mask(4) {}
	virtual ~WebSocketHeader() {}
public:
	bool _fin;
	uint8_t _reserved;
	Type _opcode;
	bool _mask_flag;
	uint64_t _playload_len;
	vector<uint8_t > _mask;
};

//FROM:github zlmediakit xiongziliang <771730766@qq.com>
class HTTP_EXPORT WebSocketSplitter : public WebSocketHeader {
public:
	WebSocketSplitter() {}
	virtual ~WebSocketSplitter() {}

	/**
	 * 输入数据以便解包webSocket数据以及处理粘包问题
	 * 可能触发onWebSocketDecodeHeader和onWebSocketDecodePlayload回调
	 * @param data 需要解包的数据，可能是不完整的包或多个包
	 * @param len 数据长度
	 */
	void decode(uint8_t* data, uint64_t len);

	/**
	 * 编码一个数据包
	 * 将触发2次onWebSocketEncodeData回调
	 * 第一次是数据头，第二次是负载数据
	 * @param header 数据头
	 * @param data 负载数据
	 * @param len 负载数据长度
	 */
	void encode(const WebSocketHeader& header, uint8_t* data, const uint64_t len);
protected:
	/**
	 * 收到一个webSocket数据包包头，后续将继续触发onWebSocketDecodePlayload回调
	 * @param header 数据包头
	 */
	virtual void onWebSocketDecodeHeader(const WebSocketHeader& header) {};

	/**
	 * 收到webSocket数据包负载
	 * @param header 数据包包头
	 * @param ptr 负载数据指针
	 * @param len 负载数据长度
	 * @param recved 已接收数据长度(包含本次数据长度)，等于header._playload_len时则接受完毕
	 */
	virtual void onWebSocketDecodePlayload(const WebSocketHeader& header, const uint8_t* ptr, uint64_t len, uint64_t recved) {};


	/**
	 * 接收到完整的一个webSocket数据包后回调
	 * @param header 数据包包头
	 */
	virtual void onWebSocketDecodeComplete(const WebSocketHeader& header) {};

	/**
	 * websocket数据编码回调
	 * @param ptr 数据指针
	 * @param len 数据指针长度
	 */
	virtual void onWebSocketEncodeData(const uint8_t* ptr, uint64_t len) {};
private:
	void onPlayloadData(uint8_t* data, uint64_t len);
private:
	string _remain_data;
	int _mask_offset = 0;
	bool _got_header = false;
	uint64_t _playload_offset = 0;
};


}
}
}
}

