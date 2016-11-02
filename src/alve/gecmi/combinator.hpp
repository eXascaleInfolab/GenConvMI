
#ifndef ALVE__GECMI__COMBINATOR_HPP_
#define ALVE__GECMI__COMBINATOR_HPP_

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <alve/gecmi/representants.hpp>
#include <alve/gecmi/branch_config.hpp>
#include <alve/gecmi/outcome_reporter.hpp>
#include <alve/gecmi/tree_helper.hpp>

namespace alve{ namespace gecmi {

void combine( 
    representant_set_t const& rep_set,
    modules_set_t const& ms1,
    modules_set_t const& ms2,
    outcome_reporter_ptr const& reporter,
    branch_config const& bc = branch_config()
    );

}} // namespace alve::gecmi

#endif // ALVE__GECMI__COMBINATOR_HPP_

