
/** 
 * This python module contains "high-level" functionality.
 * For testing and developing purposes, I will develop an alternative
 * "low level" module inside this one and accessible under the scope
 * "devel" of the current module.
 */

#include <boost/python.hpp>

#include <iostream>
using std::cout; using std::endl;
#include <sstream>

#include <boost/foreach.hpp>

#include <alve/gecmi/vertex_module_maps.hpp>
#include <alve/gecmi/driver.hpp>
#include <alve/gecmi/counts.hpp>
#include <alve/gecmi/deep_complete_simulator.hpp>
#include <alve/cluster_reader/cluster_reader.hpp>
#include <alve/gecmi/bimap_cluster_populator.hpp>
#include <alve/gecmi/calculate_till_tolerance.hpp>

extern "C" {
    #define PY_ARRAY_UNIQUE_SYMBOL _alvegecmi
    #include <numpy/libnumarray.h>
}

#include <numpy_boost.hpp>

namespace bp = boost::python;

// namespace alve::gecmi::py {{{
namespace alve { namespace gecmi { namespace py {

// bp::object get_module_counts_on_partitions( vmb1, vmb2 ) {{{
bp::object get_module_counts_on_partitions(
    vertex_module_bimap_t const& vmb1, 
    vertex_module_bimap_t const& vmb2 )
{
    size_t m1 = highest_module_plus_one( vmb1 );
    size_t m2 = highest_module_plus_one( vmb2 );
    return bp::make_tuple( m1, m2 );
} // }}}

void vmb_add_vertex_module( 
    vertex_module_bimap_t& vmb, 
    size_t vertex, 
    size_t module )
{
    vmb.insert( 
        vertex_module_bimap_t::relation( vertex, module ) );
}

vertex_module_bimap_t vmb_read_from_string( std::string const& s )
{
    using std::stringstream; 
    stringstream in( s );

    vertex_module_bimap_t vmb;
    bimap_cluster_populator  bcp( vmb );

    read_clusters_without_remappings(
        in,
        bcp
    );

    return vmb;
}

bp::object vmb_to_tuples( vertex_module_bimap_t const& vmb )
{
    bp::list l;
    BOOST_FOREACH ( relation_t const& rel, vmb )
    {
        l.append(
            bp::make_tuple( rel.left, rel.right )
        );
    }

    return l;
}

double py_nmi_exact_driver_routine( 
    vertex_module_bimap_t const& vmb1, 
    vertex_module_bimap_t const& vmb2 )
{
    return nmi_exact_driver_routine( vmb1, vmb2 );
}

double py_nmi_exact_driver_routine_with_depth( 
    vertex_module_bimap_t const& vmb1, 
    vertex_module_bimap_t const& vmb2,
    int max_depth
    )
{
    branch_config bc;
    bc.exploration_depth = max_depth;
    return nmi_exact_driver_routine( vmb1, vmb2, bc );
}

extern void event_generator_submodule();

bp::object calc_nmi_and_error(
    PyObject* rels0, 
    PyObject* rels1,
    double risk,
    double epvar
    )
{
    numpy_boost<double, 2> arr0( rels0 );
    numpy_boost<double, 2> arr1( rels1 );
    // 
    two_relations_ptr two_rel( new two_relations_t );

    if ( arr0.shape()[1] != 2 )
        throw std::runtime_error("Please provide a two-columns array (first argument)");

    if ( arr1.shape()[1] != 2 )
        throw std::runtime_error("Please provide a two-columns array (second argument)");

    for (size_t i=0; i < arr0.size(); i ++ )
    {
        two_rel->first.insert( 
            vertex_module_bimap_t::relation( arr0[i][0], arr0[i][1] ) 
        );
        // cout << "." << endl;
    }

    for (size_t i=0; i < arr1.size(); i ++ )
    {
        two_rel->second.insert( 
            vertex_module_bimap_t::relation( arr1[i][0], arr1[i][1] ) 
        );
        // C
    }
    calculated_info_t cit = calculate_till_tolerance( 
        two_rel, risk, epvar );

    //cout << "NMI: " << cit.nmi << endl;

    bp::dict result;
    result["nmi"] = cit.nmi;
    result["error"] = cit.empirical_variance;

    return result;
}

} } } // namespace alve::gecmi::py }}}


BOOST_PYTHON_MODULE( gecmi )
{
    using namespace bp; 
    using namespace alve::gecmi;
    using namespace alve::gecmi::py;

    []() -> void 
    {
        import_array();
    }();

    class_< vertex_module_bimap_t >("vertex_module_bimap_t" )
        .def("add_vertex_module", &vmb_add_vertex_module)
        // A function for interpreting an string as a vertex-module file specification.
        .def("read_from_string", &vmb_read_from_string)
        .staticmethod("read_from_string")
        .def("to_tuples", &vmb_to_tuples)
    ;

    def( "nmi_exact_driver_routine", &py_nmi_exact_driver_routine );
    def( "nmi_exact_driver_routine_with_depth", &py_nmi_exact_driver_routine_with_depth );

    def("get_module_counts_on_partitions", &get_module_counts_on_partitions );
    def("get_entity_count", &get_entity_count );
    def("calc_nmi_and_error", &calc_nmi_and_error, 
        "Simple function to calculate the nmi between two covers. Parameters: \n"
        "\n"
        "`cover1': 2-column array-convertible containing pairs (vertex,module), one per row \n"
        "`cover2': Second cover, in the same format than `cover1'\n"
        "`risk': Risk of getting a greater error than expected\n"
        "`error': Bound for the error \n");


    event_generator_submodule();
}

