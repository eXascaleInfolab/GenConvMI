
#include "combinator.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <list>

#include <boost/foreach.hpp>


namespace alve { namespace gecmi { 

// void set_intersect( s1, s2, out_s ) {{{
void set_intersect(
    std::set< size_t > const& s1,
    std::set< size_t > const& s2,
    std::set< size_t >& s )
{
    std::set_intersection(
        s1.begin(), s1.end(),
        s2.begin(), s2.end(),
        std::inserter( s, s.begin() )
    );
} // }}}

// void set_diff( s1, s2, out_s ) {{{
void set_diff(
    std::set< size_t > const& s1,
    std::set< size_t > const& s2,
    std::set< size_t >& s )
{
    std::set_difference(
        s1.begin(), s1.end(),
        s2.begin(), s2.end(),
        std::inserter( s, s.begin() )
    );
} // }}}

// representant_set_t filter_by( original, ms1, ms2 ) {{{
// Filter sets to take only the relevant entries
representant_set_t filter_by(
    representant_set_t const& original,
    modules_set_t const& ms1,
    modules_set_t const& ms2 )
{
    representant_set_t result; 
    modules_set_t ms;
    BOOST_FOREACH (representant_fw_t const& rfw, original )
    {
        modules_set_t const& rs1 = rfw.get().first;
        modules_set_t const& rs2 = rfw.get().second;

        if ( rs1 == ms1 and rs2 == ms2 )
        {
            // Don't insert a representant that 
            // brings no new information.
            continue ;
        }

        set_intersect( rs1, ms1,ms );
        if ( ms.size() )
        {
            result.insert( rfw );
        }
        ms.clear();
        set_intersect( rs2, ms2,ms );
        if ( ms.size() )
        {
            result.insert( rfw );
        }
        ms.clear();
    }
    std::cout << result << std::endl;
    return result;
} // }}}

// struct command_state {{{
struct command_state{
    bool do_intersection; // otherwise do difference
    // Remaining modules to disambiguate
    modules_set_t ms1;
    modules_set_t ms2;
    // Remaining representants
    representant_set_t representants;
    // Tree helper corresponding to this state
    tree_helper_ptr tr_helper; 
    // Deep of this node
    size_t depth;
}; // }}}

typedef std::list< command_state >
    command_stack_t;

// void expand_command( cmd, cstack, reporter, bc ) {{{
//
//     The do-nothing-if-combination-already-seen is 
//     implicit on the weighting police coded here. 
//     It is not implicit on the simulator though...
//     There is controlled by the tracker
//
void expand_command(
    command_state const& cmd,
    command_stack_t& cstack,
    outcome_reporter_ptr const& reporter,
    branch_config const& bc
    )
{
    if ( ( cmd.ms1.size() == 1 ) 
        and
         ( cmd.ms2.size() == 1 ) )
    {
        // Solved ! 
        //
        //   NOTE: This report get's done twice.
        size_t m1 = *cmd.ms1.begin();
        size_t m2 = *cmd.ms2.begin();
        //std::cout << " reported " << m1 << " " << m2 << std::endl;
        reporter->report_success( cmd.tr_helper, m1, m2 );
    } else if( cmd.representants.size() == 0 )
    {
        if ( bc.abort_on_ambigous )
        {
            throw std::runtime_error("Unsolvable");
        } else
        {
            reporter->report_fail( cmd.tr_helper, 0, 0 );
        }
    } else if ( cmd.ms1.size() == 0 )
    {
        size_t m2 = *cmd.ms2.begin();
        reporter->report_fail( cmd.tr_helper, -1, m2 );
    } else if ( cmd.ms2.size() == 0 )
    {
        size_t m1 = *cmd.ms1.begin();
        reporter->report_fail( cmd.tr_helper, m1, -1 );
    }
    else{
        // Reproduce the commands
        size_t rem_reps_count = cmd.representants.size();
        BOOST_FOREACH(  representant_fw_t const& rep_fw, cmd.representants )
        {
            representant_set_t new_rep_set( cmd.representants );
            new_rep_set.erase( rep_fw );

            modules_set_t nms1;
            if ( cmd.ms1.size() > 1 )
            {
                // Do the command
                if ( cmd.do_intersection )
                {
                    set_intersect(
                        rep_fw.get().first,
                        cmd.ms1,
                        nms1
                    );
                } else
                {
                    set_diff(
                        cmd.ms1,
                        rep_fw.get().first,
                        nms1
                    );
                }
            } // if module not ready
            else  {
                nms1 = cmd.ms1;
            }
            //std::cout << "nms1 " << nms1 << std::endl;

            modules_set_t nms2;
            if ( cmd.ms2.size() > 1 )
            {
                // Do the command
                if ( cmd.do_intersection )
                {
                    set_intersect(
                        rep_fw.get().second,
                        cmd.ms2,
                        nms2
                    );
                } else
                {
                    set_diff(
                        cmd.ms2,
                        rep_fw.get().second,
                        nms2
                    );
                }
            } // if module not ready
            else  {
                nms2 = cmd.ms2;
            }
            //std::cout << "nms2 " << nms1 << std::endl;

            // DEBUG
            //if ( cmd.depth >= 2 )
            //{
                //throw std::runtime_error("Too deep"); 
            //}
            // end DEBUG

            cstack.push_front( command_state{
                true,
                nms1,
                nms2,
                new_rep_set,
                tree_helper_ptr( new tree_helper({cmd.tr_helper,0, rep_fw, 1}) ),
                cmd.depth+1
            });
            cstack.push_front( command_state{
                false,
                nms1,
                nms2,
                new_rep_set,
                tree_helper_ptr( new tree_helper({cmd.tr_helper,0, rep_fw, 2}) ),
                cmd.depth+1
            });
        }
    }
} // }}}

// void combine( rep_set, ms1, ms2, reporter ) {{{
//    This function reports modules for the given representant_set_t
//    and the modules to resolve...
void combine( 
    representant_set_t const& rep_set,
    modules_set_t const& ms1,
    modules_set_t const& ms2,
    outcome_reporter_ptr const& reporter , 
    branch_config const& bc
    )
{
    // Obtain a filtered version of representants
    representant_set_t start_rep_set =
        filter_by( rep_set, ms1, ms2 );
    //std::cout << "start_rep_set size: " << start_rep_set.size() << std::endl;
    //std::cout << "ms1 " << ms1 << " ms2 " << ms2 << std::endl;
    // Create the primal command
    command_stack_t stack;
    command_state primal_command_1{
       true, // <-- The operation to do
       ms1,
       ms2,
       start_rep_set,
       tree_helper_ptr( new tree_helper( {tree_helper_ptr(),0, representant_ofw_t(), 1 } ) ),
       0
    };
    command_state primal_command_2{
       false, // <-- The operation to do
       ms1,
       ms2,
       start_rep_set,
       tree_helper_ptr( new tree_helper( {tree_helper_ptr(),0,representant_ofw_t(), 2}) ),
       0
    };
    stack.push_front( primal_command_1 );
    stack.push_front( primal_command_2 );
    //size_t counter = 0;
    while( stack.size() > 0 )
    {
        command_state cmd = stack.front();
        //std::cout << "exploration depth: " << bc.exploration_depth << std::endl;
        stack.pop_front();
        if ( ( bc.exploration_depth != -1 ) 
            and ( cmd.depth >= bc.exploration_depth ) )
        {
            // Do not expand this command, forget about it...
        } else
        {
            //std::cout << "depth: " << cmd.depth << std::endl;
            expand_command(
                cmd,
                stack,
                reporter,
                bc
            );
        }
        //counter += 1;
    }
    //std::cout << "Commands executed: " << counter << std::endl;
    // Command issued to the reporter to say him that
    // the current vertex has been finished and the changes
    // can be commited to the confusion matrix.
    reporter->end_count( );
} // }}}

}} // namespace alve::gecmi
