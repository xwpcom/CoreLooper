#pragma once
namespace Bear {
namespace Core {
namespace Net {
namespace Ftp {
//XiongWanPing 2018.05.22
class DataSource :public Handler
{
	SUPER(Handler)
public:

	DataSource();
	~DataSource();

	//返回成功读取的字节数
	//返回==0表示end,
	//返回<0数据表示当前无可用数据
	virtual long Read(LPBYTE buf, long bytes) = 0;

	sigslot::signal1<DataSource*> SignalDataChanged;//数据有变化时触发
};

}
}
}
}