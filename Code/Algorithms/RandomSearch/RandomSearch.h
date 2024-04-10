#pragma once
#include "../Algorithm.h"

constexpr int SOLUTIONS_PER_GENERATION = 200; /*!< The number of solutions that will be randomly generated. Of n solutions, the top 1 will be saved */
constexpr int NUM_GENERATIONS = 50; /*!< Number of "best" solutions desired, 1 from every "generation" */

class RandomSearch : public Algorithm
{
public:
    RandomSearch(const shared_ptr<ProblemDefinition>& data, const shared_ptr<Hasher>& hasher) : 
        Algorithm("Random Search", data, hasher)
    {
        vector<string> hyper_parameters;
        
        hyper_parameters.push_back(string("Solutions per Generation: ") + to_string(SOLUTIONS_PER_GENERATION));
        hyper_parameters.push_back(string("Number of Best Solutions: ") + to_string(NUM_GENERATIONS));

        SetHyperParameters(hyper_parameters);
    }
    
    void Optimize(solution &best_solution) override;

private:
    void GenerateSolutionSet(const unique_ptr<SolutionSet>& all_solutions) const;
};
