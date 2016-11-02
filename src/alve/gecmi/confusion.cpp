
#include "confusion.hpp"

#include <iostream>

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

// #include <boost/math/distributions/binomial.hpp>
#include <boost/math/special_functions/beta.hpp>

using std::cout;
using std::endl;


namespace alve { namespace gecmi { 

    // void normalize_events( cm, out_double_mat, out_cols, out_rows ) {{{
    //    Here `normalized' means just "divided by the total so that sum 
    //    of frequencies/probabilities be one". It doesn't have anything to 
    //    do with entropies.
    void normalize_events( counter_matrix_t const& cm, 
        importance_matrix_t& out_norm_conf,
        importance_vector_t& out_norm_cols,
        importance_vector_t& out_norm_rows
        )
    {
        // First I need to calculate the total number of events
        importance_float_t total_events = 0.0;
        BOOST_FOREACH( auto int2size , cm.data() )
        {
            total_events += int2size.second ;
        }
        // That was easy... now I need to reallocate
        // the matrix dimensions on the output
        out_norm_conf.resize( cm.size1(), cm.size2() , false );
        out_norm_conf.clear();

        // And finally put the numbers there... again,
        // easy enough
        BOOST_FOREACH( auto int2size , cm.data() )
        {
            importance_float_t p = int2size.second / total_events ;
            out_norm_conf.data()[ int2size.first ] = p ;
        }

        // Now time to populate the cols and rows vectors
        out_norm_cols.resize( cm.size2() );
        out_norm_cols.clear();
        out_norm_rows.resize( cm.size1() );
        out_norm_rows.clear();
        size_t col_count = cm.size2();
        BOOST_FOREACH( auto int2p , out_norm_conf.data() )
        {
            importance_float_t p = int2p.second ;
            size_t g = int2p.first;
            // g = i*n+j
            size_t j = g % col_count;
            size_t i = g / col_count;
            out_norm_cols(j) += p ;
            out_norm_rows(i) += p ;
        }
    } // }}}

    // void normalize_events_with_fails( int_mat, out_double_mat ) {{{
    //    The purpose of this function is just to assist on debugging, it is 
    //    not using for computing the results
    void normalize_events_with_fails( counter_matrix_t const& cm, 
        importance_float_t const& fail_count,
        importance_matrix_t& out_norm_conf
        )
    {
        // First I need to calculate the total number of events
        importance_float_t total_events = fail_count;
        BOOST_FOREACH( auto int2size, cm.data() )
        {
            total_events += int2size.second ;
        }

        //cout << "cm " << cm << endl;
        //cout << "fail count " << fail_count << endl;
        //cout << "total evs " << total_events << endl;
        // That was easy... now I need to reallocate
        // the matrix dimensions on the output
        out_norm_conf.resize( cm.size1(), cm.size2() , false );
        out_norm_conf.clear();

        // And finally put the numbers there... again,
        // easy enough
        BOOST_FOREACH( auto int2size, cm.data() )
        {
            importance_float_t p = int2size.second / total_events ;
            out_norm_conf.data()[ int2size.first ] = p ;
        }

    } // }}}

    // importance_float_t zlog( importance_float_t x ) {{{
    importance_float_t zlog( importance_float_t x ) 
    {
        if ( x == 0.0)
        {
            return 0.0;
        } else
        {
            return log( x ) / log( 2 );
        }
    } // }}}

    // importance_float_t unnormalized_mi( norm_conf, norm_cols, norm_rows ) {{{
    //    Input matrix and columns are normalized with respect to probabilities, 
    //    so that all sum one; not divided by semi-sum of entropies or anything 
    //    like that.
    importance_float_t unnormalized_mi( 
        importance_matrix_t const& norm_conf,
        importance_vector_t const& norm_cols,
        importance_vector_t const& norm_rows
    )
    {
        // First the unnormalized mutual information... shouldnt
        // be too hard
        importance_float_t ni = 0.0;
        size_t col_count = norm_conf.size2();
        BOOST_FOREACH( auto int2p, norm_conf.data() )
        {
            importance_float_t p = int2p.second ;
            size_t g = int2p.first;
            // g = i*n+j
            size_t j = g % col_count;
            size_t i = g / col_count;
            ni += p*zlog( 
                p / (
                    norm_cols(j) * norm_rows(i)
                )
            );
        }
        return ni;
    } // }}}

