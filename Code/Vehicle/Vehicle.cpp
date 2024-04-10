#include "Vehicle.h"
#include <iostream>
#include <mutex>

#include "../Helpers/HelperFunctions.h"
#include "../Helpers/SolutionSet.h"

mutex memo_lock;

solution Vehicle::BuildOptimalSolution(const vector<shared_ptr<Node>>& customer_nodes)
{
    const shared_ptr<Node> depot = problem_definition->GetDepotNode();
    vector full_route = {depot};
    
    vector padded_customer_route = customer_nodes;
    padded_customer_route.push_back(depot);

    shared_ptr<Node> current = depot;
    auto status = *make_unique<VehicleStatus>(parameters);
    float distance_traveled = 0.f;
    int subtour_count = 0;
    vector<shared_ptr<Node>> current_customer_subtour;
    vector<shared_ptr<Node>> current_full_subtour;
    bool just_visited_depot = true;
    int customer_index = 0;

    //for(size_t customer_index = 0; customer_index < padded_customer_route.size(); customer_index++)
    while(customer_index < static_cast<int>(padded_customer_route.size()))
    {
        bool cached_sol_found = false;
        if(just_visited_depot)
        {
            //handle the lookup of the next n nodes, where n = hasher->GetLongestSubtour()
            //vector<vector<shared_ptr<Node>>> all_possible_subtours;
            for(int i = 0; i < shared_hasher->GetLongestSubtour(); i++)
            {
                if(customer_index + i >= static_cast<int>(padded_customer_route.size() - 1)) break;
                auto current_pos = padded_customer_route.begin() + customer_index;
                auto end_pos = current_pos + i + 1;
                vector<shared_ptr<Node>> possible_subtour = {current_pos, end_pos};
                shared_ptr<Node> next_node = *(current_pos + i + 1);

                HashedSolution sol;
                unique_lock lock(memo_lock);
                const bool found_cached = shared_hasher->TryFindCachedSubtour(possible_subtour, next_node, sol);
                lock.unlock();
                if(found_cached)
                {
                    for(const auto& node : sol.full_route)
                    {
                        full_route.push_back(node);
                    }
                    distance_traveled += sol.route_distance;
                    subtour_count++;
                    customer_index += i + 1;
                    cached_sol_found = true;
                    break;
                }
                //all_possible_subtours.emplace_back(current_pos, end_pos);
            }
            
            //on a successful find, we will update customer_index to reflect the skipping over the cached subtour
            //and continue with the for loop, so the code below will only run if we didn't find a cached subtour
            
        }
        if(cached_sol_found)
        {
            //cout << "cached solution found, skipping calculations!" << endl;
            continue;
        }
        
        just_visited_depot = false;
        
        shared_ptr<Node> customer_node = padded_customer_route[customer_index];
        vector<shared_ptr<Node>> out_route;
        bool visits_depot;
        auto new_status = RouteToNext(status, current, customer_node, out_route, visits_depot);

        //if there is a viable, within constraints route to the next customer node, we want to go there
        if(!out_route.empty())
        {
            current_customer_subtour.push_back(customer_node);
            for(size_t i = 1; i < out_route.size(); i++)
            {
                full_route.push_back(out_route[i]);
                current_full_subtour.push_back(out_route[i]);
            }
            customer_index++;
            current = customer_node;
            status = new_status;
            if(customer_node == depot) just_visited_depot = true;
        }

        //there is no viable, within constraints route to get to the next customer node, so we route to the depot
        else
        {
            new_status = RouteToNext(status, current, depot, out_route, visits_depot);
            for(size_t i = 1; i < out_route.size(); i++)
            {
                full_route.push_back(out_route[i]);
                current_full_subtour.push_back(out_route[i]);
            }
            just_visited_depot = true;
        }

        //if we have just visited the depot (either we needed to for constraint reasons, or we just ended the full route)
        //we want to do some housecleaning like memoizing the subtour
        if(just_visited_depot)
        {
            //at this point, we are at the depot
            distance_traveled += new_status.distance_traveled;
            subtour_count++;

            //i actually want to remove the last element of the current_full_subtour, because it contains the depot node
            current_full_subtour.pop_back();

            if(customer_index < static_cast<int>(padded_customer_route.size()))
            {
                shared_ptr<Node> next_node = padded_customer_route[customer_index];
                unique_lock lock(memo_lock);
                shared_hasher->CacheSubtour(current_customer_subtour, current_full_subtour, next_node, new_status.distance_traveled);
                lock.unlock();
            }

            current_customer_subtour.clear();
            current_full_subtour.clear();

            //just_visited_depot = true;

            current = depot;
            status = *make_unique<VehicleStatus>(parameters);
        }

        
    }
    //solution sol(full_route, distance_traveled + status.distance_traveled, subtour_count+1);
    solution sol(full_route, distance_traveled, subtour_count);
    return sol;
}

