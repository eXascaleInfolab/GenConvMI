
#ifndef ALVE__GECMI__DRIVER_HPP_
#define ALVE__GECMI__DRIVER_HPP_

#include <string>

#include <boost/function.hpp>
#include <boost/any.hpp>

#include <alve/gecmi/vertex_module_maps.hpp>
#include <alve/gecmi/branch_config.hpp>

namespace alve{ namespace gecmi {

double nmi_exact_driver_routine(
    vertex_module_bimap_t const& vmb1,
    vertex_module_bimap_t const& vmb2,
    branch_config const& bc = branch_config()
    );

typedef boost::function< void( std::string const&, boost::any const& ) >
    message_callback_t;

double nmi_approximate_routine( 
    std::string const& partition_1,
    std::string const& partition_2,
    double prec,
    message_callback_t message_callback
    );

}} // namespace alve::gecmi

#endif //  ALVE__GECMI__DRIVER_HPP_
