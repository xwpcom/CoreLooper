#pragma once

class LogItem:enable_shared_from_this<LogItem>
{
public:
	string	appName;
	string	tag;
	eDTLevel	level= DT_VERBOSE;
	DWORD	pid = 0;
	DWORD	tid = 0;
	int		line = 0;
	DWORD	date = 0;
	DWORD	time=0;
	string	file;
	string	msg;

	list<shared_ptr<LogItem>> mRefs;//����ӵ�listctrlʱ������,ɾ��ʱ�ͷ�
};

class LogParser
{
public:
	static shared_ptr<LogItem> Input(const LPBYTE data,int bytes);
};
