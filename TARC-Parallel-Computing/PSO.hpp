#pragma once
#include <vector>
#include <string>
#include <functional>
#include <float.h>
#include <omp.h>
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
		double totalDistance = DBL_MAX;
		Route foundedRoute, offsetRoute;
	public:
		Particle() {};
		Particle(Route nodes, FitnessFunc fitness) : fit(fitness)
		{
			for (int i = nodes.size(); i > 0; i--)
			{
				int j = Utils::randRange(0, nodes.size() - 1);
				foundedRoute.push_back(nodes[j]);
				offsetRoute.push_back(nodes[j]);
				nodes.erase(nodes.begin() + j); // j starts from 0, so if it's 0, it still removes first element
			}
			totalDistance = getTotalDistance();
		};
		// swap founded route to best route (get the swap sequence needed to go from founded route to best route)
		// performs the swap inline, so no returns
		void minus(Route bestRoutes)
		{
			SwapSequence ss;
			// make copy of the current route
			Route currentRoute = foundedRoute;

			int index;

			// make current route follow best route
			for (int i = 0; i < foundedRoute.size(); i++)
			{
				if (currentRoute.at(i).id != bestRoutes.at(i).id)
				{
					index = findIndex(currentRoute, bestRoutes.at(i).id);
					swapIndex(currentRoute, i, index);
					SwapOperator so(currentRoute.at(i).id, currentRoute.at(index).id);
					ss.push_back(so);
				}
			}
			int chosenLength = (int)(ss.size() * Utils::randZeroOne());
			SwapSequence selectedSwapOperators(ss.begin(), ss.begin() + chosenLength);
			swap(foundedRoute, selectedSwapOperators);
		};
		void minusOffset(SwapSequence& ss)
		{
			//Vid to change offsetRoute
			swap(offsetRoute, ss);
			//alpha*(Pid-Xid), move toward personal best aka offset
			minus(offsetRoute);
		}
		double getBestTotalDistance() { return totalDistance; }
		const Route& getFoundedRoute() { return foundedRoute; }
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

			totalDistance = totalDist;

			return totalDist;
		};
	private:
		void swapIndex(Route& nodes, int source, int target)
		{
			Node temp = nodes[source];
			nodes[source] = nodes[target];
			nodes[target] = temp;
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
	SwapOperator swapOperator(Route& nodes)
	{
		int source = -1;
		int target = -1;

		do
		{
			source = nodes[Utils::randRange(0, nodes.size() - 1)].id;
			target = nodes[Utils::randRange(0, nodes.size() - 1)].id;
		} while (source == target);

		return SwapOperator(source, target);
	};

	class PSOSolution
	{
	public:
		Route globalBestRoute;
		double globalBestDistance = DBL_MAX;
	};

	PSOSolution Solve(int numOfParticle, int maxItr, Route nodes, FitnessFunc fit, int numThreads = 1)
	{
		std::vector<Particle> particles;
		PSOSolution solution;

		// To introduce randomness, velocity = new swap sequence = swap sequence + globalRandomSwapSequence[0:random]
		SwapSequence globalRandomSwapSequence;
		for (int i = 0; i < nodes.size(); i++)
			globalRandomSwapSequence.push_back(swapOperator(nodes));

		// Set the number of threads for OpenMP
		omp_set_num_threads(std::min(omp_get_max_threads(), numThreads));

		// Initialize particles in parallel
		#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < numOfParticle; i++)
		{
			auto temp = Particle(nodes, fit);

			#pragma omp critical // Ensure that updating the global best solution is done sequentially
			{
				if (temp.getBestTotalDistance() < solution.globalBestDistance)
				{
					solution.globalBestDistance = temp.getBestTotalDistance();
					solution.globalBestRoute = temp.getFoundedRoute();
				}
			}
			#pragma omp critical
			{
				particles.push_back(temp);
			}
		}

		// Main iteration loop, parallelizing the particle updates
		for (int currItr = 0; currItr < maxItr; currItr++)
		{
			#pragma omp parallel for schedule(dynamic)
			for (int i = 0; i < particles.size(); i++)
			{
				Particle& p = particles[i];

				p.minus(solution.globalBestRoute);

				SwapSequence velocity;
				// From which element to take from the global random swap sequence
				int lenToTake = Utils::randRange(0, globalRandomSwapSequence.size() - 1);
				// Take until which element
				int takeHowMuch = Utils::randRange(1, globalRandomSwapSequence.size() - lenToTake);
				for (int j = lenToTake; j < lenToTake + takeHowMuch; j++)
				{
					velocity.push_back(globalRandomSwapSequence[j]);
				}

				p.minusOffset(velocity);

				double totalDistance = p.getTotalDistance();

				#pragma omp critical
				{
					if (totalDistance < solution.globalBestDistance)
					{
						solution.globalBestDistance = totalDistance;
						solution.globalBestRoute = p.getFoundedRoute();
					}
				}
			}


		}

		return solution;

	};
}
