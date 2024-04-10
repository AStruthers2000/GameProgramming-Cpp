#pragma once
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <string>
#include <vector>



/***************************************************************************//**
 * Data structure definitions useful in optimizing the EVRP.
 *
 * I define the two useful data structures here. I also include every library I need for this project here,
 * which I know isn't the best practice, but since every class that has anything to do with solving this problem
 * needs to include this class to get a reference to the EVRP_Data typedef, I figured that it would be a good way
 * to guarantee that I don't have multiple of the same includes spread across multiple files.
 ******************************************************************************/

using namespace std;
struct VehicleStatus;

enum NodeType
{
	Depot,
	Charger,
	Customer
};

/** 
* The Node structure consists of an x and y coordinate, the value of the demand at this node (always 0 for charging nodes),
* a bool that represents if this node is a charger or not (depot and charging nodes both have demand = 0, so we need more specificity),
* and the index of this node in the list of all nodes. The depot is always index = 0. 
*/
struct Node
{
	double x;
	double y;
	string unique_id;
	int demand;
	NodeType node_type;
	float ready_time;
	float due_date;
	float service_time;
	int index;
	bool been_visited = false;
	vector<pair<shared_ptr<Node>, float>> relative_customers;
	vector<pair<shared_ptr<Node>, float>> relative_chargers;	

	bool operator==(const Node &n) const
	{
		return index == n.index;
	}
	bool operator!=(const Node &n) const
	{
		return index != n.index;
	}

	shared_ptr<Node> GetNearestUnvisitedCustomer() const
	{
		for(const auto& n : relative_customers)
		{
			auto closest = n.first;
			if(!closest->been_visited)
			{
				closest->been_visited = true;
				return closest;
			}
		}
		return nullptr;
	}

	shared_ptr<Node> GetNearestCharger() const
	{
		return relative_chargers[0].first;
	}

	shared_ptr<Node> GetNearestUnvisitedCharger() const
	{
		for(const auto& n : relative_chargers)
		{
			auto closest = n.first;
			if(!closest->been_visited)
			{
				closest->been_visited = true;
				return closest;
			}
		}
		return nullptr;
	}

	/*
	bool operator<(const Node &n) const
	{
		return demand < n.demand;
	}
	*/
};




/**
* the EVRP_Data structure holds a vector of every Node in the problem, as well as information for the vehicle.
* This is the core data structure that can be passed to any optimization class. All of the required information
* to start solving this problem is contained here, so this is the only payload that would need to be sent to 
* classes that implement optimization algorithms. 
*/
/*
typedef struct
{
	vector<Node> nodes;
	float fuelCapacity;
	int loadCapacity;
	float fuelConsumptionRate;
	int customerStartIndex;
} EVRP_Data;
*/

struct optimization_result
{
	string algorithm_name;
	float execution_time;
	float distance;
	int subtours;
	vector<int> solution_encoded;
	vector<shared_ptr<Node>> solution_decoded;
	vector<string> hyperparameters;
	//maybe care about memory use?
};

struct VehicleParameters
{
	//inventory
	int load_capacity;

	//battery
	float battery_capacity;
	float battery_consumption_rate;
	float inverse_recharging_rate;

	//movement
	float average_velocity;
};

struct VehicleStatus
{
	shared_ptr<VehicleParameters> parameters;
	int current_load;
	float current_battery;
	float current_time;
	float distance_traveled;
	bool invalid_route = false;

	VehicleStatus(const shared_ptr<VehicleParameters>& params) //: parameters(std::move(params)) 
	{
		parameters = params;
		distance_traveled = 0.f;
		InitializeStatusFromParams();
	}

	VehicleStatus(const shared_ptr<VehicleParameters>& params, VehicleStatus other)
	{
		parameters = params;
		current_battery = other.current_battery;
		current_load = other.current_load;
		current_time = other.current_time;
		distance_traveled = other.distance_traveled;
	}

