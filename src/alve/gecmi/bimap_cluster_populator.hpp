
#ifndef  ALVE__GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_
#define  ALVE__GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_

#include <alve/cluster_reader/cluster_reader.hpp>

namespace alve { namespace gecmi {

// class bimap_cluster_populator {{{
class bimap_cluster_populator:
    public alve::cluster_reader::mock_input_processor 
{
    vertex_module_bimap_t& vmb;
public:
    typedef vertex_module_bimap_t::relation relation_t;
    bimap_cluster_populator( vertex_module_bimap_t& vmb ):
        vmb(vmb)
    {}

    virtual void add_vertex_module( int internal_vertex_id, int module_id )
    {
        vmb.insert( relation_t( internal_vertex_id, module_id ) );
    };
   
}; // class bimap_cluster_populator }}}

}} // namespace alve::gecmi

#endif // ALVE__GECMI__BIMAP_CLUSTER_CALCULATOR_HPP_

