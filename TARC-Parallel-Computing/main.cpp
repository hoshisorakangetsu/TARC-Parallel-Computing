#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include "Utils.hpp"
#include "PSO.hpp"

#define DEBUG 1

int main()
{
	srand(time(0));

	const std::string INPUT_FILE = "burma.csv";

	// read from csv and parse into nodes
	auto nodes = Utils::readCsv<PSO::Node>(
		INPUT_FILE,
		[](std::vector<std::string> row)
		{
			int id;
			std::cout << row[0] << " " << row[1] << " " << row[2] << std::endl;
			std::string lat = row[1], longi = row[2];

			id = std::stoi(row[0]);

			return PSO::Node(id, lat, longi);
		}
	);

#ifdef DEBUG
	for (PSO::Node n : nodes)
	{
		std::cout << n.toString() << std::endl;
	}
#endif

	// defined by tsplib for the specific problem
	PSO::ConvertionFunc convNode = [](PSO::Node n)
	{
		const double PI = 3.141592; // defined by tsplib
		double deg = trunc(std::stod(n.latitude));
		double min = std::stod(n.latitude) - deg;
		double latitude = PI * (deg + 5.0 * min / 3.0) / 180.0;
		deg = trunc(std::stod(n.longitude));
		min = std::stod(n.longitude) - deg;
		double longitude = PI * (deg + 5.0 * min / 3.0) / 180.0;

		return std::pair<double, double>(latitude, longitude);
	};

	PSO::FitnessFunc fitness = [&convNode](PSO::Node n1, PSO::Node n2)
	{
		auto nodeBegin = convNode(n1);
		auto nodeEnd = convNode(n2);
		const double RRR = 6378.388; // defined by tsplib

		// formula given by tsplib
		double q1 = cos(nodeBegin.second - nodeEnd.second);
		double q2 = cos(nodeBegin.first - nodeEnd.first);
		double q3 = cos(nodeBegin.first + nodeEnd.first);
		double dij = (int)(RRR * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);

		return dij;
	};

	Utils::Timer timer;

	timer.start();
	PSO::PSOSolution finalSolution = PSO::Solve(100, 1000, nodes, fitness);
	timer.stop();

#ifdef DEBUG
	std::cout << "Best route:" << std::endl;
	for (PSO::Node n : finalSolution.globalBestRoute)
	{
		std::cout << n.toString() << std::endl;
	}
	std::cout << "Best distance: " << finalSolution.globalBestDistance << std::endl;
	std::cout << "Time elapsed (ms): " << timer.elapsedMilliseconds() << std::endl;
#endif

	return 0;
}