	void InitializeStatusFromParams()
	{
		//assert(parameters != nullptr);
		current_load = parameters->load_capacity;
		current_battery = parameters->battery_capacity;
		current_time = 0;
	}

	/*
	unique_ptr<VehicleStatus> DeepCopy() const
	{
		auto new_status = make_unique<VehicleStatus>(parameters);
		new_status->current_battery = current_battery;
		new_status->current_load = current_load;
		new_status->current_time = current_time;
		new_status->distance_traveled = distance_traveled;
		return new_status;
	}
	*/

	VehicleStatus DeepCopy() const
	{
		assert(parameters != nullptr);
		VehicleStatus new_status(parameters);
		new_status.current_battery = current_battery;
		new_status.current_load = current_load;
		new_status.current_time = current_time;
		new_status.distance_traveled = distance_traveled;
		return new_status;
	}

	void PrintStatus() const
	{
		cout << "\n====================" << endl;
		cout << "Battery   = " << current_battery << endl;
		cout << "Inventory = " << current_load << endl;
		cout << "Time      = " << current_time << endl;
		cout << "Distance  = " << distance_traveled << endl;
		cout << "====================\n" << endl;
	}
};

class ProblemDefinition
{
public:
	ProblemDefinition(const vector<shared_ptr<Node>> &nodes, const shared_ptr<VehicleParameters>& vehicle_params)// : vehicle_parameters(std::move(vehicle_params))
	{
		vehicle_parameters = vehicle_params;
		for(const auto &n : nodes)
		{
			all_nodes.push_back(n);
			switch (n->node_type)
			{
			case Depot:
				depot = n;
				break;
			case Charger:
				charger_nodes.push_back(n);
				break;
			case Customer:
				customer_nodes.push_back(n);
				break;
			}
		}
	}

	/*
	~ProblemDefinition()
	{
		delete depot;
		delete vehicle_parameters;
	}
	*/

	vector<shared_ptr<Node>> GenerateRandomTour() const
	{
		random_device rd;
		mt19937 rng(rd());
		vector<shared_ptr<Node>> shuffled;
		shuffled.reserve(customer_nodes.size());
		for(const auto& node : customer_nodes)
		{
			shuffled.push_back(node);
		}
		shuffle(shuffled.begin(), shuffled.end(), rng);
		return shuffled;
	}
	
	shared_ptr<Node> GetDepotNode() const { return depot; }
	vector<shared_ptr<Node>> GetAllNodes() const { return all_nodes; }
	vector<shared_ptr<Node>> GetChargingNodes() const { return charger_nodes; }
	vector<shared_ptr<Node>> GetCustomerNodes() const { return customer_nodes; }
	shared_ptr<VehicleParameters> GetVehicleParameters() { return vehicle_parameters; }

	shared_ptr<Node> GetNodeFromIndex(const int index) const
	{
		for(const auto &n : GetAllNodes())
		{
			if(n->index == index)
			{
				return n;
			}
		}
		return {};
	}

	bool HaveAllCustomersBeenVisited() const
	{
		return all_of(
			customer_nodes.begin(),
			customer_nodes.end(),
			[](const auto& c)
			{
				return c->been_visited;
			});
	}

	void ResetAllChargerVisitationStatus() const
	{
		for(const auto& node : charger_nodes)
		{
			node->been_visited = false;
		}
	}

	void ResetAllCustomerVisitationStatus() const
	{
		for(const auto& node : customer_nodes)
		{
			node->been_visited = false;
		}
	}

	void ResetAllNodeVisitationStatus() const
	{
		for(const auto& node : all_nodes)
		{
			node->been_visited = false;
		}
	}

private:
	shared_ptr<Node> depot;
	vector<shared_ptr<Node>> all_nodes;
	vector<shared_ptr<Node>> customer_nodes;
	vector<shared_ptr<Node>> charger_nodes;

	shared_ptr<VehicleParameters> vehicle_parameters;
};
