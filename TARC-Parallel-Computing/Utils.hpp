#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <sstream>
#include <chrono>

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

    class Timer
    {
    public:
        void start()
        {
            m_StartTime = std::chrono::system_clock::now();
            m_bRunning = true;
        }

        void stop()
        {
            m_EndTime = std::chrono::system_clock::now();
            m_bRunning = false;
        }

        double elapsedMilliseconds()
        {
            std::chrono::time_point<std::chrono::system_clock> endTime;

            if (m_bRunning)
            {
                endTime = std::chrono::system_clock::now();
            }
            else
            {
                endTime = m_EndTime;
            }

            return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
        }

        double elapsedSeconds()
        {
            return elapsedMilliseconds() / 1000.0;
        }

    private:
        std::chrono::time_point<std::chrono::system_clock> m_StartTime;
        std::chrono::time_point<std::chrono::system_clock> m_EndTime;
        bool m_bRunning = false;
    };
}
