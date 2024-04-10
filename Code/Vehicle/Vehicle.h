#pragma once
#include "../Helpers/Hasher.h"
#include "../Helpers/ProblemDefinition.h"

struct VehicleParameters;
struct VehicleStatus;
struct solution;
class ProblemDefinition;

constexpr float TIME_PUNISHMENT = 1e06;

class Vehicle
{
public:
    Vehicle(const shared_ptr<ProblemDefinition>& problem, const shared_ptr<Hasher>& hasher) //: problem_definition(std::move(problem))
    {
        problem_definition = problem;
        parameters = problem_definition->GetVehicleParameters();
        shared_hasher = hasher;
    }

    ~Vehicle() = default;

    VehicleStatus RouteToNext(const VehicleStatus& status, const shared_ptr<Node>& start, const shared_ptr<Node>& destination, vector<shared_ptr<Node>>& out_route, bool &out_visited_depot);
    solution BuildOptimalSolution(const vector<shared_ptr<Node>>& customer_nodes);
    
private:
    static float CalculatePerimeter(const vector<shared_ptr<Node>>& vertices);
    static shared_ptr<Node> GetChargerWithSmallestPerimeter(const vector<shared_ptr<Node>>& chargers, const shared_ptr<Node>& start, const shared_ptr<Node>& destination);

    vector<pair<vector<shared_ptr<Node>>, VehicleStatus>> RecursivePathfind(const VehicleStatus& status, const shared_ptr<Node>& start, shared_ptr<Node> destination, const vector<shared_ptr<Node>>& path);
    VehicleStatus CalculateVehicleStatus(const VehicleStatus& status, const shared_ptr<Node>& from, const shared_ptr<Node>& to) const;

    vector<shared_ptr<Node>> GetAllChargersInRange(const shared_ptr<Node>& current, const VehicleStatus& status) const;
    static shared_ptr<Node> GetClosestChargerFromRange(const vector<shared_ptr<Node>>& range, const shared_ptr<Node>& source);

    float BatteryCost(const shared_ptr<Node>& from, const shared_ptr<Node>& to) const;
    float TimeCost(const shared_ptr<Node>& from, const shared_ptr<Node>& to) const;
    float RefuelingTime(const VehicleStatus& status) const;
    
    
    
    
    static bool CompareFoundPaths(const pair<vector<shared_ptr<Node>>, VehicleStatus>& a, const pair<vector<shared_ptr<Node>>, VehicleStatus>& b)
    {
        constexpr float tolerance = 0.01f;
        const auto a_status = a.second;
        const auto b_status = b.second;

        //first I want to compare times. If one status has a lower time than the other, that status is better
        const float time_delta = a_status.current_time - b_status.current_time;
        
        //if there is a difference in time, I want to return the status with the least time
        if(fabs(time_delta) > tolerance) return a_status.current_time < b_status.current_time;

        //if I didn't return yet, the current time for both statuses is the same, and I want to compare battery
        const float battery_delta = a_status.current_battery - b_status.current_battery;
        
        //if there is a difference in battery, i want to return the most battery
        if(fabs(battery_delta) > tolerance) return a_status.current_battery > b_status.current_battery;

        //if they have the same time and the same battery, I now want the route with the shortest distance
        return a_status.distance_traveled < b_status.distance_traveled;
    }

    shared_ptr<ProblemDefinition> problem_definition;
    shared_ptr<VehicleParameters> parameters;
    shared_ptr<Hasher> shared_hasher;
};
