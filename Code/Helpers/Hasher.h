#pragma once
#include <memory>
#include <unordered_map>

#include "ProblemDefinition.h"

struct HashedSolution
{
    vector<shared_ptr<Node>> customer_route;
    vector<shared_ptr<Node>> full_route;
    float route_distance;
    shared_ptr<Node> next_node_after_route;
};

struct NodeVectorHash
{
    size_t operator()(const vector<shared_ptr<Node>>& nodes) const
    {
        size_t hash = 0;

        string id;
        for(const auto& node : nodes)
        {
            //hash ^= stringHash(node->unique_id) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            id += node->unique_id;
        }
        constexpr std::hash<string> stringHash;
        hash = stringHash(id);
        return hash;
    }
};

class Hasher
{
public:
    bool TryFindCachedSubtour(const vector<shared_ptr<Node>>& subtour, const shared_ptr<Node>& next_node, HashedSolution &out_hashed_solution);
    void CacheSubtour(const vector<shared_ptr<Node>>& customer_subtour, const vector<shared_ptr<Node>>& full_subtour, const shared_ptr<Node>& next_node, float distance);
    int GetLongestSubtour() const { return longest_memoized_subtour; }
    
private:
    unordered_map<vector<shared_ptr<Node>>, HashedSolution, NodeVectorHash> memo;
    int longest_memoized_subtour = 0;
};
