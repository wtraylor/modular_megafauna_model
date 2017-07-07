///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Classes for the spatial units where herbivores live.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#include "config.h"
#include "herbiv_habitat.h"
#include "herbiv_foraging.h" // for HabitatForage and ForageMass

#include "guess.h" // for Date and date
// We need to use the global variable date, just 
// because ndaymonth is not implemented as static.
// We also need Date::MAX_YEAR_LENGTH.
// The code is still independent of the global date and
// anything else in guess.h


using namespace Fauna;

HabitatOutputData HabitatOutputData::merge( 
		const std::vector<HabitatOutputData> data,
		const int first,
		int last
		){

	assert (!data.empty());
	if (last < 0)
		last = data.size() -1;
	assert (first <= last);
	assert (first < data.size());
	assert (last < data.size());

	// Result object with the merged data
	HabitatOutputData result;

	// Vector of habitat forage.
	std::vector<const HabitatForage*> available_forage_vector;

	for (int i=first; i<=last; i++) {
		const HabitatOutputData& item = data[i];

		// Check if item is valid
		if (!item.is_valid) {
			result.is_valid = false;
			return result;
		}

		// ACCUMULATED VALUES
		result.eaten_forage += item.eaten_forage;

		// HabitatForage has its own merge function that needs a vector
		available_forage_vector.push_back(&item.available_forage); 
	}

	// the HabitatForage class has its own merge function
	result.available_forage = HabitatForage::merge(available_forage_vector);

	result.is_valid = true;
	return result;
}

Habitat::Habitat():
	daily_output(Date::MAX_YEAR_LENGTH)
{}


void Habitat::init_todays_output(const int today){
	// Set the state variable
	day_of_year = today;

	// On January 1st: reset all output from previous year
	if (today == 0) {
		for (int i=0; i<daily_output.size(); i++)
			daily_output[i].is_valid = false;
	}

	// The data object for todays output.
	HabitatOutputData& todays_output = get_todays_output();

	// reset the output by overwriting it with a newly constructed 
	// object
	todays_output = HabitatOutputData();

	todays_output.available_forage = get_available_forage();
	// TODO: Add more output here
	
	todays_output.is_valid = true;
}

const HabitatOutputData& Habitat::read_todays_output() {
	// Reference to this day’s output
	HabitatOutputData& todays_output = get_todays_output();

	assert(todays_output.is_valid);

	// Insert more values that are calculated at the end of the
	// day here

	return todays_output;
}

std::vector<HabitatOutputData> Habitat::get_monthly_output() const {
	assert(daily_output.size() == Date::MAX_YEAR_LENGTH);

	std::vector<HabitatOutputData> monthly_output;
	monthly_output.reserve(12); // reserve disk space for 12 entries without creating
	int month        = 0;       // January = 0
	int day_of_month = 0;       // First   = 0

	// iterate through the whole year 
	for (int i=0; i<Date::MAX_YEAR_LENGTH; i++){
		assert(month < 12);
		assert(day_of_month < date.ndaymonth[month]);

		if (day_of_month == date.ndaymonth[month] - 1){
			// We have reached the end of the month 

			monthly_output.push_back(HabitatOutputData::merge(
						daily_output,
						i - day_of_month, // index of first day of the month
						i));              // index of last day of the month

			day_of_month = 0;
			month++;
		} else 
			day_of_month++;
	}
	return monthly_output;
}

HabitatOutputData Habitat::get_annual_output() const {
	return HabitatOutputData::merge(daily_output);
}


void Habitat::remove_eaten_forage(const ForageMass& eaten_forage){
	assert(eaten_forage.grass >= 0.0);
    assert( eaten_forage.grass <= get_available_forage().grass.get_mass() );
	// add assertions for other forage types here

	get_todays_output().eaten_forage += eaten_forage;
}
