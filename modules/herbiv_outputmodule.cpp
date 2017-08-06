///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Output module for the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_outputmodule.h"
#include "guess.h"               // for Date and Gridcell
#include "herbiv_habitat.h"      // for HabitatList
#include "herbiv_patchhabitat.h" // for Patch::get_habitat()
#include <cassert>               // for assert()
#ifndef NO_GUESS_PARAMETERS
#include "parameters.h"     // for declare_parameter()
#endif // NO_GUESS_PARAMETERS

using namespace GuessOutput;
using namespace Fauna;

bool GuessOutput::is_first_day_of_month(int day){
	if (day < 0 || day > 364)
		throw std::invalid_argument("GuessOutput::is_first_day_of_month() "
				"Parameter \"day\" is out of range.");
	int m = 0;
	int month_length = date.ndaymonth[m];
	// Subtract successively the months (beginning with January).
	while (day > 0){
		assert(m<=11);
		month_length = date.ndaymonth[m];
		day -= month_length;
		m++;
	}
	assert( day <= 0);
	return day == 0;
}

bool IncludeNoSpinup::operator()(const int year, 
		const int day_of_year) const{
	return year >= nyear_spinup;
}

REGISTER_OUTPUT_MODULE("herbivory", HerbivoryOutput)

// define and initialize static variable
HerbivoryOutput* HerbivoryOutput::global_instance = NULL;

HerbivoryOutput::HerbivoryOutput():
	interval(ANNUAL),
	isactive(true),
	precision(4),
	include_date(new IncludeDate())
{
	// Check if someone is trying to create another instance
	if (global_instance == NULL)
		global_instance = this; 
	else
		throw std::logic_error("GuessOutput::HerbivoryOutput::HerbivoryOutput(): "
				"Constructor called a second time. There should be only "
				"one global instance of an output module.");


	declare_parameter("herbiv_output_interval", 
			&interval_xtring, 128,
			"Interval for herbivory output: \"annual\", \"monthly\"\n");

	declare_parameter("herbiv_output_precision",
			&precision,
			0, 20, // min,max
			"Decimal precision of values in the herbivory output tables. "
			"Defaults to 4 if not defined.");
}

HerbivoryOutput& HerbivoryOutput::get_instance(){
	if (global_instance == NULL)
		throw std::logic_error("GuessOutput::HerbivoryOutput::get_instance(): "
				"No instance for this class created yet.");
	return *global_instance;
}

void HerbivoryOutput::init() {
	if (!isactive) return;

	if (interval_xtring == "annual")
		interval = ANNUAL;
	else if (interval_xtring == "monthly")
		interval = MONTHLY;
	else if (interval_xtring == "daily")
		interval = DAILY;
	else if (interval_xtring == "decadal")
		interval = DECADAL;
	else if (interval_xtring == ""){
		dprintf("HerbivoryOutput: parameter herbiv_output_interval "
				"is missing in the instruction file.");
		fail();
	} else {
		dprintf("HerbivoryOutput: parameter herbiv_output_interval "
				"has invalid value: %s", (char*) interval_xtring);
		fail();
	}
}

void HerbivoryOutput::outdaily(Gridcell& gridcell){
	if (!isactive) return;

	// References to all Habitat objects in the gridcell.
	std::vector<Habitat*> habitats;

	// reserve space in array: number of stands (gridcell.size())
	// times number of patches in first stand object.
	if (gridcell.size() > 0)
		habitats.reserve(gridcell.size() * (*gridcell.begin()).nobj);

	//Loop through Patches to gather all habitats
	Gridcell::iterator gc_itr = gridcell.begin();
	while (gc_itr != gridcell.end()) {
		Stand& stand = *gc_itr;
		stand.firstobj(); 
		while (stand.isobj) {
			Patch& patch = stand.getobj();
			// add pointer to the habitat to the list
			Habitat& habitat = patch.get_habitat();
			habitats.push_back(&habitat);
			stand.nextobj(); // next patch
		} // patch loop
		++gc_itr; // next stand
	} // stand loop

	// Use the more general function to do the rest
	outdaily(gridcell.get_lon(), gridcell.get_lat(), 
			date.day, date.year, habitats);
}	

void HerbivoryOutput::outdaily(
		const double longitude, const double latitude,
		const int day, const int year,
		const std::vector<Fauna::Habitat*>& habitats)
{
	if (day<0 || day>=365)
		throw std::invalid_argument("GuessOutput::HerbivoryOutput::outdaily() "
				"Parameter \"day\" is out of range.");
	if (year < 0)
		throw std::invalid_argument("GuessOutput::HerbivoryOutput::outdaily() "
				"Parameter \"year\" is below zero.");

	if (!isactive) return;
	assert(include_date.get() != NULL);

	// Check if this day is included
	if (!(*include_date)(day, year))
		return;

	// Days since last output
	static int day_counter = 0;

	if ((interval == DAILY) ||
			(interval == MONTHLY && is_first_day_of_month(day)) ||
			(interval == ANNUAL && day==0) ||
			(interval == DECADAL && year%10==0)){
		// TODO
	}
	day_counter++;
}

void HerbivoryOutput::set_hftlist(const HftList* hftlist){
	// TODO
}
