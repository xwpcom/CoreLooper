#include "stdafx.h"
#include "mime.h"

namespace Bear {
namespace Core {
namespace Net {
namespace Http {
using namespace Core;

string Mime::GetFileContentType(string filePath)
{
	auto ext = File::GetFileExt(filePath.c_str());
	static struct tagExtType
	{
		const char* ext;
		const char* type;
	}arr[] =
	{
		".jpg",		"image/jpeg",
		".jpeg",	"image/jpeg",
		".png",		"image/png",
	};

	for (auto i = 0; i < COUNT_OF(arr); ++i)
	{
		if (StringTool::CompareNoCase(arr[i].ext, ext) == 0)
		{
			return arr[i].type;
		}
	}

	return "application/octet-stream";
}

}
}
}
}
