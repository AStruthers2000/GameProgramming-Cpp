#include "NEH_TimeWindow.h"
#include <cassert>
#include <iostream>
#include <mutex>

#include "../../Helpers/HelperFunctions.h"

mutex build_neh_subtours;


void NEH_TimeWindow::Optimize(solution& best_solution)
{
    unique_lock lock(build_neh_subtours);
    const auto subtours = GenerateSubtours();
    lock.unlock();

    vector<solution> optimal_subtours;
    optimal_subtours.reserve(subtours.size());
    for (const auto& subtour : subtours)
    {
        optimal_subtours.push_back(NEH_Optimize(subtour));
    }

    solution full_tour = {};
    for (const auto& subtour : optimal_subtours)
    {
        for (const auto& node : subtour.tour)
        {
            full_tour.tour.push_back(node);
        }
    }

    const auto sol = vehicle->BuildOptimalSolution(full_tour.tour);
    best_solution = sol;
}

vector<vector<shared_ptr<Node>>> NEH_TimeWindow::GenerateSubtours() const
{
    /*
     * Generate subtours from nearest neighbor metric
     */
    problem_data->ResetAllNodeVisitationStatus();
    vector<vector<shared_ptr<Node>>> subtours;
    while (!problem_data->HaveAllCustomersBeenVisited())
    {
        auto subtour = GenerateSubtour();
        subtours.push_back(subtour);
    }

    for (const auto& n : problem_data->GetCustomerNodes())
    {
        assert(n->been_visited == true);
    }
    problem_data->ResetAllNodeVisitationStatus();

    return subtours;
}

vector<shared_ptr<Node>> NEH_TimeWindow::GenerateSubtour() const
{
    auto status = *make_unique<VehicleStatus>(problem_data->GetVehicleParameters());

    const shared_ptr<Node> depot = problem_data->GetDepotNode();
    shared_ptr<Node> current = depot->GetNearestUnvisitedCustomer();

    if (current == nullptr)
    {
        cerr << "No customers unvisited" << endl;
        return {};
    }

    vector<shared_ptr<Node>> route;
    bool visited_depot;
    status = vehicle->RouteToNext(status, depot, current, route, visited_depot);
    if (route.empty())
    {
        cerr << "uh oh" << endl;
        return {};
    }

    vector subtour = {current};
    shared_ptr<Node> next = current->GetNearestUnvisitedCustomer();
    while (next != nullptr)
    {
        route.clear();
        status = vehicle->RouteToNext(status, current, next, route, visited_depot);
        if (route.empty())
        {
            next->been_visited = false;
            break;
        }
        subtour.push_back(next);
        current = next;
        next = current->GetNearestUnvisitedCustomer();
    }
    return subtour;
}

solution NEH_TimeWindow::NEH_Optimize(vector<shared_ptr<Node>> subtour) const
{
    if (subtour.size() == 1) return {subtour};

    int L = 2;
    solution optimal = {{subtour[0]}};
    do
    {
        const auto partial_solutions = make_unique<SolutionSet>();
        for (size_t i = 0; i < optimal.tour.size(); i++)
        {
            vector<shared_ptr<Node>> temp_subtour = optimal.tour;
            const auto index = temp_subtour.begin() + static_cast<long long>(i);

            temp_subtour.insert(index, subtour[L - 1]);

            solution temp_solution = vehicle->BuildOptimalSolution(temp_subtour);
            temp_solution.tour = HelperFunctions::StripNonCustomerNodesFromTour(temp_solution.tour);
            partial_solutions->AddSolutionToSet(temp_solution);
        }

        optimal = partial_solutions->GetBestSolution();
        L++;
    }
    while (L < static_cast<int>(subtour.size() + 1));

    return optimal;
}
