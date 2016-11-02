/*
 * uncertaintity_volume.cpp
 *
 *  Created on: Sep 14, 2011
 *      Author: alcides
 */

#include "uncertaintity_volume.hpp"

#include <alve/gecmi/confusion.hpp>

#include <boost/math/distributions/binomial.hpp>

namespace alve { namespace gecmi {

// Computing, with a given probability threshold, the uncertaintity
// multidimensional volume at a given point in time and with a given
// number of steps. if there is a total of G = n*m variables in the matrix,
// the returned volume is G-dimensional in the volume formed by all the
// variables.
void get_uncertaintity_estimation(
	counter_matrix_t const& cmat, // <-- Integer events explicitly expected...!
	importance_matrix_t const& normprobs, // <-- Needed as an estimator
	size_t total_events,
	double prob_threshold,
	uncertaintity_estimation_t& out_unc_est
	)
{
    assert( prob_threshold < 0.5 );
    // Let's go for all the variables, each one at a time.
    size_t row_count = cmat.size1();
    size_t col_count = cmat.size2();
    double  volume = 1.0;
    double max_side = 0.0;
    size_t dim_count = row_count * col_count;
    for( size_t i=0; i < row_count; i ++)
    {
            for( size_t j=0; j < col_count; j++)
            {
                    double left_tail = prob_threshold;
                    double right_tail = 1.0 - prob_threshold;

                    double estimated_prob = normprobs( i, j);
                    boost::math::binomial_distribution<> dist( total_events, estimated_prob);
                    double left = boost::math::quantile( dist, left_tail);
                    double right = boost::math::quantile( dist, right_tail);
                    // What fraction is this of the unit?
                    double left_fraction = left / total_events;
                    double right_fraction = right / total_events;
                    double space = right_fraction - left_fraction;
                    assert( space >= 0.0 );
                    if ( space > max_side )
                    {
                            max_side = space;
                    }
                    volume *= space;
            }
    }

    out_unc_est.volume = volume;
    out_unc_est.max_side = max_side;
    out_unc_est.equivalent_side = std::pow( volume, 1.0/dim_count);
}

}} // namespace alve::gecmi
