#pragma once

//https://www.codeproject.com/Articles/3651/Change-Internet-Proxy-settings?msg=1068644#xx1068644xx
//XiongWanPing 2022.08.01
class WIN_CLASS IEHttpProxy
{
public:
	static int enableProxy();
	static BOOL SetConnectionOptions(LPWSTR proxy_full_addr, LPWSTR conn_name=_T(""));
	static BOOL DisableConnectionProxy(LPWSTR conn_name = _T(""));
};