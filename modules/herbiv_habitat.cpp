///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Classes for the spatial units where herbivores live.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#include "config.h"
#include "herbiv_habitat.h"
#include "herbiv_forageclasses.h" // for HabitatForage and ForageMass

using namespace Fauna;

namespace {
	/// Length of a year.
	const int YEAR_LENGTH = 365;
	/// Number of days in each month.
	const int MONTH_DAYS[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
}

HabitatOutputData HabitatOutputData::merge( 
		const std::vector<HabitatOutputData> data,
		const int first,
		int last
		){
	if (last == -1)
		last = data.size()-1;

	if (data.empty())
		throw std::invalid_argument("Fauna::HabitatOutputData::merge() "
				"Data vector is empty.");
	if (last < first)
		throw std::invalid_argument("Fauna::HabitatOutputData::merge() "
				"invalid index range: last < first");
	if (last >= data.size())
		throw std::out_of_range("Fauna::HabitatOutputData::merge() "
			"last >= data.size()");
	if (first >= data.size())
		throw std::out_of_range("Fauna::HabitatOutputData::merge() "
			"first >= data.size()");

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

		// HFT VALUES TODO: rework this with output collector
		// THIS IS TO BE REMOVED AND JUST FOR TESTING
		for (std::map<const Hft*, double>::const_iterator itr = item.density_ind.begin();
				itr != item.density_ind.end(); itr++){
			const Hft* hft = itr->first;
			double dens_ind = item.density_ind.find(hft)->second;
			double dens_mass = item.density_mass.find(hft)->second;
			// build average directly:
			result.density_ind[hft] += 
				dens_ind / (double)data.size();
			result.density_mass[hft] += 
				dens_mass / (double)data.size();
		}

	}

	// the HabitatForage class has its own merge function
	result.available_forage = HabitatForage::merge(available_forage_vector);

	result.is_valid = true;
	return result;
}

Habitat::Habitat(std::auto_ptr<HftPopulationsMap> populations):
	populations(populations),
	daily_output(365)
{
	if (this->populations.get() == NULL)
		throw std::invalid_argument("Fauna::Habitat::Habitat() "
				"Argument \"populations\" is NULL.");
}
std::vector<HabitatOutputData> Habitat::get_monthly_output() const {
	assert(daily_output.size() == YEAR_LENGTH);

	std::vector<HabitatOutputData> monthly_output;
	monthly_output.reserve(12); // reserve disk space for 12 entries without creating
	int month        = 0;       // January = 0
	int day_of_month = 0;       // First   = 0

	// iterate through the whole year 
	for (int i=0; i<YEAR_LENGTH; i++){
		assert(month < 12);
		assert(day_of_month < MONTH_DAYS[month]);

		if (day_of_month == MONTH_DAYS[month] - 1){
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
	HabitatOutputData result = HabitatOutputData::merge(daily_output);
	return result;
}

void Habitat::remove_eaten_forage(const ForageMass& eaten_forage){
	get_todays_output().eaten_forage += eaten_forage;
	// actual forage removal in derived classes
}

void Habitat::init_todays_output(const int today){
	if (today>=365 || today<0)
		throw std::invalid_argument("Fauna::Habitat::init_todays_output() "
				"Parameter \"today\" out of range");

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
	{ // TODO: Remove this and use herbivore output instead
		todays_output.density_ind.clear();
		todays_output.density_mass.clear();

		for (HftPopulationsMap::iterator itr_pop=populations->begin();
				itr_pop != populations->end(); itr_pop++){
			const PopulationInterface& population = **itr_pop;
			const Hft& hft = population.get_hft();

			todays_output.density_mass[&hft] = 0.0;
			todays_output.density_ind[&hft]  = 0.0;

			// sum up population densities for HFT
			const ConstHerbivoreVector herbivores = population.get_list();
			for (ConstHerbivoreVector::const_iterator itr_h = herbivores.begin();
					itr_h != herbivores.end(); itr_h++) {
				const HerbivoreInterface& herbivore = **itr_h;

				todays_output.density_mass[&hft] += 
					herbivore.get_kg_per_km2();
				todays_output.density_ind[&hft] += 
					herbivore.get_ind_per_km2();
			}
		}
	}
	// TODO: Add more output here
	
	todays_output.is_valid = true;
}

const HabitatOutputData& Habitat::read_todays_output() {
	// Reference to this day’s output
	HabitatOutputData& todays_output = get_todays_output();

	// Insert more values that are calculated at the end of the
	// day here

	assert(todays_output.is_valid);
	return todays_output;
}

