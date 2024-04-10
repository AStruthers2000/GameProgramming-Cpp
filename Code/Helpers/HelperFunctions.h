#pragma once
#include "ProblemDefinition.h"

class HelperFunctions
{
public:
	static int RandomNumberGenerator(int min, int max);
	static void ShuffleVector(vector<int>& container);
	static void PrintTour(const vector<int> &tour);
	static vector<int> GenerateRandomTour(int customerStart, int size);
	static float CalculateInterNodeDistance(const shared_ptr<Node>& node1, const shared_ptr<Node>& node2);
	static vector<int> GetIndexEncodedTour(const vector<shared_ptr<Node>> &tour);
	static vector<shared_ptr<Node>> GetNodeDecodedTour(const shared_ptr<ProblemDefinition>& problem, const vector<int> &tour);
	static vector<shared_ptr<Node>> StripNonCustomerNodesFromTour(const vector<shared_ptr<Node>>& tour);

	static bool CompareNodesByDistance(const pair<shared_ptr<Node>, float>& a, const pair<shared_ptr<Node>, float>& b)
	{
		return a.second < b.second;
	}
};

