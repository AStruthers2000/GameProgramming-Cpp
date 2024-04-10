#include "Hasher.h"

bool Hasher::TryFindCachedSubtour(const vector<shared_ptr<Node>>& subtour, const shared_ptr<Node>& next_node,
                                  HashedSolution& out_hashed_solution)
{
    bool found_cache_hit = false;
    const auto it = memo.find(subtour);
    if(it!=memo.end())
    {
        //matching record for subtour found

        //if the matched record also tried to visit the next node, we know that we have a perfect match
        if(it->second.next_node_after_route == next_node)
        {
            out_hashed_solution = it->second;
            found_cache_hit = true;
        }
    }

    return found_cache_hit;
}

void Hasher::CacheSubtour(const vector<shared_ptr<Node>>& customer_subtour,
    const vector<shared_ptr<Node>>& full_subtour, const shared_ptr<Node>& next_node, float distance)
{
    HashedSolution new_solution;
    new_solution.customer_route = customer_subtour;
    new_solution.full_route = full_subtour;
    new_solution.next_node_after_route = next_node;
    new_solution.route_distance = distance;

    longest_memoized_subtour = max(longest_memoized_subtour, static_cast<int>(new_solution.customer_route.size()));
    memo[customer_subtour] = new_solution;
}
