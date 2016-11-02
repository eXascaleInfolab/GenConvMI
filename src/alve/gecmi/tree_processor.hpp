
#ifndef ALVE__GECMI__TREE_PROCESSOR_HPP_
#define ALVE__GECMI__TREE_PROCESSOR_HPP_

#include <cstdint>

#include <boost/shared_ptr.hpp>

#include <alve/gecmi/combinator.hpp>
#include <alve/gecmi/confusion.hpp>
#include <alve/gecmi/outcome_reporter.hpp>

namespace alve { namespace gecmi { 

using std::size_t;


class tree_processor:
    public outcome_reporter
{
    struct pimpl_t; 
    pimpl_t* impl;
public:
    // m1, m2: number of modules in each partition. The 
    // built ambiguity matrix will include extra-entries
    // for storing the non-solved instance occurrences.
    //
    // repmap: A map from representant to its weight.
    tree_processor(size_t m1, size_t m2, 
        representant2appereances_map_t const& repmap);
    ~tree_processor();
    tree_processor( const tree_processor& other) = delete;

    virtual void report_success(
        tree_helper_ptr th,
        size_t m1,
        size_t m2 );
    virtual void report_fail(tree_helper_ptr th, int m1, int m2);
    virtual void end_count();

    //void report( tree_helper_ptr th_helper, size_t m1, size_t m2 );
    counter_matrix_t const& get_counter_matrix_ref() const;    
};

typedef boost::shared_ptr< tree_processor > tree_processor_ptr;

// 

}} // namespace alve::gecmi

#endif // ALVE__GECMI__TREE_PROCESSOR_HPP_

