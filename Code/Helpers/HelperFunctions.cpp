#include "HelperFunctions.h"

#include <iostream>
#include <random>

/**
* Helper functions used in the Genetic Algorithm code
*
* Pseudo-random number generator that implements the standard C++ Mersenne Twister algorithm
*
* @param min The lower end of the range of values in the uniform distribution (inclusive)
* @param max The upper end of the range of values in the uniform distribution (inclusive)
*
* @return A uniformly distributed integer between min (inclusive) and max (inclusive)
*/
int HelperFunctions::RandomNumberGenerator(const int min, const int max)
{
	random_device rd;
	mt19937 generator(rd());
	uniform_int_distribution<> distribution(min, max);
	return distribution(generator);
}

/**
* Helper functions used in the Genetic Algorithm code.
*
* Shuffles a generic vector in place using the Mersenne Twister algorithm for the random number generator
*
* @param container The vector that needs to be shuffled
*
* @return A shuffled vector relative to the input container
*/
void HelperFunctions::ShuffleVector(vector<int>& container)
{
	random_device rd;
	mt19937 generator(rd());
	shuffle(container.begin(), container.end(), generator);
}

/**
* Helper functions used in the Genetic Algorithm code.
*
* Prints the tour in a human readable form, useful for debugging initial tours and final solution tour.
*
* @param tour The tour to be printed.
*/
void HelperFunctions::PrintTour(const vector<int> &tour)
{
	cout << "Tour: ";
	for (const int i : tour)
	{
		cout << i << " ";
	}
	cout << endl;
}

/**
* Helper functions used in the Genetic Algorithm code.
*
* Generates a random tour through all customer nodes, represented by index number.
*
* @param customerStart The index representing the first customer node. When we read all nodes from the file, index 0 = depot node, and then there are some amount of charging nodes. We only care about generating indices for customer nodes
* @param size The number of total nodes in the list of all nodes.
*
* @return A vector of node indices (ints) starting with customerStart and going to customerStart + size
*/
vector<int> HelperFunctions::GenerateRandomTour(const int customerStart, const int size)
{
	vector<int> tour(size);
	for (int i = customerStart; i < customerStart + size; i++)
	{
		tour[i - customerStart] = i;
	}
	ShuffleVector(tour);
	return tour;
}

/**
* Calculates the distance between two nodes. Each Node has an x and y coordinate, so we can find the straight line between both points. 
* 
* @param node1 The first node in the distance calculation
* @param node2 The second node in the distance calculation
* 
* @return Returns the Euclidean distance calculation between two nodes.
*/
float HelperFunctions::CalculateInterNodeDistance(const shared_ptr<Node>& node1, const shared_ptr<Node>& node2)
{
	const auto dist = static_cast<float>(hypot(node1->x - node2->x, node1->y - node2->y));
	return dist;
}

vector<int> HelperFunctions::GetIndexEncodedTour(const vector<shared_ptr<Node>>& tour)
{
	vector<int> encoded_tour;
	encoded_tour.reserve(tour.size());
    
	for(const auto &n: tour)
	{
		encoded_tour.push_back(n->index);
	}
	return encoded_tour;
}

vector<shared_ptr<Node>> HelperFunctions::GetNodeDecodedTour(const shared_ptr<ProblemDefinition>& problem, const vector<int>& tour)
{
	vector<shared_ptr<Node>> node_tour;
	node_tour.reserve(tour.size());
	
	for(const auto &i:tour)
	{
		node_tour.push_back(problem->GetAllNodes()[i]);
	}
	return node_tour;
}

vector<shared_ptr<Node>> HelperFunctions::StripNonCustomerNodesFromTour(const vector<shared_ptr<Node>>& tour)
{
	vector<shared_ptr<Node>> stripped_tour;
	for(const auto& n : tour)
	{
		if(n->node_type == Customer)
		{
			stripped_tour.push_back(n);
		}
	}
	return stripped_tour;
}

