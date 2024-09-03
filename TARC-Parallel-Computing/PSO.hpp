#pragma once
#include <vector>
#include <string>
#include <format>
#include <functional>
#include "Utils.hpp"

namespace PSO {
	class Particle
	{
	private:
		FitnessFunc fit;
		double bestTotalDistance;
		Route foundedRoute, offsetRoute;
	public:
		Particle(Route nodes, FitnessFunc fitness): fit(fitness) 
		{
			srand(0);
			for (int i = nodes.size(); i > 0; i--)
			{
				int j = Utils::randRange(0, nodes.size());
				foundedRoute.push_back(nodes[j]);
				offsetRoute.push_back(nodes[j]);
				nodes.erase(nodes.begin() + j); // j starts from 0, so if it's 0, it still removes first element
			}
			bestTotalDistance = getTotalDistance();
		};
		double getBestTotalDistance() { return bestTotalDistance; }
		Route getFoundedRoute() { return foundedRoute; }
		double getTotalDistance()
		{
			double totalDist = 0;

			// sum from first to last
			for (int i = 0; i <= foundedRoute.size() - 2; i++)
			{
				totalDist += fit(foundedRoute[i], foundedRoute[i + 1]);
			}

			// sum last and first (make it a loop)
			totalDist += fit(foundedRoute[foundedRoute.size() - 1], foundedRoute[0]);

			return totalDist;
		};
	};

	class SwapOperator
	{
	public:
		int sourceIndex;
		int targetIndex;
		SwapOperator(int source, int target) : sourceIndex(source), targetIndex(target) {};
		std::string toString()
		{
			return "SO{ source: " + std::to_string(sourceIndex) + ", target: " + std::to_string(targetIndex) + " }";
		}
	};

	class Node
	{
	public:
		int id;
		std::string latitude;
		std::string longitude;
		Node(int id, std::string lat, std::string longi) : id(id), latitude(lat), longitude(longi) {};
		std::string toString()
		{
			return id + " " + latitude + "," + longitude + "\n";
		};
	};

	typedef std::function<double(Node, Node)> FitnessFunc;

	typedef std::function<std::pair<double, double>(Node)> ConvertionFunc;

	typedef std::vector<Node> Route;

	typedef std::vector<SwapOperator> SwapSequence;
}
