///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Classes for the spatial units where herbivores live.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#include "config.h"
#include "herbiv_habitat.h"

using namespace Fauna;

Habitat::Habitat(std::auto_ptr<HftPopulationsMap> populations):
	populations(populations)
{
	if (this->populations.get() == NULL)
		throw std::invalid_argument("Fauna::Habitat::Habitat() "
				"Argument \"populations\" is NULL.");
}

void Habitat::aggregate_todays_output(){
	if (todays_output().datapoint_count > 0) {
		aggregated_output.merge(todays_output());
		todays_output().reset();
	}
}

void Habitat::init_day(const int today){
	if (today<0 || today >= 365)
		throw std::invalid_argument( "Fauna::Habitat::init_day() "
				"Parameter \"today\" is out of range.");
	day_of_year = today;

	/// - Aggregate old output.
	aggregate_todays_output();

	/// - Initialize new output.
	todays_output().datapoint_count = 1; // one single day’s data
	todays_output().available_forage = get_available_forage();
}

FaunaOut::HabitatData Habitat::retrieve_output(){
	// Add current output to aggregation
	aggregate_todays_output();

	// Call copy constructor
	FaunaOut::HabitatData result(aggregated_output);
	// reset aggregated output
	aggregated_output.reset();
	return result;
}
