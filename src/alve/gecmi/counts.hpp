
#ifndef ALVE__GECMI__COUNTS_HPP_
#define ALVE__GECMI__COUNTS_HPP_

#include <boost/foreach.hpp>

#include <alve/gecmi/vertex_module_maps.hpp>


namespace alve { namespace gecmi {

    // So, we can use this as a size.
    inline size_t highest_module_plus_one( vertex_module_bimap_t const& vmb )
    {
        size_t result = 0;
        BOOST_FOREACH( auto const& r, vmb )
        {
            if ( r.right > result ) result = r.right; 
        }
        return result+1;
    }

    // For getting the combinatorial size of the space, we need the actual
    // number of entities on game (any dictionary).
    inline size_t get_entity_count( vertex_module_bimap_t const& vmb )
    {
        size_t result = 0;
        int seeing = -1;
        BOOST_FOREACH( auto const& r, vmb.left )
        {
            if ( int( r.first ) > seeing ) 
            {
                seeing = int( r.first );
                result ++; 
            }
        }
        return result;
    }

}} // namespace alve::gecmi

#endif // ALVE__GECMI__COUNTS_HPP_

