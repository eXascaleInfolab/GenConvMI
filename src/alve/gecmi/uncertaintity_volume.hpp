/*
 * uncertaintity_volume.hpp
 *
 *  Created on: Sep 15, 2011
 *      Author: alcides
 */

#ifndef ALVE__GECMI__UNCERTAINTITY_VOLUME_HPP_
#define ALVE__GECMI__UNCERTAINTITY_VOLUME_HPP_

#include <alve/gecmi/confusion.hpp>

#include <boost/math/distributions/binomial.hpp>

namespace alve { namespace gecmi {

struct uncertaintity_estimation_t {
	double volume;
	double equivalent_side;
	double max_side;
};

// Functions for estimating the uncertaintity volume
void get_uncertaintity_estimation(
	counter_matrix_t const& cmat, // <-- Integer events explicitly expected...!
	importance_matrix_t const& normprobs, // <-- Needed as an estimator
	size_t total_events,
	double prob_threshold,
	uncertaintity_estimation_t& out_unc_est
	);

}} // namespace alve::gecmi


#endif /* UNCERTAINTITY_VOLUME_HPP_ */