    // importance_float_t normalized_mi( norm_conf, norm_cols, norm_rows ) {{{
    //    Input matrix and columns are normalized with respect to probabilities, 
    //    so that all sum one; not divided by semi-sum of entropies or anything 
    //    like that. But we do divide for returning the result in this function.
    importance_float_t normalized_mi( 
        importance_matrix_t const& norm_conf,
        importance_vector_t const& norm_cols,
        importance_vector_t const& norm_rows
    )
    {
        // First the unnormalized mutual information... shouldnt
        // be too hard
        importance_float_t ni = 0.0;
        size_t col_count = norm_conf.size2();
        BOOST_FOREACH( auto int2p, norm_conf.data() )
        {
            importance_float_t p = int2p.second ;
            size_t g = int2p.first;
            // g = i*n+j
            size_t j = g % col_count;
            size_t i = g / col_count;
            // Rows are indexed using i, so, norm_rows are the 
            // marginal probabilities of all the rows, and we can visualize
            // it as a column vector. Similarly, we can visualize norm_cols as
            // a row vector (although it is the sum, column-wize, of all 
            ni += p*zlog( 
                p / (
                    norm_cols(j) * norm_rows(i)
                )
            );
        }
        // Now the entropies and the normalization
        importance_float_t H0 = 0.0;
        double s = 0.0;
        BOOST_FOREACH( importance_float_t p, norm_cols )
        {
            H0 -= p * zlog( p );
            s += p;
        }
        //cout << "H0: " << H0 << " s " << s << endl;
        importance_float_t H1 = 0.0;
        s = 0.0;
        BOOST_FOREACH ( importance_float_t p, norm_rows )
        {
            H1 -= p * zlog( p );
            s += p;
        }
        //cout << "H1: " << H1 << " s " << s << endl;
        importance_float_t result = 2*ni / ( H0 + H1 );
        return result;
    } // }}}

