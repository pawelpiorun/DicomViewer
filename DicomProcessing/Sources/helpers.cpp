#include "helpers.h"
#include <direct.h>

namespace helpers
{
	char* StringToCStr(std::string str)
	{
		ULONG size = strlen(str.c_str()) + sizeof(char);
		char* cstr = (char*)::CoTaskMemAlloc(size);
		strcpy(cstr, str.c_str());
		return cstr;
	}

	std::string rtrim(const std::string& s)
	{
		return std::regex_replace(s, std::regex("\\s+$"), std::string(""));
	}

	std::string ltrim(const std::string& s)
	{
		return std::regex_replace(s, std::regex("^\\s+"), std::string(""));
	}

	std::string trim(const std::string& s)
	{
		return ltrim(rtrim(s));
	}


	char* GetAppDataTempPath()
	{
		auto appData = std::getenv("LOCALAPPDATA");
		char buffer[256];
		strncpy(buffer, appData, sizeof(buffer));

		char* subFolder = "\\BimaxViewer";
		strncat(buffer, subFolder, sizeof(buffer));

		auto status = _mkdir(buffer);

		char* tempFolder = "\\Temp";
		strncat(buffer, tempFolder, sizeof(buffer));

		status = _mkdir(buffer);

		return buffer;
	}
}