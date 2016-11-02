
#ifndef ALVE__CLUSTER_READER__CLUSTER_READER_HPP_
#define ALVE__CLUSTER_READER__CLUSTER_READER_HPP_

#include <boost/shared_ptr.hpp>

namespace alve { namespace cluster_reader { 

// Input interface...
class input_interface {
public:
    virtual void add_vertex_mapping( int internal_vertex_id, int internal_module_id) = 0;
    virtual void add_module_mapping( int internal_vertex_id, int internal_module_id) = 0;
    virtual void add_vertex_module( int internal_vertex_id, int module_id ) = 0; 
};

// A test mock
class mock_input_processor : public input_interface {
    virtual void add_vertex_mapping( int internal_vertex_id, int internal_module_id)
    {};
    virtual void add_module_mapping( int internal_vertex_id, int internal_module_id)
    {};
    virtual void add_vertex_module( int internal_vertex_id, int module_id )
    {};
};

typedef boost::shared_ptr< input_interface > 
    input_interface_ptr;

// The name of this method reflects the fact that remappings
// are not made in the numbers of vertices and modules
void read_clusters_without_remappings( 
    std::istream& input,
    input_interface& inp_interf );

}} // namespaces

#endif //


