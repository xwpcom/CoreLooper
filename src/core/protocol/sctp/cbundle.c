#include "protocol/sctp/cbundle.h"
#include "protocol/sctp/tool.h"
#include <stdlib.h>
//when item NOT exists,reutrn "";
//never return NULL,string sz=NULL will crash
const char* Bundle_GetString(tagBundle* obj, const char *name)
{
	int i;
	for (i = 0; i < obj->mCount; ++i)
	{
		if (strcmp(obj->mItems[i].name, name) == 0)
		{
			if (obj->mItems[i].value)
			{
				return obj->mItems[i].value;
			}

			return "";
		}
	}

	return "";
}

int Bundle_GetInt(tagBundle* obj, const char *name)
{
	const char* value = Bundle_GetString(obj, name);
	if (value[0])
	{
		return atoi(value);
	}
	
	return 0;
}

unsigned int Bundle_GetUInt(tagBundle* obj, const char* name)
{
	const char* value = Bundle_GetString(obj, name);
	if (value[0])
	{
		return (unsigned int)atoi(value);
	}

	return 0;
}

unsigned char Bundle_GetByte(tagBundle* obj, const char *name)
{
	return (unsigned char)Bundle_GetInt(obj, name);
}

void Bundle_clear(tagBundle* obj)
{
	obj->mCount = 0;
}

void Bundle_Create(tagBundle* obj, tagKeyValue *items, unsigned char totalCount)
{
	{
		int i;
		for (i = 0; i < totalCount; ++i)
		{
			items[i].name = 0;
			items[i].value = 0;
		}
	}

	obj->mItems = items;
	obj->mCount = 0;
	obj->mTotalCount= totalCount;
}

int Bundle_SetString(tagBundle* obj, const char* name, const char* value)
{
	for (int i = 0; i < obj->mCount; i++)
	{
		tagKeyValue* item = &obj->mItems[i];
		if (strcmp(name, item->name) == 0)
		{
			item->value = value;
			return 0;
		}
	}

	return Bundle_Push(obj, name, value);
}

int Bundle_Push(tagBundle* obj, const char *name, const char *value)
{
	if (obj->mCount >= obj->mTotalCount)
	{
		return -1;
	}

	obj->mItems[obj->mCount].name = name;
	obj->mItems[obj->mCount].value = value;
	++obj->mCount;

	return 0;
}

int Bundle_Exists(tagBundle* obj, const char *name)
{
	int i;
	for (i = 0; i < obj->mCount; ++i)
	{
		if (strcmp(obj->mItems[i].name, name) == 0)
		{
			return 1;
		}
	}

	return 0;
}
