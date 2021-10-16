#ifndef BIMAXPROCESSING_HELPERS_H
#define BIMAXPROCESSING_HELPERS_H

#pragma once
#include <combaseapi.h>
#include <regex>

namespace helpers
{
	char* StringToCStr(std::string str);

	std::string rtrim(const std::string& s);

	std::string ltrim(const std::string& s);

	std::string trim(const std::string& s);

	char* GetAppDataTempPath();
}

#endif