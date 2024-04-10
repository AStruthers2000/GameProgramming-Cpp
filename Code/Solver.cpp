#include "Solver.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <mutex>
#include <iostream>
#include <sstream>

#include "Algorithms/GA/GeneticAlgorithm.h"
#include "Algorithms/NEH Seeded GA/SeededGA.h"
#include "Algorithms/RandomSearch/RandomSearch.h"
#include "Helpers/ProblemDefinition.h"
#include "Helpers/HelperFunctions.h"
#include "Helpers/SolutionSet.h"
#include "Algorithms\NEH\NEH_TimeWindow.h"


mutex file_write_mutex;

/***************************************************************************//**
 * EVRP_Solver constructor handles the loading of data from a file.
 *
 * The filepath of the problem is defined in the filename variable. 
 * The data consists of rows representing a node in the graph, where each node
 * is either the depot (type = d), a charging station (type = f), or a customer 
 * node (type = c). Each node has an x and y coordinate that specify its location in 
 * the graph, as well as a demand value for customer nodes. The depot and all charging 
 * stations always have demand = 0, and there is always a charging station at the depot.
 * 
 * The datasets also include information on the vehicle fuel tank capacity, which in 
 * the case of the EVRP, this is the battery capacity. There is also information 
 * denoting the maximum inventory capacity each vehicle can hold, the fuel consumption
 * rate, the inverse refueling rate (unused) and the average velocity (unused). 
 * 
 * We populate a data structure of type EVRP_Data with a vector of all nodes, the 
 * battery capacity, the inventory capacity, the fuel consumption rate, and the index
 * representing the first customer node in the vector of all nodes. 
 * 
 * After the data is loaded into the custom EVRP_Data struct, we do a quick summation
 * to find the total demand from all customer nodes, divided by the vehicle inventory capacity,
 * to determine the true minimum number of subtours possible if the only constraint is inventory.
 ******************************************************************************/
Solver::Solver(const string &file_name)
{
	ifstream file;
	string filepath = R"(./EVRP TW/)" + file_name;
	
	file.open(filepath);
	if (!file.is_open())
	{
		cout << "Failed to open data file " << file_name << ", exiting" << endl;
		_is_good_open = false;
		return;
	}
	_current_filename = file_name;
	
	vector<shared_ptr<Node>> nodes;
	auto params = make_shared<VehicleParameters>();

	int node_index = 0;
	string line;

	//ignore the header
	getline(file, line);

	//read all nodes
	while(getline(file, line))
	{
		istringstream iss(line);
		auto node = make_shared<Node>();
		string string_id;
		char type;
		float demand;
		if(iss >> string_id >> type >> node->x >> node->y >> demand >>node->ready_time >> node->due_date >> node->service_time)
		{
			node->demand = static_cast<int>(demand);
			node->unique_id = string_id;
			switch(type)
			{
			case 'f':
				node->node_type = Charger;
				break;
			case 'c':
				node->node_type = Customer;
				break;
			case 'd':
				node->node_type = Depot;
				break;
			default: break;
			}
			node->index = node_index;
			node_index++;
			nodes.push_back(node);
		}
		else
		{
			break;
		}
	}

	//read the vehicle parameters
	while(getline(file, line))
	{
		istringstream params_stream(line);
		char identifier = line[0];

		float value = 0;
		string segment;
		vector<string> segment_list;
		while(getline(params_stream, segment, '/'))
		{
			segment_list.push_back(segment);
		}
		value = stof(segment_list[1]);
		
		switch(identifier)
		{
		case 'Q':
			params->battery_capacity = value;
			break;
		case 'C':
			params->load_capacity = static_cast<int>(value);
			break;
		case 'r':
			params->battery_consumption_rate = value;
			break;
		case 'g':
			params->inverse_recharging_rate = value;
			break;
		case 'v':
			params->average_velocity = value;
			break;
		default: cout << "Unidentified vehicle parameter" << endl; break;
		}
	}
	file.close();

	//build a nearest charger and customer neighbor relationship for each node, for ease in finding nearest customer or nearest charger
	for(auto& node : nodes)
	{
		for(auto& other : nodes)
		{
			if(node != other)
			{
				const float node_distance = HelperFunctions::CalculateInterNodeDistance(node, other);

				//i use a vector of pairs here because a map is only sorted by the key, and my structure uses the
				//value of the map as the distance maps can only have unique keys, so if I made the distance the key,
				//and two nodes had exactly the same distance from one node, the map would ignore one of the two nodes
				const auto node_relation = make_pair(other, node_distance);
				if(other->node_type == Customer)
				{
					node->relative_customers.push_back(node_relation);
				}
				else if(other->node_type == Charger)
				{
					node->relative_chargers.push_back(node_relation);
				}
			}
		}
		sort(node->relative_customers.begin(), node->relative_customers.end(), HelperFunctions::CompareNodesByDistance);
		sort(node->relative_chargers.begin(), node->relative_chargers.end(), HelperFunctions::CompareNodesByDistance);
	}

	problem_definition = make_shared<ProblemDefinition>(nodes, params);
	shared_hasher = make_shared<Hasher>();
	//delete params;
	cout << "~=~=~=~= Solving problem " << file_name << " now ~=~=~=~=" << endl;
}

