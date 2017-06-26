///////////////////////////////////////////////////////////////////////////////////////
/// \file herbiv_framework.cpp
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_framework.h"
#include "guess.h"          // for Date
#include "herbiv_habitat.h" // for Habitat
#include "assert.h"

using namespace Fauna;


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