VehicleStatus Vehicle::RouteToNext(const VehicleStatus& status, const shared_ptr<Node>& start,
                                   const shared_ptr<Node>& destination, vector<shared_ptr<Node>>& out_route, bool& out_visited_depot)
{
    /*
     * The goal of this function is to take a start and a destination node and calculate the most optimal
     * feasible path from the start to the destination, making sure that we *could* go from the destination
     * to the depot within constraints. This function will return a vehicle status that represents the vehicle
     * as it finishes servicing (but not yet leaving!!!) the destination. There is an out parameter for the
     * path that contains the start and destination nodes, as well as any charging stations or depot visits.
     *
     * We are going to do this by finding potential feasible routes that begin at start, visit 0, 1, or 2 nodes
     * on the way to destination, then visit 0, 1, or 2 nodes on the way to the depot. Whichever feasible solution
     * ends at the depot the earliest in time is the optimal route. In the event that two routes end at the depot
     * at the exact same time, the route that covers the least distance will be the winner. We say the optimal route
     * is the route that ends at the depot the earliest in time is the optimal because the sooner the route *could* get
     * to the depot, the more time the route has to visit other customer nodes before needing to visit the depot. 
     */
    
    auto all_paths = RecursivePathfind(status, start, destination, {start});
    sort(all_paths.begin(),
        all_paths.end(),
        [this](const auto& a, const auto& b)
        {
            return CompareFoundPaths(a, b);
        });

    out_visited_depot = true;
    if(all_paths.empty()) return status;
        
    const auto best_path = all_paths[0];
    //HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(best_path.first));
    //best_path.second.PrintStatus();
    vector<shared_ptr<Node>> route;
    auto current_status = status;
    for(size_t i = 1; i < best_path.first.size(); i++)
    {
        const auto from = best_path.first[i-1];
        const auto to = best_path.first[i];
        const auto new_status = CalculateVehicleStatus(current_status, from, to);
        current_status = new_status;
        if(to == problem_definition->GetDepotNode())
        {
            out_visited_depot = true;
        }
        if(to == destination)
        {
            route = {best_path.first.begin(), best_path.first.begin() + static_cast<long long>(i+1)};
            break;
        }
    }
    //HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(route));
    //current_status.PrintStatus();
    out_route = route;
    return current_status;
}

float Vehicle::CalculatePerimeter(const vector<shared_ptr<Node>>& vertices)
{
    float dist = 0.f;
    for(size_t i = 1; i < vertices.size(); i++)
    {
        dist += HelperFunctions::CalculateInterNodeDistance(vertices[i-1], vertices[i]);
    }
    return dist;
}

shared_ptr<Node> Vehicle::GetChargerWithSmallestPerimeter(const vector<shared_ptr<Node>>& chargers,
    const shared_ptr<Node>& start, const shared_ptr<Node>& destination)
{
    shared_ptr<Node> min_node;
    float min_dist = numeric_limits<float>::max();
    for(const auto& node : chargers)
    {
        const float dist = CalculatePerimeter({start, node, destination});
        if(dist < min_dist)
        {
            min_node = node;
            min_dist = dist;
        }
    }
    return min_node;
}

vector<pair<vector<shared_ptr<Node>>, VehicleStatus>> Vehicle::RecursivePathfind(
    const VehicleStatus& status, const shared_ptr<Node>& start,
    shared_ptr<Node> destination, const vector<shared_ptr<Node>>& path)
{
    if(status.invalid_route) return {};
    if(start == destination)
    {
        //problem_definition->ResetAllChargerVisitationStatus();
        destination = problem_definition->GetDepotNode();
    }

    if(start == problem_definition->GetDepotNode() && path.size() > 1)
    {
        //HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(path));
        //status.PrintStatus();
        return {make_pair(path, status)};
    }

    const vector<shared_ptr<Node>> chargers_in_range = GetAllChargersInRange(start, status);
    if(chargers_in_range.empty())
    {
        return {};
    }
    
    const shared_ptr<Node> closest_charger = GetChargerWithSmallestPerimeter(chargers_in_range, start, destination);

    

    vector<shared_ptr<Node>> charger_path = path;
    charger_path.push_back(closest_charger);

    vector<shared_ptr<Node>> destination_path = path;
    destination_path.push_back(destination);

    const VehicleStatus charger_status = CalculateVehicleStatus(status, start, closest_charger);
    const VehicleStatus destination_status = CalculateVehicleStatus(status, start, destination);

    vector<pair<vector<shared_ptr<Node>>, VehicleStatus>> results;
    
    if(path.size() > 1 && start->node_type == Charger && (*(path.end()-2))->node_type == Charger)
    {
        results = RecursivePathfind(destination_status, destination, destination, destination_path);
    }
    else
    {
        const auto dest_results = RecursivePathfind(destination_status, destination, destination, destination_path);
        const auto charger_results = RecursivePathfind(charger_status, closest_charger, destination, charger_path);

        for(const auto& p : dest_results)
        {
            results.push_back(p);
        }
        for(const auto& p : charger_results)
        {
            results.push_back(p);
        }
    }
    return results;
}

