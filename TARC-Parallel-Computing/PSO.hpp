#pragma once
#include <vector>
#include <string>
#include <functional>
#include <float.h>
#include "Utils.hpp"

namespace PSO {

	class Node
	{
	public:
		int id;
		std::string latitude;
		std::string longitude;
		Node(int id, std::string lat, std::string longi) : id(id), latitude(lat), longitude(longi) {};
		std::string toString()
		{
			return "Node (" + std::to_string(id) + "): { lat: " + latitude + ", long: " + longitude + " }";
		};
	};


	class SwapOperator
	{
	public:
		int sourceId;
		int targetId;
		SwapOperator() : sourceId(0), targetId(0) {};
		SwapOperator(int source, int target) : sourceId(source), targetId(target) {};
		std::string toString()
		{
			return "SO{ source: " + std::to_string(sourceId) + ", target: " + std::to_string(targetId) + " }";
		}
	};

	typedef std::function<double(Node, Node)> FitnessFunc;

	typedef std::function<std::pair<double, double>(Node)> ConvertionFunc;

	typedef std::vector<Node> Route;

	typedef std::vector<SwapOperator> SwapSequence;

	class Particle
	{
	private:
		FitnessFunc fit;
		double bestTotalDistance = DBL_MAX;
		Route foundedRoute, offsetRoute;
	public:
		Particle() {};
		Particle(Route nodes, FitnessFunc fitness) : fit(fitness)
		{
			srand(0);
			for (int i = nodes.size(); i > 0; i--)
			{
				int j = Utils::randRange(0, nodes.size() - 1);
				foundedRoute.push_back(nodes[j]);
				offsetRoute.push_back(nodes[j]);
				nodes.erase(nodes.begin() + j); // j starts from 0, so if it's 0, it still removes first element
			}
			bestTotalDistance = getTotalDistance();
		};
		// swap founded route to best route (get the swap sequence needed to go from founded route to best route)
		// performs the swap inline, so no returns
		void minus(Route& bestRoutes)
		{
			SwapSequence ss;
			// make copy of the current route
			Route currentRoute;
			for (Node n : foundedRoute)
			{
				currentRoute.push_back(n);
			}

			int index;

			// make current route follow best route
			for (int i = 0; i < foundedRoute.size(); i++)
			{
				if (currentRoute[i].id != bestRoutes[i].id)
				{
					index = findIndex(currentRoute, bestRoutes[i].id);
					swapIndex(currentRoute, i, index);
					SwapOperator so(currentRoute[i].id, currentRoute[index].id);
					ss.push_back(so);
				}
			}
			int chosenLength = (int)(ss.size() * Utils::randZeroOne());
			SwapSequence selectedSwapOperators(ss.begin(), ss.begin() + chosenLength);
			swap(foundedRoute, selectedSwapOperators);
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
	private:
		void swapIndex(Route& nodes, int source, int target)
		{
			Node temp = nodes[source];
			nodes[source] = nodes[target];
			nodes[target] = nodes[source];
		};
		void swap(Route& nodes, SwapSequence& ss)
		{
			for (SwapOperator so : ss)
			{
				int sourceIndex = findIndex(nodes, so.sourceId);
				int targetIndex = findIndex(nodes, so.targetId);
				swapIndex(nodes, sourceIndex, targetIndex);
			}
		}
		int findIndex(Route& nodes, int id)
		{
			for (int i = 0; i < nodes.size(); i++)
			{
				if (nodes[i].id == id)
				{
					return i;
				}
			}
			return -1;
		};
	};

	// randomly generates swap operator
	SwapOperator swapOperator(Route &nodes)
	{

	};

	class PSOSolution
	{
	public:
		Route globalBestRoute;
		double globalBestDistance = DBL_MAX;
	};

	PSOSolution Solve(int numOfParticle, int maxItr, Route nodes, FitnessFunc fit)
	{
		std::vector<Particle> particles;
		PSOSolution solution;

		SwapSequence globalRandomSwapSequence;
		for (int i = 0; i < nodes.size(); i++)
			swapOperator(nodes);

		// init particles
		for (int i = 0; i < numOfParticle; i++)
		{
			auto temp = Particle(nodes, fit);

			if (temp.getBestTotalDistance() < solution.globalBestDistance)
			{
				solution.globalBestDistance = temp.getBestTotalDistance();
				solution.globalBestRoute = temp.getFoundedRoute();
			}
			particles.push_back(temp);
		}

		for (int currItr = 0; currItr < maxItr; currItr++)
		{
			for (Particle p : particles)
			{
				p.minus(solution.globalBestRoute);

			}
		}

		return solution;
	};
}
