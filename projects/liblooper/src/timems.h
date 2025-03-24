#pragma once
namespace Core {
//time accurate to ms
struct tagTimeMs
{
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	int second = 0;
	int ms = 0;

	tagTimeMs()
	{
		clear();
	}

	void clear()
	{
		memset(this, 0, sizeof(*this));
	}

	tagTimeMs(time_t t)
	{
		from_time_t(t);
	}

	int date()const
	{
		return year * 10000 + month * 100 + day;
	}

	int time()const
	{
		return hour * 10000 + minute * 100 + second;
	}

	time_t to_time_t()const;
	void from_time_t(time_t t);
	void from_dt(int date, int time);
	void from_dt(const string& dt);

	int laterDays(const tagTimeMs& obj);
	int currentDaySeconds()
	{
		return hour * 3600 + minute * 60 + second;
	}
	static int time2Seconds(int time)
	{
		//time格式:hhmmss

		int h = time / 10000;
		int m = (time / 100) % 100;
		int s = time % 100;
		return h * 3600 + m * 60 + s;
	}

	static int String2TimeMs(const string& text, tagTimeMs& ms);
	string toText()const;
	string toDT()const;
	string stdDateTimeText()const;
	static tagTimeMs now();
	//static int setTime(const string& timeStr, const string& desc = "");
};

class DateTime
{
public:
	static time_t time();
	static time_t mktime(tm* pT);
	static void localtime(time_t tim, tm* pT);
	static int spanDays(const tm& tm1, const tm& tm2);
};

}