VehicleStatus Vehicle::CalculateVehicleStatus(const VehicleStatus& status, const shared_ptr<Node>& from,
    const shared_ptr<Node>& to) const
{
    VehicleStatus new_status = status;
    
    new_status.invalid_route = false;
    const float distance_traveled = HelperFunctions::CalculateInterNodeDistance(from, to);
    
    new_status.distance_traveled += distance_traveled;
    new_status.current_battery -= BatteryCost(from, to);
    new_status.current_time += TimeCost(from, to);

    //assert(new_status.current_battery >= 0);
    if(new_status.current_battery < 0)
    {
        new_status.invalid_route = true;
    }

    const auto depot = problem_definition->GetDepotNode();
    switch(to->node_type)
    {
    case Depot:
        {
            if(status.current_time > depot->due_date)
            {
                new_status.invalid_route = true;
            }
        }
        break;
        
    case Charger:
        new_status.current_time += RefuelingTime(status);
        new_status.current_battery = status.parameters->battery_capacity;
        break;
        
    case Customer:
        if(new_status.current_time < to->ready_time)
        {
            new_status.current_time = to->ready_time;
        }
        //we arrive after the due date, which means we can't service this node
        //we effectively reset our status to before visiting this node, since we
        //can't properly service it anyway
        else if (new_status.current_time > to->due_date)
        {
            new_status.distance_traveled -= distance_traveled;
            new_status.current_battery += BatteryCost(from, to);
            new_status.current_time -= TimeCost(from, to);
            new_status.invalid_route = true;
        }
        
        new_status.current_load -= to->demand;
        if(new_status.current_load < 0)
        {
            new_status.invalid_route = true;
        }
        new_status.current_time += to->service_time;
        break;
    }

    return new_status;
}



vector<shared_ptr<Node>> Vehicle::GetAllChargersInRange(const shared_ptr<Node>& current, const VehicleStatus& status) const
{
    vector<shared_ptr<Node>> chargers_in_range;
    for(const auto& node : problem_definition->GetChargingNodes())
    {
        if(node == current || (current == problem_definition->GetDepotNode() && node->index == 1)) continue;
        
        if(BatteryCost(current, node) <= status.current_battery)
        {
            chargers_in_range.push_back(node);
        }
    }
    return chargers_in_range;
}

shared_ptr<Node> Vehicle::GetClosestChargerFromRange(const vector<shared_ptr<Node>>& range, const shared_ptr<Node>& source)
{
    pair<shared_ptr<Node>, float> closest_charger = {nullptr, numeric_limits<float>::max()};
    for(auto n : range)
    {
        if(n == source) continue;
        const float distance = HelperFunctions::CalculateInterNodeDistance(n, source);
        if(distance < closest_charger.second)
        {
            closest_charger = make_pair(n, distance);
        }
    }
    closest_charger.first->been_visited = true;
    return closest_charger.first;
}

float Vehicle::BatteryCost(const shared_ptr<Node>& from, const shared_ptr<Node>& to) const
{
    return HelperFunctions::CalculateInterNodeDistance(from, to) * parameters->battery_consumption_rate;
}

float Vehicle::TimeCost(const shared_ptr<Node>& from, const shared_ptr<Node>& to) const
{
    return HelperFunctions::CalculateInterNodeDistance(from, to) * parameters->average_velocity;
}

float Vehicle::RefuelingTime(const VehicleStatus& status) const
{
    const float difference = status.parameters->battery_capacity - status.current_battery;
    return difference * status.parameters->inverse_recharging_rate;
}
