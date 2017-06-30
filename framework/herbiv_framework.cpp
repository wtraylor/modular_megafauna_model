///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_framework.h"
#include "guess.h"          // for Date
#include "herbiv_habitat.h" // for Habitat
#include "herbiv_digestibility.h"
#include "herbiv_parameters.h"
#include "assert.h"
#include <stdexcept> // for std::logic_error, std::invalid_argument

using namespace Fauna;

Simulator::Simulator(const Parameters& params):
	params(params){

		// DIGESTIBILITY MODEL
		// The global object instance is held in a static variable.

		switch (params.dig_model) {
			case DM_PFT_FIXED:
				static PftDigestibility pft_dig; // global instance
				DigestibilityModel::init_global(pft_dig);
				break;
			// add other models here ...
			default:
				// TODO: Use exceptions or assertions?
				throw std::invalid_argument(
						"DigestibilityModel::init_model(): "
						"unknown enum type."); 
				break;
		}

}

void Simulator::simulate_day(const int day_of_year, Habitat& habitat,
		const bool do_herbivores){
	assert(day_of_year >= 0);
	assert(day_of_year < 365);
	// pass the current date into the herbivore module
	habitat.init_todays_output(day_of_year);

	if (do_herbivores) {
		// TODO
	}
}
