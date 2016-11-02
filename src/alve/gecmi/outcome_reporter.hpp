
#ifndef ALVE__GECMI__OUTCOME_REPORTER_HPP_
#define ALVE__GECMI__OUTCOME_REPORTER_HPP_

#include <boost/shared_ptr.hpp>

#include <alve/gecmi/tree_helper.hpp>

namespace alve {  namespace gecmi {

// A leaf tree helper, if empty the tree processor should
// complete the processing for the tree and restart itself.
//
// The function is called only on leaves.
class outcome_reporter{
public:
    virtual void report_success(
        tree_helper_ptr th,
        size_t m1,
        size_t m2 ) = 0;
    // Using integers here to signify that the special values -1 
    // should be translated to the nothing-symbols.
    virtual void report_fail(tree_helper_ptr th, int m1, int m2)=0;
    virtual void end_count()=0;
    virtual ~outcome_reporter(){};
};

typedef boost::shared_ptr< outcome_reporter> 
    outcome_reporter_ptr;

}}

#endif // ALVE__GECMI__OUTCOME_REPORTER_HPP_
