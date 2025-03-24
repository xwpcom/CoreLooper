#include "pch.h"
#include "timems.h"
#include "logger.h"
#include "stringtool.h"
namespace Core
{
static const char* TAG = "timems";

tagTimeMs tagTimeMs::now()
{
	#ifdef _MSC_VER
	SYSTEMTIME st;
	::GetLocalTime(&st);
	int year = st.wYear;
	int month = st.wMonth;
	int day = st.wDay;
	int hour = st.wHour;
	int minute = st.wMinute;
	int second = st.wSecond;
	int ms = st.wMilliseconds;
	#else
	struct timeval tv = { 0 };
	gettimeofday(&tv, nullptr);
	time_t time = tv.tv_sec;
	auto ms = (int)(tv.tv_usec / 1000);

	struct tm tmNow;
	localtime_r(&time, &tmNow);
	int year = tmNow.tm_year + 1900;
	int month = tmNow.tm_mon + 1;
	int day = tmNow.tm_mday;
	int hour = tmNow.tm_hour;
	int minute = tmNow.tm_min;
	int second = tmNow.tm_sec;
	#endif

	tagTimeMs obj;
	obj.year = year;
	obj.month = month;
	obj.day = day;
	obj.hour = hour;
	obj.minute = minute;
	obj.second = second;
	obj.ms = ms;
	return obj;
}

time_t tagTimeMs::to_time_t()const
{
	tm tm1 = { 0 };
	tm1.tm_year = year - 1900;
	tm1.tm_mon = month - 1;
	tm1.tm_mday = day;
	tm1.tm_hour = hour;
	tm1.tm_min = minute;
	tm1.tm_sec = second;
	time_t t1 = DateTime::mktime(&tm1);
	return t1;
}

//返回晚于obj的天数
//比如是obj的后一天，则返回1
//如果obj晚于本对象,返回天数为负数
int tagTimeMs::laterDays(const tagTimeMs& obj)
{
	tm tm1 = { 0 };
	tm1.tm_year = this->year - 1900;
	tm1.tm_mon = this->month - 1;
	tm1.tm_mday = this->day;
	//time_t t1 = DateTime::mktime(&tm1);

	tm tm2 = { 0 };
	tm2.tm_year = obj.year - 1900;
	tm2.tm_mon = obj.month - 1;
	tm2.tm_mday = obj.day;

	//time_t t2 = DateTime::mktime(&tm2);
	int days = DateTime::spanDays(tm1, tm2);
	return days;
}

//支持的几种dt样本:
// 20220716123456789,其中789是ms
// 20220716123456
// 20220716
void tagTimeMs::from_dt(const string& dt)
{
	clear();
	if (dt.length() >= strlen("20220716"))
	{
		int date = atoi(dt.substr(0, 8).c_str());
		int time = atoi(dt.substr(8, 6).c_str());
		int ms_ = atoi(dt.substr(14, 3).c_str());

		from_dt(date, time);
		ms = ms_;
	}

	if (year == 0)
	{
		String2TimeMs(dt, *this);
	}
}

void tagTimeMs::from_dt(int date, int time)
{
	year = date / 10000;
	month = (date / 100) % 100;
	day = date % 100;
	hour = time / 10000;
	minute = (time / 100) % 100;
	second = time % 100;
	ms = 0;
}

void tagTimeMs::from_time_t(time_t t)
{
	tm time = { 0 };
	DateTime::localtime(t, &time);

	year = time.tm_year + 1900;
	month = time.tm_mon + 1;
	day = time.tm_mday;
	hour = time.tm_hour;
	minute = time.tm_min;
	second = time.tm_sec;
	ms = 0;
}

/* 对接第三方平台时经常用到，标准日期时间字符串 */
string tagTimeMs::stdDateTimeText()const
{
	string text = StringTool::Format("%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
	return text;
}

string tagTimeMs::toDT()const
{
	string text = StringTool::Format("%04d%02d%02d%02d%02d%02d", year, month, day, hour, minute, second);
	return text;
}

string tagTimeMs::toText()const
{
	string text = StringTool::Format("%04d.%02d.%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
	return text;
}

int tagTimeMs::String2TimeMs(const string& text, tagTimeMs& ms)
{
	tagTimeMs& t = ms;
	int ret = sscanf(text.c_str(), "%04d.%02d.%02d %02d:%02d:%02d"
					 , &t.year, &t.month, &t.day, &t.hour, &t.minute, &t.second
	);

	if (ret == 6)
	{
		return 0;
	}

	ret = sscanf(text.c_str(), "%04d-%02d-%02d %02d:%02d:%02d"
				 , &t.year, &t.month, &t.day, &t.hour, &t.minute, &t.second
	);

	if (ret == 6)
	{
		return 0;
	}

	if (text.length() == strlen("20220416160000"))
	{
		int off = 0;
		t.year = atoi(text.substr(off, 4).c_str());
		off += 4;

		t.month = atoi(text.substr(off, 2).c_str());
		off += 2;

		t.day = atoi(text.substr(off, 2).c_str());
		off += 2;

		t.hour = atoi(text.substr(off, 2).c_str());
		off += 2;

		t.minute = atoi(text.substr(off, 2).c_str());
		off += 2;

		t.second = atoi(text.substr(off, 2).c_str());
		off += 2;

		return 0;
	}

	logV(TAG)<<__func__<<" fail, text="<<text;
	return -1;
}

static ULONG gGmtSecondOffset = 8 * 3600;//时区偏移秒数,默认为中国时区GMT+08:00
static const char mon_list[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const char leap_mon_list[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

time_t DateTime::mktime(tm* pT)
{
	const char* pDays = NULL;
	time_t tmp = 0;
	int16_t i = 0;

	pT->tm_year += 1900;
	pT->tm_mon += 1;

	//计算总共有多少个闰年
	tmp = (pT->tm_year / 4 - pT->tm_year / 100 + pT->tm_year / 400) - (1970 / 4 - 1970 / 100 + 1970 / 400);

	//如果当年是闰年，需要减去当年的闰年
	if ((pT->tm_year % 4 == 0) && ((pT->tm_year % 100 != 0) || (pT->tm_year % 400 == 0)))
	{
		tmp = tmp - 1 + (pT->tm_year - 1970) * 365;
		pDays = leap_mon_list;
	}
	else
	{
		tmp = tmp + (pT->tm_year - 1970) * 365;
		pDays = mon_list;
	}

	for (i = 0; i < pT->tm_mon - 1; i++)
		tmp += pDays[i];

	tmp = tmp + pT->tm_mday - 1;

	tmp = tmp * 24 + pT->tm_hour;

	tmp = tmp * 60 + pT->tm_min;

	tmp = tmp * 60 + pT->tm_sec;
	tmp -= gGmtSecondOffset;//减去默认加上的GMT+08:00

	return tmp;
}

/*
时间戳转为时间
tim:当前时间戳
tm *pT： 输出的时间缓冲区
*/
void DateTime::localtime(time_t tim, tm* pT)
{
	const char* pDays = NULL;

	uint16_t index = 0;

	//memset(pT, 0, sizeof(*pT));
	tim += gGmtSecondOffset;

	#if 1
	//year initialization
	if (tim >= 0x5685C180L)            // 2016-1-1 0:0:0
	{
		pT->tm_year = 2016;
		tim -= 0x5685C180L;
	}
	else if (tim >= 0x4B3D3B00L)       // 2010-1-1 0:0:0
	{
		pT->tm_year = 2010;
		tim -= 0x4B3D3B00L;
	}
	else if (tim >= 0x386D4380L)       // 2000-1-1 0:0:0
	{
		pT->tm_year = 2000;
		tim -= 0x386D4380L;
	}
	else
	{
		pT->tm_year = 1970;
	}

	//now have year
	while (tim >= 366L * 24 * 60 * 60)
	{
		if ((pT->tm_year % 4 == 0) && ((pT->tm_year % 100 != 0) || (pT->tm_year % 400 == 0)))
			tim -= 366L * 24 * 60 * 60;
		else
			tim -= 365L * 24 * 60 * 60;

		pT->tm_year++;
	}

	// then 365 * 24 * 60 * 60 < tim < 366 * 24 * 60 * 60
	if (!(((pT->tm_year % 4 == 0) && ((pT->tm_year % 100 != 0) || (pT->tm_year % 400 == 0))))
		&& (tim >= 365L * 24 * 60 * 60))
	{
		tim -= 365L * 24 * 60 * 60;
		pT->tm_year++;
	}

	// this year is a leap year?
	if (((pT->tm_year % 4 == 0) && ((pT->tm_year % 100 != 0) || (pT->tm_year % 400 == 0))))
		pDays = leap_mon_list;
	else
		pDays = mon_list;

	pT->tm_mon = 0;
	// now have mon
	while (tim >= pDays[index] * 24L * 60 * 60)
	{
		tim -= pDays[index] * 24L * 60 * 60;
		index++;
		pT->tm_mon++;
	}

	// now have days
	pT->tm_mday = (int)(tim / (24L * 60 * 60) + 1);
	tim = tim % (24L * 60 * 60);

	// now have hour
	pT->tm_hour = (int)(tim / (60 * 60));
	tim = tim % (60 * 60);

	// now have min
	pT->tm_min = (int)(tim / 60);
	tim = tim % 60;

	pT->tm_sec = (int)(tim);

	pT->tm_year -= 1900;//符合linux/windows api标准
	#endif
}

//返回tm1晚于tm2的天数
//比如tm1是tm2后一天，则返回1
//如果tm2晚于tm1,返回天数为负数
int DateTime::spanDays(const tm& tm1, const tm& tm2)
{
	tm tt1 = tm1;
	tm tt2 = tm2;
	time_t t1 = DateTime::mktime(&tt1);
	time_t t2 = DateTime::mktime(&tt2);

	int days = (int)((t1 - t2) / (3600 * 24));
	return days;
}

static time_t gBaseTime = 0;//可由外界设定

time_t DateTime::time()
{
	#ifdef _MSC_VER
	return ::time(nullptr);
	#else
	return (ShellTool::GetTickCount() / 1000) + gBaseTime;
	#endif
}

}