#pragma once
#include "../Algorithm.h"
class SolutionSet;

constexpr int POPULATION_SIZE = 200; /*!< Size of the population, aka how many solutions should each successive generation have*/
constexpr int MAX_GENERATIONS = 250; /*!< Number of generations the evolution will take place over.*/
constexpr int TOURNAMENT_SIZE = 10; /*!< The number of candidate solutions chosen at random from the current population when doing tournament selection*/
constexpr float MUTATION_RATE = 0.05f; /*!< The percent chance that each child will get mutated*/

class GeneticAlgorithm : public Algorithm
{
public:
	GeneticAlgorithm(const shared_ptr<ProblemDefinition>& data, const shared_ptr<Hasher>& hasher) : 
		Algorithm("Genetic Algorithm", data, hasher)
	{
		vector<string> hyper_parameters;
        
		hyper_parameters.push_back(string("Population Size: ") + to_string(POPULATION_SIZE));
		hyper_parameters.push_back(string("Maximum Generations: ") + to_string(MAX_GENERATIONS));
		hyper_parameters.push_back(string("Tournament Size: ") + to_string(TOURNAMENT_SIZE));
		hyper_parameters.push_back(string("Mutation Rate: ") + to_string(MUTATION_RATE));

		SetHyperParameters(hyper_parameters);
	}

	/*
	~GeneticAlgorithm() override
	{
		delete seed_solutions;
		delete vehicle;
		//delete problem_data;
		delete found_tours;
	}
	*/
	
	void AddSeedSolution(const solution& good_solution);
	void Optimize(solution &best_solution) override;

private:
	solution TournamentSelection(const shared_ptr<SolutionSet>& current_population) const;
	solution Crossover(const solution &parent_1, const solution &parent_2) const;
	void Mutate(solution &child);

	unique_ptr<SolutionSet> seed_solutions;
	bool has_seed_solutions = false;

	/*
	float CalculateAverageSolution(vector<float> distances) const
	{
		if(distances.empty()) return 0;

		auto const count = static_cast<float>(distances.size());
		const float result = accumulate(distances.begin(), distances.end(), 0.f) / count;
		return result;
	}

	float CalculateBestSolution(vector<float> distances) const
	{
		if(distances.empty()) return 0;
		const float result = *min_element(begin(distances), end(distances));
		return result;
	}
	*/
};