    // void variances_at_prob( nconf, ncols, nrows, n, threshold_prob, &out_max_variance, &out_nmi ) {{{
    void variances_at_prob( 
        importance_matrix_t const& norm_conf,
        importance_vector_t const& norm_cols,
        importance_vector_t const& norm_rows,
        size_t total_events,
        double prob,
        double & out_max_variance,
        double & out_nmi
    )
    {
        // Get a binomial distribution in place.
        //using boost::math::binomial;

        // Start with zero
        out_max_variance = 0;

        // Now I need to go as if yo calculate the mutual information completely. 
        // But then I also need to keep intermediate magnitudes so that 
        // I can go back and forth easily.

        // First the unnormalized mutual information... shouldnt
        // be too hard
        importance_float_t ni = 0.0;
        size_t col_count = norm_conf.size2();


        BOOST_FOREACH( auto int2p, norm_conf.data() )
        {
            importance_float_t p = int2p.second ;
            
            //check_total_one += p;

            size_t g = int2p.first;
            // g = i*n+j
            size_t j = g % col_count;
            size_t i = g / col_count;
            // Rows are indexed using i, so, norm_rows are the 
            // marginal probabilities of all the rows, and we can visualize
            // it as a column vector. Similarly, we can visualize norm_cols as
            // a row vector (although it is the sum, column-wize, of all 
            ni += p*zlog( 
                p / (
                    norm_cols(j) * norm_rows(i)
                )
            );
        }


        // Now the entropies
        double s = 0.0;
        double check_total_one = 0.0;
        importance_float_t H0 = 0;
        BOOST_FOREACH( importance_float_t p, norm_cols )
        {

            H0 -= p * zlog( p );
            s += p;
        }

        importance_float_t H1 = 0.0;
        s = 0.0;
        BOOST_FOREACH ( importance_float_t p, norm_rows )
        {
            H1 -= p * zlog( p );
            s += p;
        }

        importance_float_t unmi = ni;
        importance_float_t nmi = unmi / std::max( H0 , H1 );

        // Let's populate a vector with all the increments of the
        // y according to the variation of the x.... of course this 
        // vector will be big.
        size_t alpha_size = norm_conf.size1() * norm_conf.size2();

        importance_float_t s2 = 0.0;

        // Now I'm goint fo calculate the error components...
        BOOST_FOREACH( auto int2p, norm_conf.data() )
        {
            // This prob is what is
            importance_float_t p = int2p.second ;
            int64_t success_count =  p * total_events ;

            //binomial bn( total_events, p );
            //  This is  the lower point where with probability
            // `prob' we can go. 
            //double pp = quantile( bn, prob ) / total_events;

            // To understand this formula please check "more_about_the_error.nb"
            double pp = 1.0 - boost::math::ibeta_inv( 
                int64_t(total_events) - success_count,
                success_count + 1,
                prob );

            //std::cout <<
                //boost::format( "total_events %1% success_count %2% prob %3%" ) 
                    //% total_events 
                    //% success_count 
                    //% prob
                //<< std::endl;

            //double pp = quantile( bn, 0.50 ) / total_events;  // <--- For verification
            //std::cout << "p : " << p << " pp: " << pp << std::endl;
            
            // 'g' is the general index inside the data array of the matrix.
            size_t g = int2p.first;
            // g = i*col_count + j  <-- That's how it is related to rows and cols
            size_t j = g % col_count;
            size_t i = g / col_count;
            // Now I need to calculate what would be if 
            // we use the quantile...
            importance_float_t h0used = norm_cols( j );
            importance_float_t h1used = norm_rows( i );
            // Anulate old value used there... note inversion
            // of signs
            importance_float_t h0 = H0 + h0used * zlog( h0used );
            // New value to use 
            importance_float_t h0new = h0used - p + pp ;
            // Put it in place... note inversion of signs
            h0 -= h0new * zlog( h0new );
            //cout << "H0 " << H0 << " h0 " << h0 << endl;
            
            // Anulate old value used there... note inversion
            // of signs
            importance_float_t h1 = H1 + h1used * zlog( h1used );
            // New value to use 
            importance_float_t h1new = h1used - p + pp ;
            // Put it in place... note inversion of signs
            h1 -= h1new * zlog( h1new );
            //cout << "H1 " << H1 << " h1 " << h1 << endl;

            // Rows are indexed using i, so, norm_rows are the 
            // marginal probabilities of all the rows, and we can visualize
            // it as a column vector. Similarly, we can visualize norm_cols as
            // a row vector. 
            
            // Suppressing old effect
            importance_float_t old_summand = p*zlog( 
                p / (
                    norm_cols(j) * norm_rows(i)
                ));
            //cout << "old summand " << old_summand << std::endl;
            ni = unmi - old_summand;
            // Putting new effect
            importance_float_t new_summand =pp*zlog( pp / ( h0new * h1new ) );
            //cout << "new summand " << new_summand << std::endl;

            ni += new_summand;

            // Now ni contains the no-sum factor
            // Do the division
            importance_float_t nv = ni / std::max( h0 , h1 );

            importance_float_t var = nv - nmi ;

            s2 += var*var;

        }
        out_max_variance = std::sqrt( s2 );
        out_nmi = nmi;
    } // }}}

    size_t total_events_from_unmi_cm( 
        counter_matrix_t const& cm 
    )
    {
        size_t total_events = 0.0;
        BOOST_FOREACH( auto int2size , cm.data() )
        {
            total_events += int2size.second ;
        }

        return total_events;
    }

}} // namespace alve::gecmi

