
#ifndef ALVE__GECMI__TREE_HELPER_HPP_
#define ALVE__GECMI__TREE_HELPER_HPP_   

namespace alve { namespace gecmi { 

struct tree_helper;

typedef boost::shared_ptr< tree_helper >
    tree_helper_ptr;

struct tree_helper{
    tree_helper_ptr parent;
    // To be populated later by the tree processor: aggregated
    // number of children this node has. By "aggregated" I mean
    // that any child node is counted as many times as 
    // there are entities with the representant that is causing
    // the descend.
    size_t registered_uses;
    // Flightweight for the set of modules remaining in
    // both state threads when this tree-helper was 
    // generated. That is, `descend_by' corresponds to 
    // the from-up link in the tree.
    representant_ofw_t descend_by;
    // Operation that generated the current tree:
    //    0 - None
    //    1 - Intersection
    //    2 - difference
    size_t op_generator;
};

}} // namespace alve::gecmi

#endif // ALVE__GECMI__TREE_HELPER_HPP_

