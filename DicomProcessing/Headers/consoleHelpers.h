#pragma once
#include <iostream>

namespace ConsoleHelpers
{
	inline void writeStart()
	{
		std::cout << std::endl << "------------------ BEGIN ------------------" << std::endl;
	}

	inline void writeEnd()
	{
		std::cout << "------------------- END -------------------" << std::endl << std::endl;
	}
}
