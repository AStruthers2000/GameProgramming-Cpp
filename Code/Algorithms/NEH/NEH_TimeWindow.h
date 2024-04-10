#pragma once
#include "../Algorithm.h"


class NEH_TimeWindow : public Algorithm
{
public:
    NEH_TimeWindow(const shared_ptr<ProblemDefinition>& data, const shared_ptr<Hasher>& hasher) :
        Algorithm("NEH with Nearest Neighbor Subtours", data, hasher)
    {
        vector<string> hyper_parameters;
        hyper_parameters.emplace_back("None");
        SetHyperParameters(hyper_parameters);
    }

    void Optimize(solution &best_solution) override;

private:
    vector<vector<shared_ptr<Node>>> GenerateSubtours() const;
    vector<shared_ptr<Node>> GenerateSubtour() const;

    solution NEH_Optimize(vector<shared_ptr<Node>> subtour) const;
};
