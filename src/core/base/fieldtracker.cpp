#include "stdafx.h"
#include "fieldtracker.h"
namespace Bear {
namespace Core
{
static const char* TAG = "FieldTracker";

FieldTracker::FieldTracker(const char* comment) :m_comment(comment)
{
	LogV(TAG, "%s#enter", m_comment);
}

FieldTracker::~FieldTracker()
{
	LogV(TAG, "%s#exit", m_comment);
}

}
}
