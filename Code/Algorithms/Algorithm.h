#pragma once
#include "../Helpers/Hasher.h"
#include "../Vehicle/Vehicle.h"
#include "../Helpers/SolutionSet.h"

class Algorithm
{
public:
    Algorithm();
   
    Algorithm(const string &algorithm_name, const shared_ptr<ProblemDefinition>& data, const shared_ptr<Hasher>& hasher)
    {
        problem_data = data;
        vehicle = make_unique<Vehicle>(problem_data, hasher);
        problem_data = data;

        name = algorithm_name;
        hyper_parameters.clear();

        found_tours = make_unique<SolutionSet>();

        shared_hasher = hasher;
    }

    // Assume a move constructor
    Algorithm(Algorithm&& other) noexcept
        : problem_data(std::move(other.problem_data)),
          vehicle(std::move(other.vehicle)),
          found_tours(std::move(other.found_tours)),
          name(std::move(other.name)),
          hyper_parameters(std::move(other.hyper_parameters)) {}

    // Assume a move assignment operator
    Algorithm& operator=(Algorithm&& other) noexcept {
        if (this != &other) {
            problem_data = std::move(other.problem_data);
            vehicle = std::move(other.vehicle);
            name = std::move(other.name);
            hyper_parameters = std::move(other.hyper_parameters);
            found_tours = std::move(other.found_tours);
        }
        return *this;
    }

    virtual ~Algorithm()
    = default;

    /*
    virtual ~Algorithm()
    {
        delete vehicle;
        //delete problem_data;
        delete found_tours;
    }
    */
    
    virtual void Optimize(solution &best_solution) = 0;
    string GetName() { return name; }
    vector<string> GetHyperParameters() { return hyper_parameters; }
    unique_ptr<SolutionSet> GetFoundTours() { return std::move(found_tours); }
   
protected:
    shared_ptr<ProblemDefinition> problem_data;
    unique_ptr<Vehicle> vehicle;

    unique_ptr<SolutionSet> found_tours;
   
    void SetHyperParameters(const vector<string> &params)
    {
        for(const auto &iter : params)
        {
            hyper_parameters.push_back(iter);
        }
    }

    shared_ptr<Hasher> shared_hasher;

private:
    string name;
    vector<string> hyper_parameters;
};
