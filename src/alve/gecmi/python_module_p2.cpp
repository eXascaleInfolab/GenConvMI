/** 
 * Part of the python module. Here we develop the event recorder
 * interface, that should be used with numpy from python to construct
 * matrices of events. 
 */

#include <boost/python.hpp>

#include <iostream>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/foreach.hpp>

#include <numpy/arrayobject.h>


#include <alve/gecmi/vertex_module_maps.hpp>
#include <alve/gecmi/deep_complete_simulator.hpp>
#include <alve/gecmi/counts.hpp>
#include <alve/gecmi/confusion.hpp>
#include <alve/gecmi/uncertaintity_volume.hpp>
#include <alve/gecmi/parallel_worker.hpp>

extern "C" {
    #define PY_ARRAY_UNIQUE_SYMBOL _alvegecmi
    #define NO_IMPORT_ARRAY 1
    #include <numpy/libnumarray.h>
}

namespace bp = boost::python;
using namespace tbb;

namespace alve { namespace gecmi { namespace py {

// deep_complete_simulator* build_dcs( vmb1, vmb2 ) {{{
deep_complete_simulator* build_dcs( 
    vertex_module_bimap_t const& vmb1, 
    vertex_module_bimap_t const& vmb2 )
{
    two_relations_ptr tr(
        new two_relations_t(vmb1, vmb2 ) );
    return new deep_complete_simulator( tr );
} // }}}

// bp::object dcs_get_sample(deep_complete_simulator & dcs ) {{{
bp::object dcs_get_sample(deep_complete_simulator & dcs ) {
    simulation_result_t result = dcs.get_sample();
    return bp::make_tuple( result.first, result.second, 
        result.importance, result.failed_attempts );
} // }}}

// bp::object dcs_get_n_events( dcs, n ) {{{
// A helper function that creates a list with events
bp::object dcs_get_n_events( 
    deep_complete_simulator& dcs,
    size_t n ) 
{
    bp::list result;
    //std::cout << "a " << std::endl;
    for( size_t i=0; i < n; i++ )
    {
        simulation_result_t sr = dcs.get_sample();
        result.append(
            bp::make_tuple( sr.first, sr.second, 
                sr.importance, sr.failed_attempts )
        );
        //std::cout << "ii " << i << std::endl;
    }

    return result;
} // }}}

typedef 
    std::vector< simulation_result_t > sr_vector_t;

// struct worker {{{
// Second, I will need several instances of the dcs to do 
// the work, so fork them... (fork function in deep_complete_simulator 
// takes care of doing a deterministic reseed of the randm 
// number generator...)
struct worker {
    deep_complete_simulator_uptr dcs_u;
    sr_vector_t& srs;
    worker( deep_complete_simulator& dcs, sr_vector_t& srs ):
        dcs_u( dcs.fork() ),
        srs( srs )
    {}
    worker( worker const& other):
        dcs_u( other.dcs_u->fork() ),
        srs( other.srs )
    {}
    void operator()( const blocked_range<size_t>& r ) const {
        for( size_t i=r.begin(); i != r.end(); ++i )
        {
            // Pure and safe memory access to (almost) unrelated
            // locations... (yet contigous, so cache might suffer...)
            //
            // TODO: Optimize here.
            simulation_result_t sr = dcs_u->get_sample();
            srs[i]= sr;
        }
    }
}; // }}}

// bp::object dcs_get_n_events_parallel(  dcs, n ) {{{
bp::object dcs_get_n_events_parallel( 
    deep_complete_simulator& dcs,
    size_t n )
{
    sr_vector_t srs( n );

    bool raised = false;
    Py_BEGIN_ALLOW_THREADS 

    try {
        parallel_for( blocked_range< size_t >( 0, n, 1000 ), worker( dcs, srs ) );
    } catch (tbb::tbb_exception const& e) {
        std::cout << "Catching !" << std::endl;
        raised = true;
    }

    Py_END_ALLOW_THREADS
    if ( raised )
    {
        throw std::runtime_error("SystemIsSuspiciuslyFailingTooMuch (maybe your partition is not solvable?)");
    }

    bp::list result;
    BOOST_FOREACH ( simulation_result_t const& sr, srs ){
        result.append( bp::make_tuple(sr.first, sr.second, sr.importance, sr.failed_attempts) );
    }

    return result;
} // }}}

// void get_and_assimilate_events_in_parallel(ev_count, cm ) {{{
void get_and_assimilate_events_in_parallel(
    deep_complete_simulator& dcs,
    size_t ev_count,
    counter_matrix_t& cm)
{
    bool raised = false;

    Py_BEGIN_ALLOW_THREADS 

    tbb::spin_mutex wait_for_matrix;

    try {
        parallel_for( 
            blocked_range< size_t >( 0, ev_count, 1000 ), 
            direct_worker< counter_matrix_t* >( dcs, &cm, &wait_for_matrix )
        );
    } catch (tbb::tbb_exception const& e) {
        std::cout << "Catching !" << std::endl;
        raised = true;
    }

    Py_END_ALLOW_THREADS
    if ( raised )
    {
        throw std::runtime_error("SystemIsSuspiciuslyFailingTooMuch (maybe your partition is not solvable?)");
    }
}// }}}

// counter_matrix_t* build_cm( size_t rows, size_t cols ) {{{
counter_matrix_t* build_cm( size_t rows, size_t cols )
{
    counter_matrix_t* result = new counter_matrix_t( rows, cols );
    *result = boost::numeric::ublas::zero_matrix< importance_float_t >( rows, cols );
    return result;
} // }}}

// void cm_assimilate_events( cm, event_strip ) {{{
void cm_assimilate_events(
    counter_matrix_t& cm,
    bp::list event_strip )
{
    size_t dims1 = bp::len( event_strip );
    // Populate the confusion matrix
    for( size_t i=0; i < dims1; i++ )
    {
        int i1 = bp::extract<int>(event_strip[i][0]);
        int i2 = bp::extract<int>(event_strip[i][1]);
        bp::extract< importance_float_t > ift_e( event_strip[i][2] );
        bp::extract< double > double_e( event_strip[i][2] );
        importance_float_t prob;
        if ( double_e.check() )
        {
            prob = (double)double_e;
        } else if ( ift_e.check() )
        {
            prob = ift_e ;
        }
        //std::cout << i1 << " " << i2 << " " << prob << std::endl;
        cm( i1, i2 ) += prob; // Check: possible problem with dimensions here
    }
} // }}}

// double cm_calculate_nmi( counter_matrix_t const& cm ) {{{
template< importance_float_t(*f)( importance_matrix_t const&,
                      importance_vector_t const&,
                      importance_vector_t const&) >
importance_float_t cm_calculate_nmi( counter_matrix_t const& cm )
{
    importance_matrix_t norm_conf;
    importance_vector_t norm_cols;
    importance_vector_t norm_rows;
    //std::cout << cm << std::endl;
    normalize_events( 
        cm, 
        norm_conf,
        norm_cols,
        norm_rows
        );
    //std::cout << norm_conf << std::endl;
    
    // Now time to calculate that mi thingy...
    importance_float_t mi = f( norm_conf, norm_cols, norm_rows ); 
    return mi;
} // }}}

// bp::object cm_get_normalized_matrix( counter_matrix_t const& cm ) {{{
bp::object cm_get_normalized_matrix( counter_matrix_t const& cm )
{
    importance_matrix_t norm_conf;
    importance_vector_t norm_cols;
    importance_vector_t norm_rows;
    //std::cout << cm << std::endl;
    normalize_events( 
        cm, 
        norm_conf,
        norm_cols,
        norm_rows
        );

    // Time to go numpy...
    int nd= 2; npy_intp dims[] = {cm.size1(), cm.size2()};
    PyObject* arr = PyArray_SimpleNew( nd, dims, NPY_DOUBLE );

    for( size_t i=0; i < cm.size1(); i ++ )
    {
        for( size_t j=0; j < cm.size2(); j++ )
        {
            npy_double& d = *( ( npy_double* )PyArray_GETPTR2(arr, i, j ) );
            d = norm_conf( i, j );
        }
    }

    bp::handle<> h(arr);
    bp::object result( h );
    return  result;
} // }}}

// bp::object cm_get_normalized_matrix_with_fails( cm, fails  ) {{{
bp::object cm_get_normalized_matrix_with_fails( 
    counter_matrix_t const& cm,
    importance_float_t const& fails
    )
{
    importance_matrix_t norm_conf;
    //std::cout << cm << std::endl;
    normalize_events_with_fails( 
        cm, 
        fails,
        norm_conf
        );

    // Time to go numpy...
    int nd= 2; npy_intp dims[] = {cm.size1(), cm.size2()};
    PyObject* arr = PyArray_SimpleNew( nd, dims, NPY_DOUBLE );

    for( size_t i=0; i < cm.size1(); i ++ )
    {
        for( size_t j=0; j < cm.size2(); j++ )
        {
            npy_double& d = *( ( npy_double* )PyArray_GETPTR2(arr, i, j ) );
            d = norm_conf( i, j );
        }
    }

    bp::handle<> h(arr);
    bp::object result( h );
    return  result;
} // }}}

// bp::object cm_get_matrix( counter_matrix_t const& cm ) {{{
bp::object cm_get_matrix( 
    counter_matrix_t const& cm
    )
{
    // Time to go numpy...
    int nd= 2; npy_intp dims[] = {cm.size1(), cm.size2()};
    PyObject* arr = PyArray_SimpleNew( nd, dims, NPY_DOUBLE );

    for( size_t i=0; i < cm.size1(); i ++ )
    {
        for( size_t j=0; j < cm.size2(); j++ )
        {
            npy_double& d = *( ( npy_double* )PyArray_GETPTR2(arr, i, j ) );
            d = cm( i, j );
        }
    }

    bp::handle<> h(arr);
    bp::object result( h );
    return  result;
} // }}}

// bp::object cm_calculate_nmi_and_mvar( counter_matrix_t const& cm, double prob ) {{{
bp::object cm_calculate_nmi_and_mvar( counter_matrix_t const& cm, double prob )
{
    importance_matrix_t norm_conf;
    importance_vector_t norm_cols;
    importance_vector_t norm_rows;
    size_t total_events = total_events_from_unmi_cm( cm );
    normalize_events( 
        cm, 
        norm_conf,
        norm_cols,
        norm_rows
        );
    importance_float_t nmi;
    importance_float_t max_var;
    variances_at_prob( 
        norm_conf, norm_cols, norm_rows, 
        total_events,
        prob,
        max_var, 
        nmi
        ); 

    return bp::make_tuple( nmi, max_var );
} // }}}

// uncertaintity_estimation_ptr cm_get_uncertaintity_estimation( cm, prob_threshold ) {{{
boost::shared_ptr< uncertaintity_estimation_t > cm_get_uncertaintity_estimation(
	counter_matrix_t const& cm,
	double prob_threshold
	)
{
    importance_matrix_t norm_conf;
    importance_vector_t norm_cols;
    importance_vector_t norm_rows;
    normalize_events(
        cm,
        norm_conf,
        norm_cols,
        norm_rows
        );
    // Let's calculate the total events by hand
    size_t total_events = 0;
    size_t row_cnt = cm.size1();
    size_t col_count= cm.size2();
    for( size_t i=0; i < row_cnt; i++)
    {
    	for ( size_t j=0; j < col_count; j++)
    	{
    		total_events += cm(i,j);
    	}
    }
    boost::shared_ptr< uncertaintity_estimation_t > result( new uncertaintity_estimation_t);
    get_uncertaintity_estimation(cm, norm_conf, total_events, prob_threshold, *result);
    return result;
} // }}}

void event_generator_submodule() {
    using namespace bp; 
    using namespace alve::gecmi;
    using namespace alve::gecmi::py;

    []() -> void 
    {
        import_array();
    }();

    bp::object package = bp::scope();
    package.attr("__path__") = "gecmi";
    bp::object devel_module(
        bp::handle<>(bp::borrowed(PyImport_AddModule("gecmi.event_generator"))));
    bp::scope().attr("event_generator") = devel_module;
    bp::scope devel_scope = devel_module;

    class_< deep_complete_simulator, boost::noncopyable >( 
        "deep_complete_simulator", bp::no_init )
        .def("__init__", bp::make_constructor(&build_dcs) )
        .def("get_sample", &dcs_get_sample )
        .def("set_seed", &deep_complete_simulator::set_seed )
        .def("get_n_events", &dcs_get_n_events )
        .def("get_n_events_parallel", &dcs_get_n_events_parallel )
        .def("get_and_assimilate_events_in_parallel", &get_and_assimilate_events_in_parallel)
    ;

    class_< counter_matrix_t >("counter_matrix_t", 
        bp::no_init )
        .def("__init__", bp::make_constructor(&build_cm) )
        .def("assimilate_events", &cm_assimilate_events )
        .def("calculate_nmi", &cm_calculate_nmi<normalized_mi> )
        .def("calculate_umi", &cm_calculate_nmi<unnormalized_mi> )
        .def("calculate_nmi_and_mvar", &cm_calculate_nmi_and_mvar )
        .def("get_matrix", &cm_get_matrix)
        .def("get_normalized_matrix", &cm_get_normalized_matrix )
        .def("get_normalized_matrix_with_fails", &cm_get_normalized_matrix_with_fails )
        .def("get_uncertaintity_estimation", &cm_get_uncertaintity_estimation)
    ;

    class_< uncertaintity_estimation_t, boost::shared_ptr<uncertaintity_estimation_t> >("uncertaintity_estimation_t")
		.def_readwrite("volume", &uncertaintity_estimation_t::volume)
		.def_readwrite("equivalent_side", &uncertaintity_estimation_t::equivalent_side)
		.def_readwrite("max_side", &uncertaintity_estimation_t::max_side)
	;

}

}}} // namespace alve::gecmi::py



