
#include "tree_processor.hpp"

#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <iterator>

#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>

#include <alve/gecmi/combinator.hpp>
#include <alve/gecmi/confusion.hpp>

namespace alve { namespace gecmi {

struct tree_processor::pimpl_t{
    // All what this class do is to keep a table
    // with things to do
    typedef boost::tuples::tuple< 
        tree_helper_ptr, int, int > leaf_t;
    typedef std::vector< leaf_t > leaf_vector_t;

    // Original number of modules... I need these numbers
    // to populate the extra column-row in the counter matrix
    // dedicated to undecided events.
    size_t mp1;
    size_t mp2;

    // Leaves
    leaf_vector_t leaves;
    leaf_vector_t fail_leaves;

    // Counter matrix
    counter_matrix_t counter_matrix;
    representant2appereances_map_t repmap;

    // Constructor... needs the number of things
    // to put in the matrix... weird!
    pimpl_t( size_t mp1, size_t mp2, representant2appereances_map_t const& repmap ):
        mp1( mp1 ),
        mp2( mp2 ),
        //counter_matrix( mp1+1, mp2+1 ), ... on a second though...
        counter_matrix( mp1, mp2 ),
        repmap( repmap )
    {}

    // void print_tree() {{{
    // For debugging and publication purposes
    void print_tree( std::ostream& out )
    {
        out << "-- begin-print_tree --" << std::endl; 
        typedef std::set< tree_helper_ptr > 
            printed_nodes_set_t;
        printed_nodes_set_t printed_nodes;
        BOOST_FOREACH ( leaf_t const& leaf, leaves )
        {
            tree_helper_ptr th = leaf.get<0>();
            tree_helper_ptr parent = th->parent;
            while( th )
            {
                if ( printed_nodes.count( th ) == 0 )
                {
                    size_t uses = th->descend_by ? repmap[*(th->descend_by) ] : 0 ;
                    out << th.get() << " -> " << parent.get() 
                        << " uses=" << uses
                        << " chtotal=" << th->registered_uses
                        << " descend_by=" << th->descend_by 
                        << " " << ( ( th->op_generator == 1 ) ? "* " : "- "  )
                        << std::endl;
                    printed_nodes.insert( th );
                    th = parent;
                    if ( parent )
                    {
                        parent = th -> parent;
                    }
                } else {
                    break ;
                }
            } // end of while
        }
        out << "-- end-print_tree --" << std::endl;
    } // }}}

    // Function that does things
    // void report( th_helper, m1, m2 ) {{{
    void report_success( tree_helper_ptr th_helper, size_t m1, size_t m2 )
    {
        // Just add it to the table
        leaves.push_back(leaf_t{ th_helper, int(m1), int(m2) } );
    } // }}}

    // void report_fail() {{{
    void end_count() 
    {
        summarize();  
    } // }}}

    // void report_fail()  {{{
    void report_fail(tree_helper_ptr th_helper, int m1, int m2) 
    {
        fail_leaves.push_back( leaf_t{ th_helper, m1, m2} );
    }// }}}

    // void summarize() {{{
    void summarize()
    {
        // There is actually no difference, join all the leaves...
        std::copy( 
            fail_leaves.begin(), fail_leaves.end(), 
            std::back_inserter( leaves ) );
        
        // Here things are done. First decorate the nodes, 
        // in such a way that each node has its number of 
        // children. 
        count_ways();

        // Now, for each of the leaves, compute a weight and
        // register it on the matrix
        tree_helper_ptr th_helper;
        int m1, m2;
        BOOST_FOREACH ( auto& tp , leaves )
        {
            th_helper = tp.get<0>();
            m1 = tp.get<1>();
            m2 = tp.get<2>();

            double w = 1.0;
            size_t uses;
            // Yeap
            while ( th_helper )
            {
                uses = 
                    th_helper->descend_by ? repmap[*(th_helper->descend_by) ] : 1 ;

                if ( not th_helper )
                    break ;
                tree_helper_ptr parent = th_helper -> parent ;
                size_t ru = parent ? parent -> registered_uses : 1.0;

                w = w*uses / (double) ru;

                th_helper = parent;
            }
            
            // And influence
            // DEBUG
            std::cout << " p m1, m2, w " << m1 << " " << m2 << " " << w << std::endl;
            // end DEBUG
            
            if ( m1 != -1 and m2 != -1 )
                counter_matrix(m1, m2 ) += w;
        } // End of loop over leaves.

        // DEBUG
        //print_tree(std::cout);
        // end DEBUG

        // Final step: throw the kitchen sink
        leaves.clear();
    } // }}}

    // void count_ways() {{{
    //    Make each node to know how many children it has on
    //    an "aggregated" sense.
    void count_ways()
    {
        // Nothing really complicated
        std::set< tree_helper_ptr > visited_nodes_in_the_tree;

        BOOST_FOREACH( leaf_t & leaf, leaves )
        {
            // And up all the way
            tree_helper_ptr th = leaf.get<0>();
            tree_helper_ptr parent = th->parent;
            size_t uses = th->descend_by ? repmap[*(th->descend_by) ] : 0 ;
            while( th && (visited_nodes_in_the_tree.count(th) == 0) )
            {
                // Attending to the previous condition, this node has
                // never been visited.
                visited_nodes_in_the_tree.insert( th );
                if ( parent )
                {
                    parent->registered_uses += uses;
                }

                th = parent;
                if ( th )
                {
                    uses = th->descend_by ? repmap[*(th->descend_by) ] : 0 ;
                    parent = th->parent;
                }
            } // while
        }
    } // }}}
};
    
tree_processor::tree_processor(size_t m1, size_t m2,
    representant2appereances_map_t const& repmap ):
    impl( new pimpl_t(m1, m2, repmap) )
{
    
}

tree_processor::~tree_processor()
{
    delete impl;
}

void tree_processor::report_success( 
    tree_helper_ptr th_helper, size_t m1, size_t m2
    )
{
    impl->report_success( th_helper, m1, m2 );
}

void tree_processor::end_count( 
    )
{
    impl->end_count( );
}

void tree_processor::report_fail(tree_helper_ptr th, int m1, int m2) 
{
    impl->report_fail(th, m1, m2);
}

counter_matrix_t const& tree_processor::get_counter_matrix_ref() const
{
    return impl->counter_matrix;
}

}}// namespace alve::gecmi
