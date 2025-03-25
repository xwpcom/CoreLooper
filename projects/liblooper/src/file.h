#pragma once

namespace Core {

class File
{
public:
	static void pathMakePretty(char* szFile);
	static void pathMakePretty(std::string& filePath);
	static string parentFolder(const string& szFile);

};
}