#include "common.h"


std::wstring get_wstring(const char* cs)
{
	const size_t wn = mbsrtowcs(NULL, &cs, 0, NULL);

	if (wn == size_t(-1))
	{
		std::cout << "Error in mbsrtowcs(): " << errno << std::endl;
		return L"";
	}

	std::vector<wchar_t> buf(wn + 1);
	const size_t wn_again = mbsrtowcs(&buf[0], &cs, wn + 1, NULL);

	if (wn_again == size_t(-1))
	{
		std::cout << "Error in mbsrtowcs(): " << errno << std::endl;
		return L"";
	}

	assert(cs == NULL); // successful conversion

	return std::wstring(&buf[0], wn);
}