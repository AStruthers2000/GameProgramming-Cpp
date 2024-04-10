#include "SeededGA.h"

#include "../../Helpers/HelperFunctions.h"
#include "../NEH/NEH_TimeWindow.h"

void SeededGA::Optimize(solution& best_solution)
{
    const auto neh_alg = make_unique<NEH_TimeWindow>(problem_data, shared_hasher);
    const auto ga_alg = make_unique<GeneticAlgorithm>(problem_data, shared_hasher);

    solution best_neh;
    neh_alg->Optimize(best_neh);

    //cout << "NEH best solution distance: " << best_neh.distance << " with " << best_neh.subtours << " subtours" << endl;
    //HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(best_neh.tour));
    best_neh.tour = HelperFunctions::StripNonCustomerNodesFromTour(best_neh.tour);

    //cout << "=-=-=-=-=-=-=-=-=-=-=-=-" << endl;
    ga_alg->AddSeedSolution(best_neh);

    solution best_ga;
    ga_alg->Optimize(best_ga);

    //cout << "GA best solution distance: " << best_ga.distance << " with " << best_ga.subtours << " subtours" << endl;
    //HelperFunctions::PrintTour(HelperFunctions::GetIndexEncodedTour(best_ga.tour));

    best_solution = best_ga;
}
