
#ifndef ALVE__GECMI__BRANCH_CONFIG_HPP_
#define ALVE__GECMI__BRANCH_CONFIG_HPP_

namespace alve { namespace gecmi { 

// Structure with decisions for exploration 
// depth; weighting, and if either to stop 
// the exploration when a dead-end branch is 
// found. 
struct branch_config{
    // -1 for infinite.
    int exploration_depth;

    // TODO: weighting scheme?

    // What to do when the combinator gets to a
    // no-more-remaining representants set. 
    bool abort_on_ambigous;

    // Should I print the conter_matrix? Could be 
    // usefull for some scenarios
    bool print_counter_matrix;

    // What to do when the combinator gets 
    // empty subsets ms1 and ms2 ...? 
    // what: always ignore. This is a pretty 
    // natural situation.

    // The default constructor 
    branch_config():
        exploration_depth( -1 ),
        abort_on_ambigous( false ),
        print_counter_matrix( false )
    {}
};

}} // namespace alve::gecmi //

#endif // ALVE__GECMI__BRANCH_CONFIG_HPP_

