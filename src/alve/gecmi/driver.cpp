
#include "driver.hpp"

#include <iostream> 
#include <sstream>
#include <fstream> 
#include <map>

#include <boost/program_options.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/lexical_cast.hpp>

#include <alve/cluster_reader/cluster_reader.hpp>
#include <alve/gecmi/vertex_module_maps.hpp>
#include <alve/gecmi/representants.hpp>
#include <alve/gecmi/combinator.hpp>
#include <alve/gecmi/counts.hpp>
#include <alve/gecmi/confusion.hpp>
#include <alve/gecmi/tree_processor.hpp>
#include <alve/gecmi/deep_complete_simulator.hpp>
#include <alve/gecmi/bimap_cluster_populator.hpp>

namespace alve { namespace gecmi {

using boost::lexical_cast;
using std::string;
    
// double nmi_exact_driver_routine( vmb1, vmb2, branch_config ) {{{
double nmi_exact_driver_routine(
    vertex_module_bimap_t const& vmb1,
    vertex_module_bimap_t const& vmb2,
    branch_config const& bc
    )
{
    representant2appereances_map_t representants_map =
        extract_representants( vmb1, vmb2 );        

    // Get a set with the keys of the previous dictionary
    representant_set_t representants;
    BOOST_FOREACH( auto const& p, representants_map )
    {
        representants.insert( p.first );
    }

    //std::cout << "There are " << representants.size() << 
           //" representants" <<std::endl;

    // Get the vertices
    vertex_set_t vertices;
    BOOST_FOREACH( auto const& r, vmb1 )
    {
        vertices.insert( r.left ); 
    }
    // Now take each node and do the thing: to put the event on the
    // confusion matrix
    size_t mc1 = highest_module_plus_one( vmb1 );
    size_t mc2 = highest_module_plus_one( vmb2 );
    tree_processor_ptr tproc( 
        new tree_processor( mc1, mc2, representants_map ) );
    outcome_reporter_ptr reporter = 
        boost::dynamic_pointer_cast< outcome_reporter,
                                     tree_processor > ( tproc );


    BOOST_FOREACH (size_t vertex, vertices )
    {
        //std::cout << "Analizing vertex " << vertex << std::endl;
        modules_set_t ms1, ms2;
        for( auto t = vmb1.left.lower_bound( vertex ),
             t_end = vmb1.left.upper_bound(vertex);
             t != t_end ; t ++ )
        {
            ms1.insert( t->second );
        }
        for( auto t = vmb2.left.lower_bound( vertex ),
             t_end = vmb2.left.upper_bound(vertex);
             t != t_end ; t ++ )
        {
            ms2.insert( t->second );
        }
        combine( representants, ms1, ms2, reporter, bc );
    }

    counter_matrix_t const& cm = tproc->get_counter_matrix_ref();
    if ( bc.print_counter_matrix ) 
    {
        std::cout << cm << std::endl;
    }
    importance_matrix_t conf_real_mat;
    importance_vector_t row_acc, col_acc;
    normalize_events( cm, conf_real_mat, col_acc, row_acc );
    if ( bc.print_counter_matrix ) 
    {
        std::cout << conf_real_mat << std::endl;
    }
    importance_float_t b_nmi = normalized_mi( conf_real_mat, col_acc, row_acc );
    double nmi = importance_float_t( b_nmi );

    return nmi;
} // }}}

// double nmi_approximate_routine( string const& part1, string const& part2 ) {{{
double nmi_approximate_routine( 
    std::string const& partition_1,
    std::string const& partition_2,
    double prec,
    message_callback_t message_callback
    )
{
    two_relations_ptr trp( new two_relations_t() );

    bimap_cluster_populator bcp1(  trp->first );
    bimap_cluster_populator bcp2(  trp->second );

    std::stringstream ss1( partition_1 );
    std::stringstream ss2( partition_2 );

    //message_callback( "debug", partition_1 );
    //message_callback( "debug", partition_2 );

    try {

        read_clusters_without_remappings( ss1, bcp1 );
        read_clusters_without_remappings( ss2, bcp2 );

    } catch ( std::runtime_error e )
    {
        std::string msg( e.what() );
        boost::any a(msg);
        std::string head( "error" );
        message_callback( head, a );
        return 0.0;
    } 
    vertex_module_bimap_t const& vmb1 = trp->first;
    vertex_module_bimap_t const& vmb2 = trp->second;;
    //message_callback( "info", std::string("FileReadSuccess") );

    deep_complete_simulator dcs( trp ); 

    size_t rows = highest_module_plus_one( vmb1 );
    size_t cols = highest_module_plus_one( vmb2 );
    //message_callback( "debug", string("rows " ) + lexical_cast<string>(rows) );
    //message_callback( "debug", string("cols " ) + lexical_cast<string>(cols) );
    // And now just get enough samples for a while until they don't change too much.
    counter_matrix_t cmat; 
    // Take good care of initializing it to zero
    cmat = 
        boost::numeric::ublas::zero_matrix< importance_float_t >( rows, cols );

    double last_nmi = -1.0;
    double nmi = 0.0;

    // Now, just to test, let's take a few events

    while( fabs( nmi-last_nmi) > 0.1 )
    {
        for ( size_t i=0; i < 1000; i ++ )
        {
            simulation_result_t sr = dcs.get_sample();
            size_t i0 = sr.first;
            size_t i1 = sr.second;
            cmat( i0, i1 ) += sr.importance;
        }
        message_callback("debug", string("formatrix"));
        message_callback("parmatrix", cmat );

        importance_matrix_t norm_conf;
        importance_vector_t norm_cols;
        importance_vector_t norm_rows;

        normalize_events(   
            cmat, 
            norm_conf,
            norm_rows,
            norm_cols
        );

        last_nmi = nmi ;
        nmi = normalized_mi( 
            norm_conf,
            norm_rows,
            norm_cols
        );

        if ( nmi != nmi /*nan*/ )
        {
            // These silly cases are better handled as
            // zero.
            nmi = 0.0; 
        }

        // 
        //last_nmi = nmi;
    }
    return nmi;
} // nmi_approximate_routine }}}

}} // namespace alve::gecmi 

