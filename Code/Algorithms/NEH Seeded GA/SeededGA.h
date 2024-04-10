#pragma once
#include "../Algorithm.h"
#include "../GA/GeneticAlgorithm.h"

class SeededGA : public Algorithm
{
public:
    SeededGA(const shared_ptr<ProblemDefinition>& data, const shared_ptr<Hasher>& hasher) :
        Algorithm("Genetic Algorithm Seeded from NEH", data, hasher)
    {
        vector<string> hyper_parameters;
        
        hyper_parameters.push_back(string("Population Size: ") + to_string(POPULATION_SIZE));
        hyper_parameters.push_back(string("Maximum Generations: ") + to_string(MAX_GENERATIONS));
        hyper_parameters.push_back(string("Tournament Size: ") + to_string(TOURNAMENT_SIZE));
        hyper_parameters.push_back(string("Mutation Rate: ") + to_string(MUTATION_RATE));

        SetHyperParameters(hyper_parameters);
    }

    void Optimize(solution &best_solution) override;
};
