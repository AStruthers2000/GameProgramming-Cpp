#include "RandomSearch.h"

#include <mutex>
#include <thread>

#include "../../Helpers/HelperFunctions.h"
#include "../../Helpers/SolutionSet.h"

/**
 * \brief Generate #NUM_GENERATIONS * #SOLUTIONS_PER_GENERATION random solutions, saving the best from each generation.
 * We use random generation to generate #SOLUTIONS_PER_GENERATION purely random solutions. We save the best one, and
 * do this #NUM_GENERATIONS times. By the end, we will have #NUM_GENERATIONS "good" solutions. This could be used as
 * a good seed for other algorithms that start with an initial population.
 * \param best_solution
 */
void RandomSearch::Optimize(solution &best_solution)
{
    const auto best_solutions = make_unique<SolutionSet>();
    
    for (int i = 0; i < NUM_GENERATIONS; i++)
    {
        //cout << "Calculating generation " << i + 1 << endl;
        GenerateSolutionSet(best_solutions);
    }
    
    best_solution = best_solutions->GetBestSolution();
}

void RandomSearch::GenerateSolutionSet(const unique_ptr<SolutionSet>& all_solutions) const
{
    const auto generation_solutions = make_unique<SolutionSet>();
    
    for (int j = 0; j < SOLUTIONS_PER_GENERATION; j++)
    {
        //cout << "\tGenerating solution " << j << endl;
        vector<shared_ptr<Node>> tour = problem_data->GenerateRandomTour();

        //cout << "\tCalculating solution " << j << endl;
        auto sol = vehicle->BuildOptimalSolution(tour);
        //cout << "\tAdding solution " << j << endl;
        generation_solutions->AddSolutionToSet(sol);
    }
    const solution best = generation_solutions->GetBestSolution();
    all_solutions->AddSolutionToSet(best);
}



     