void Solver::DebugEVRP() const
{
	/*
	auto *alg = new NEH_NN(problem_definition);
	solution s = {};
	alg->Optimize(s);
	
	HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(s.tour));
	cout << "Best tour has a distance of: " << s.distance << endl;
	*/


	const auto vehicle = make_unique<Vehicle>(problem_definition, shared_hasher);
	auto status = make_unique<VehicleStatus>(problem_definition->GetVehicleParameters());
	
	
	//true route is:                0 7 6 14 5 11 17 13 2 16 19 9 12 3 8 4 20 15 10 0 18 0
	//const vector<int> test_route = {0, 7, 6, 14, 11, 17, 13, 16, 19, 9, 12, 8, 20, 15, 10, 0, 18, 0};

	/*
	*   0 87 112 113 116 77 85 104 86 111 1 117 92 114 115 101 0 102 75
	*   93 62 60 4 82 89 76 91 121 109 74 10 119 90 0 78 95 80 15 98 79
	*   96 118 108 107 73 120 14 103 0 88 71 83 20 51 55 52 48 47 49 50
	*   53 54 19 84 106 0 23 27 25 66 28 100 94 11 99 81 0 72 97 110 17
	*   39 69 42 44 46 105 0 30 34 36 37 38 68 33 35 32 31 12 0 59 58 56
	*   57 61 64 65 63 4 22 24 0 26 29 67 10 41 70 40 43 45 16 0
	 */
	const vector<int> test_route = {
		87, 112, 113, 116, 77, 85, 104, 86, 111, 117, 92, 114,
		115, 101, 102, 75, 93, 62, 60, 82, 89, 76, 91, 121, 109,
		74, 119, 90, 78, 95, 80, 98, 79, 96, 118, 108, 107, 73,
		120, 103, 88, 71, 83, 51, 55, 52, 48, 47, 49, 50, 53, 54,
		84, 106, 23, 27, 25, 66, 28, 100, 94, 99, 81, 72, 97, 110,
		39, 69, 42, 44, 46, 105, 30, 34, 36, 37, 38, 68, 33, 35,
		32, 31, 59, 58, 56, 57, 61, 64, 65, 63, 22, 24, 26, 29,
		67, 41, 70, 40, 43, 45
	};

	/*
	 * Good news everyone! It seems my SimulateDrive function is perfectly capable of taking in a list of
	 * nodes, doing the appropriate pathfinding, and calculating the correct distance for just the EVRP.
	 * What it isn't doing at the moment is figuring out when it needs to go back to the depot. It is
	 * currently just blindly going from one node to the next. This proof of concept shows the power
	 * of the VehicleStatus struct instead of tracking battery, inventory, and time separately. What
	 * I need to do now is figure out how to go back to the depot when needed, and build a function in
	 * Vehicle that takes a full list of customer nodes and calculates the whole tour. From there, I
	 * can get the NEH code to do the proper things again. Then I will add back in the time window stuff
	 * and see what happens. What I should focus on first is getting the EVRP working with this new code
	 * then worry about adding in the TW stuff. Trying to do both at once has made it very difficult to
	 * track down where bugs are occurring and where the wrong answers are showing up.
	 *
	 *
	 * what i need to do is this:
	 * make a function called BuildFullRoute in vehicle that takes an any sized vector of customer nodes.
	 * this function will pad the route with a start at the depot, and an end at the depot. it will then
	 * go through, calling CanGetToNextCustomerWithinConstraints on every pair of nodes, and placing
	 * trips to charging stations and the depot into the route accordingly. the output of this function
	 * should be a vector of nodes through the full route, taking into account trips to the depot or chargers
	 * as well as all the customers. then, what i can do with that output is build another function called
	 * CalculateFullRouteDistance or something, that takes in the full route and does the vehicle status updates
	 * for each pair of nodes in the route. after each status update, i can assert that all of the constraints
	 * are being met between each node, and i can sum up all of the distances very similarly to what i am doing
	 * below this comment. That way, in the GA or random search code, i can call 1 function that takes the
	 * customer route and returns a fully simulated and constrained distance. likewise in the NEH stuff,
	 * I can call this function with any portion of the subtour when figuring out where to place the next node.
	 * I can test it by first removing the TW stuff from the UpdateVehicleStatus function and seeing if
	 * I can generate the test route with the charging station and depot trips automatically, instead of having
	 * to pad the depot visits manually. 
	 */

	
	//for(int i = 0; i < 50; i++)
	//{
	/*
	auto alg = new RandomSearch(problem_definition);
	solution best;
	alg->Optimize(best);
	HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(best.tour));
	cout << best.distance << endl;
	*/
	//}
	/*
	float dist = 0.f;
	for(size_t i = 1; i < test_route.size(); i++)
	{
		const Node* from = problem_definition->GetNodeFromIndex(test_route[i-1]);
		const Node* to = problem_definition->GetNodeFromIndex(test_route[i]);
		vehicle->SimulateDrive(status, from, to);

		if(to->node_type == Depot)
		{
			dist += status->distance_traveled;
			status = new VehicleStatus(problem_definition->GetVehicleParameters());
		}

		//cout << "===== The route from " << from->index << " to " << to->index << " had a distance of " 
		//dist += status->distance_traveled;
	}
	cout << "Best tour has a distance of: " << dist << endl;
	*/

	
	//true route is:                0 7 6 14 5 11 17 13 2 16 19 9 12 3 8 4 20 15 10 0 18 0
	const vector<int> test_route1 = {7, 6, 14, 11, 17, 13, 16, 19, 9, 12, 8, 20, 15, 10, 18};
	//true route is:                
	const vector<int> test_route2 = {7, 8, 4, 5, 6};

	//true route is:                 0, 5, 2, 6, 0, 8, 1, 7, 0, 4, 0
	const vector<int> test_route3 = {5, 6, 8, 7, 4};

	const vector<int> test_route4 = {17, 14};

	//expected:					0, 84, 6, 0
	const vector test_route5 = {84};


	//expected:					0, 65, 6, 0
	const vector test_route6 = {65};

	/*
	int subtours = 0;
	const auto node_route = HelperFunctions::GetNodeDecodedTour(problem_definition, test_route6);
	const auto route = vehicle->BuildFullRoute(node_route);
	HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(route));
	
	const auto dist = vehicle->CalculateTrueRouteDistance(node_route, subtours, true);
	cout << dist << endl;
	*/

	const vector test_recursion = {0, 8};
	const vector test_full_build = {8, 7, 4, 5, 6};
	const auto nodes = HelperFunctions::GetNodeDecodedTour(problem_definition, test_full_build);
	vector<shared_ptr<Node>> out_route;
	//bool out_visited_depot;
	const VehicleStatus test_status(problem_definition->GetVehicleParameters());
	//vehicle->RouteToNext(test_status, nodes[0], nodes[1], out_route, out_visited_depot);
	auto sol = vehicle->BuildOptimalSolution(nodes);
	HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(sol.tour));
	cout << sol.distance << endl;

	/*
	const auto alg = new RandomSearch(problem_definition);
	solution best;
	alg->Optimize(best);
	int subtours = 0;
	const auto dist = vehicle->CalculateTrueRouteDistance(best.tour, subtours, true);
	
	HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(best.tour));
	cout << "Solution: " << best.distance << endl;
	cout << "Route distance: " << dist << " and route subtours: " << subtours << endl;

	assert(fabs(best.distance - dist) < 0.01);

	delete alg;
	delete vehicle;
	delete status;
	*/
}

