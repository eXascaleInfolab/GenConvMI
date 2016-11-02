
#ifndef ALVE__GECMI__CALCULATE_TILL_TOLERANCE_HPP_
#define ALVE__GECMI__CALCULATE_TILL_TOLERANCE_HPP_ 

#include <utility>

#include <alve/gecmi/vertex_module_maps.hpp>

namespace alve { namespace gecmi {
    
struct calculated_info_t {
    double empirical_variance;
    double nmi;
};

calculated_info_t calculate_till_tolerance(
    two_relations_ptr two_rel,
    double risk , // <-- Upper bound of probabibility of the true value being 
                  //  -- farthest from estimated value than the epvar
    double epvar
    );

}} // alve::gecmi

#endif // ALVE__GECMI__CALCULATE_TILL_TOLERANCE_HPP_ 

