#pragma once
#include <set>

#include "ProblemDefinition.h"

enum SolutionType
{
    DefaultSolution = -1
};

struct solution
{
    vector<shared_ptr<Node>> tour;
    float distance;
    int subtours;

    solution(vector<shared_ptr<Node>> t = {}, const float dist = DefaultSolution, const int subs = 1) : tour(std::move(t)), distance(dist), subtours(subs) {}

    bool operator==(const solution& other) const
    {
        //if the tours contain a different amount of nodes (maybe I'm storing the full tour, not just customer nodes)
        if(tour.size() != other.tour.size()) return false;
        
        for(size_t i = 0; i < tour.size(); i++)
        {
            //if I have a node that doesn't align with the other tour, we aren't the same
            if(tour[i] != other.tour[i]) return false;
        }

        //if we haven't returned false by now, we are probably the same
        return true;
    }

    
};

inline bool operator !=(const solution& me, const solution& other)
{
    return !(me == other);
}

struct CompareSolution
{
    bool operator()(const solution &a, const solution &b) const
    {
        if(a.subtours != b.subtours) return a.subtours < b.subtours;
        return a.distance < b.distance;
    }
};

class SolutionSet
{
public:
    SolutionSet()
    {
        solution_set.clear();
        sum_all_distances = 0.f;
        num_solutions = 0;
    }

    SolutionSet(const SolutionSet *other_solutions)
    {
        solution_set.clear();
        sum_all_distances = 0.f;
        num_solutions = 0;
        for(const auto& sol : other_solutions->solution_set)
        {
            AddSolutionToSet(sol);
        }
    }

    /*
    ~SolutionSet()
    {
        solution_set.clear();
    }
    */

    void CopyFrom(const SolutionSet* other)
    {
        solution_set.clear();
        sum_all_distances = 0.f;
        num_solutions = 0;
        for(const auto& sol : other->solution_set)
        {
            AddSolutionToSet(sol);
        }
    }
    
    solution GetBestSolution() const;
    solution GetRandomSolution() const;
    float GetMinimumDistance() const;
    float GetAverageDistance() const;
    void AddSolutionToSet(const solution &sol);
    int GetNumberOfSolutions() const { return num_solutions; }
    multiset<solution, CompareSolution> GetSolutionSet() const { return solution_set; }

private:
    multiset<solution, CompareSolution> solution_set;
    float sum_all_distances = 0.f;
    int num_solutions = 0;
    
};
