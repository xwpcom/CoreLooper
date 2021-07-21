#pragma once
namespace Bear {
namespace Core
{

//XiongWanPing 2012.07.05
class CORE_EXPORT  FieldTracker
{
public:
	FieldTracker(const char* comment);
	~FieldTracker();
protected:
	const char* m_comment;
};

}
}
