#pragma once
#include <iostream>

namespace Utils
{
	int randRange(int low, int high)
	{
		return (rand() % (high - low + 1)) + low;
	}
}
