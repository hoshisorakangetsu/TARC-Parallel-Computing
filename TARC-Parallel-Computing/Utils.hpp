#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <sstream>

namespace Utils
{
	int randRange(int low, int high)
	{
		return (rand() % (high - low + 1)) + low;
	};

	double randZeroOne()
	{
		return ((double)rand()) / RAND_MAX;
	}

	template <typename T>
	std::vector<T> readCsv(std::string fileName, std::function<T(std::vector<std::string>&)> parseRowFunc)
	{
		std::string row;
		std::ifstream fileReader(fileName);
		std::vector<std::string> rowVec;
		std::vector<T> resultVec;
		while (std::getline(fileReader, row))
		{
			std::stringstream rowStream(row);
			rowVec.clear();
			std::string temp;
			while (std::getline(rowStream, temp, ','))
			{
				rowVec.push_back(temp);
			}
			resultVec.push_back(parseRowFunc(rowVec));
		}

		return resultVec;
	};
}