/***************************************************************************//**
 * \brief SolveEVRP is where the choice of algorithm occurs. 
 *
 * In order to keep the problem and the algorithm implementation separate, the 
 * SolveEVRP function has control over which algorithm it selects. Currently, we
 * implement GeneticAlgorithmOptimizer, RandomSearchOptimizer, and NEH_NearestNeighbor.
 * Each one of these algorithms runs with the provided problem instance, and the results
 * are each logged to a file with the proper information. 
 ******************************************************************************/
void Solver::SolveEVRP(Alg algorithm, bool solving_in_debug) const
{
	unique_ptr<Algorithm> alg;
	switch (algorithm) {
	case RNG:
		alg = make_unique<RandomSearch>(problem_definition, shared_hasher);
		break;
	case NEH:
		alg = make_unique<NEH_TimeWindow>(problem_definition, shared_hasher);
		break;
	case GA:
		alg = make_unique<GeneticAlgorithm>(problem_definition, shared_hasher);
		break;
	case GA_Seed:
		alg = make_unique<SeededGA>(problem_definition, shared_hasher);
		break;
	}

	//cout << "Calculating standard solve for " << alg->GetName() << "!" << endl;
		
	solution best_solution = {};
		
	//What time is it before solving the problem
	const HANDLE thread = GetCurrentThread();
	const ULARGE_INTEGER start = get_thread_cpu_time(thread);

	//Function call to the GeneticAlgorithmOptimizer class that will return the best tour
	//from the given data
	alg->Optimize(best_solution);

	//What time is it now that we've solved the problem
	const ULARGE_INTEGER end = get_thread_cpu_time(thread);
		
	//Get the execution time in milliseconds 
	const double duration = static_cast<double>(end.QuadPart - start.QuadPart) / 10000;

	//assert that each customer node only appears once in the solution
	for (const auto& index : best_solution.tour)
	{
		if(index->node_type != Customer) continue;
		
		int index_count = 0;
		for (const auto& i : best_solution.tour)
		{
			if (index == i) index_count++;
		}
		assert(index_count == 1);
	}

	if(!solving_in_debug)
	{
		optimization_result result;
		result.algorithm_name = alg->GetName();
		result.execution_time = static_cast<float>(duration) / 1000.0f;
		result.solution_encoded = HelperFunctions::GetIndexEncodedTour(HelperFunctions::StripNonCustomerNodesFromTour(best_solution.tour));
		result.solution_decoded = best_solution.tour;
		result.distance = best_solution.distance;
		result.subtours = best_solution.subtours;
		result.hyperparameters = alg->GetHyperParameters();
	
		unique_lock lock(file_write_mutex);
		WriteToFile(result);
		lock.unlock();
	}
	else
	{
		cout << "Best distance: " << best_solution.distance << " with " << best_solution.subtours << " subtours" << endl;
		HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(best_solution.tour));
		HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(HelperFunctions::StripNonCustomerNodesFromTour(best_solution.tour)));
	}
}

