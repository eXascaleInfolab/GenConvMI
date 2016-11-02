
#include "cluster_reader.hpp"

#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/foreach.hpp>

namespace alve { namespace cluster_reader { 

    // Regular expressions {{{
    namespace detail { 

        using namespace boost::xpressive;

        sregex
            _comment = (* _s ) >> as_xpr("#") >> *( ~ _ln ) >> ! _ln ;
        sregex
            _white_line = (* _s ) >> ! _ln ;
        sregex
            _to_skip = ( _comment | _white_line );

        sregex le = !( as_xpr("\r\n") | as_xpr( "\r") );

        sregex rgx_header_line_is_module = 
            (* _s) >> as_xpr( "module" ) >> (* _s ) >> as_xpr(":") >> (*_s) >> as_xpr("vertices" );

        sregex rgx_header_line_is_vertex = 
            (* _s) >> as_xpr( "vertex" ) >> (* _s ) >> as_xpr(":") >> (*_s) >> as_xpr("modules" );

        sregex rgx_natural_number = + _d; 

        //sregex rgx_many_numbers = + ( s1 = rgx_natural_number );

        sregex rgx_numbers_capture = 
            (* _s) >> rgx_natural_number >> (* _s ) >> as_xpr(":") >> (*_s) >> 
                keep( * ( ( s1 = rgx_natural_number )  >> (* _s ) ) ) ;

    } // namespace detail }}}

// std::string get_line_skipping_comments(std::ostream& out) {{{
//     Return "false" if eof.
bool get_line_skipping_comments(std::istream& input, std::string& line)
{
    namespace xpr = boost::xpressive;
    using namespace detail; 

    getline( input, line );
    if ( input.eof() )
        return false ;
    xpr::smatch mo;
    bool match_ok 
        = xpr::regex_match( line, mo, _to_skip );
    while ( match_ok )
    {
        getline( input, line );
        if ( input.eof() )
            return false ;
        match_ok = xpr::regex_match( line, mo, _to_skip );
    }
    return true;
} // }}}

// void read_clusters_without_remappings( istream& input, input_interface& ) {{{
void read_clusters_without_remappings( 
    std::istream& input,
    input_interface& inp_interf )
{
    namespace xpr = boost::xpressive;
    using namespace detail; 

    // Get the first line with contents 
    using std::string;
    string line;
    get_line_skipping_comments(input, line );

    bool line_is_module = false;
    // Check the line adjusts to one or the other style
    xpr::smatch mo;
    bool match_ok = xpr::regex_match( line, mo, rgx_header_line_is_module );
    if (  match_ok )
    {
        line_is_module = true; 
    } else {
        match_ok = xpr::regex_match( line, mo, rgx_header_line_is_vertex );
        if ( match_ok )
        {
            line_is_module = false;
        } else
        {
            // The file must contain as first line one of:
            //     module: vertices
            // or
            //     vertex: modules
            throw std::runtime_error("NoHeaderInClustersFile");
        }
    }

    //
    bool more_lines = get_line_skipping_comments( input, line );
    while ( more_lines )
    {
        // Reading code
        match_ok = xpr::regex_match( line, mo, rgx_numbers_capture);

        if ( not match_ok )
        {
            throw std::runtime_error("ErrorReadingFile");
        }

        //size_t nhead = boost::lexical_cast< size_t >( mo[0].nested_results()[0] );
        //std::cout << mo.nested_results()[0] << std::endl;
        size_t i=0;
        size_t nhead = 0;
        BOOST_FOREACH (auto const& nr, mo.nested_results() )
        {
            //std::cout << nr[0] << std::endl;
            if ( i==0 )
            {
                nhead = boost::lexical_cast< size_t >( nr[0] );
            } else
            {
                size_t body_member = boost::lexical_cast< size_t >( nr[0] );
                if ( line_is_module )
                {
                    inp_interf.add_vertex_module( body_member, nhead );
                } else
                {
                    inp_interf.add_vertex_module( nhead, body_member );
                }
            }
            i++;
        }

        more_lines = get_line_skipping_comments( input, line );
    } // while 
} // Reader function }}}

}} // namespace alve::cluster_reader 