/**
 * \brief Static write to file function that takes the results and writes to @WRITE_FILENAME.
 * We care about logging the results distance, name of the problem, algorithm name, and execution time.
 * We also care about writing the solution and the hyperparameters to the file in case we want to do more
 * evaluation on the specifics that went into generating this solution.
 * \param result A const reference to the optimization_results data structure that holds the information we care about
 */
void Solver::WriteToFile(const optimization_result& result) const
{
	ofstream file;
	file.open(WRITE_FILENAME, ios_base::app);

	if(!file.is_open())
	{
		cerr << "Failed to write to file: " << WRITE_FILENAME << endl;
	}

	//The distance of the solution
	file << result.distance << ",";
	file << result.subtours << ",";

	//The problem name
	file << _current_filename << ",";
	
	file << result.algorithm_name << ",";
	file << result.execution_time << ",";

	//Writing each element of the solution vector
	string encoded_solution;
	for(const auto &iter : result.solution_encoded)
	{
		encoded_solution += to_string(iter) + " ";
	}
	encoded_solution.pop_back();
	file << encoded_solution << ",";

	//Writing each element of the hyperparameters vector
	string hyper_parameters;
	for(const auto &iter : result.hyperparameters)
	{
		hyper_parameters += iter + "|";
	}
	hyper_parameters.pop_back();
	file << hyper_parameters;
	
	file << "\n";

	file.close();
}

ULARGE_INTEGER Solver::get_thread_cpu_time(const HANDLE h_thread)
{
	FILETIME ft_creation, ft_exit, ft_kernel, ft_user;
	ULARGE_INTEGER ul_kernel, ul_user;
	if (GetThreadTimes(h_thread, &ft_creation, &ft_exit, &ft_kernel, &ft_user)) {
		ul_kernel.HighPart = ft_kernel.dwHighDateTime;
		ul_kernel.LowPart = ft_kernel.dwLowDateTime;
		ul_user.HighPart = ft_user.dwHighDateTime;
		ul_user.LowPart = ft_user.dwLowDateTime;
	}
	ULARGE_INTEGER total;
	total.QuadPart = ul_kernel.QuadPart + ul_user.QuadPart;
	return total;
}
