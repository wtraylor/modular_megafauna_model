///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Output module for the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_outputmodule.h"
#include "guess.h"                // for Date and Gridcell
#include "herbiv_habitat.h"       // for HabitatList
#include "herbiv_hft.h"           // for HftList
#include "herbiv_outputclasses.h" // for HabitatData and HerbivoreData
#include "herbiv_patchhabitat.h"  // for Patch::get_habitat()
#include "herbiv_population.h"    // for HerbivoreVector
#include <cassert>                // for assert()
#include <cmath>                  // for NAN
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
const double HerbivoryOutput::NA_VALUE = NAN;

HerbivoryOutput::HerbivoryOutput():
	interval(ANNUAL),
	isactive(true),
	precision(4),
	include_date(new IncludeDate()),
	// Output variables in the order of declaration in the header:
	TBL_AVAILABLE_FORAGE(
			"file_herbiv_available_forage",
			"Available forage in the habitats.",
			"kgDM/km²",
			CS_FORAGE),
	TBL_DIGESTIBILITY(
			"file_herbiv_digestibility",
			"Forage digestibility.",
			"frac",
			CS_FORAGE),
	TBL_EATEN_FORAGE(
			"file_herbiv_eaten_forage",
			"Forage eaten by herbivores per day.",
			"kgDM/km²/day",
			CS_FORAGE),
	TBL_BODYFAT(
			"file_herbiv_bodyfat",
			"Herbivore body fat.",
			"kg/kg",
			CS_HFT),
	TBL_EXPENDITURE(
			"file_herbiv_expenditure",
			"Herbivore daily energy expenditure.",
			"MJ/ind/day",
			CS_HFT),
	TBL_INDDENS(
			"file_herbiv_inddens",
			"Individual herbivore density.",
			"ind/km²",
			CS_HFT),
	TBL_MASSDENS(
			"file_herbiv_massdens",
			"Herbivore mass density.",
			"kg/km²",
			CS_HFT),
	TBL_STARVATION(
			"file_herbiv_starvation",
			"Daily herbivore mortality through starvation.",
			"ind/ind/day",
			CS_HFT),
	TBL_EATEN_IND(
			"file_herbiv_eaten_ind",
			"Forage eaten by herbivores per day.",
			"kgDM/ind/day",
			CS_HFT_FORAGE),
	TABLEFILES(init_tablefiles())
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

	// Declare parameters for file names
	assert( !TABLEFILES.empty() );
	for (std::vector<TableFile*>::const_iterator itr = TABLEFILES.begin();
			itr != TABLEFILES.end(); itr++) {
		TableFile& tf = **itr;
		// Check if parameter name is valid
		const std::string param_check_str(tf.paramname);
		assert( param_check_str.find(' ') == std::string::npos );
		assert( param_check_str.find('.') == std::string::npos );

		const std::string help = tf.description + " ["+tf.unit+"]";
		declare_parameter(
				tf.paramname, // This must be const char
				&tf.filename,
				300, // max string length
				help.c_str());
	}
}

HerbivoryOutput& HerbivoryOutput::get_instance(){
	if (global_instance == NULL)
		throw std::logic_error("GuessOutput::HerbivoryOutput::get_instance(): "
				"No instance for this class created yet.");
	return *global_instance;
}

const std::vector<HerbivoryOutput::TableFile*> HerbivoryOutput::init_tablefiles(){
	std::vector<HerbivoryOutput::TableFile*> list;
	// initialize the vector on first call
	if (list.empty()){
		list.push_back(&TBL_AVAILABLE_FORAGE);
		list.push_back(&TBL_DIGESTIBILITY);
		list.push_back(&TBL_EATEN_FORAGE);
		list.push_back(&TBL_BODYFAT);
		list.push_back(&TBL_EXPENDITURE);
		list.push_back(&TBL_INDDENS);
		list.push_back(&TBL_MASSDENS);
		list.push_back(&TBL_STARVATION);
		list.push_back(&TBL_EATEN_IND);
	}
	return list;
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

	if (hftlist.get() == NULL)
		throw std::logic_error("GuessOutput::HerbivoryOutput::init() "
				"HFT list has not been set. Call HerbivoryOutput::set_hftlist() "
				"before initializing the output module.");

	// CREATE TABLES
	for (std::vector<TableFile*>::const_iterator itr = TABLEFILES.begin();
			itr != TABLEFILES.end(); itr++)
	{
		TableFile& tf = **itr;
		create_output_table(
				tf.table, 
				tf.filename.c_str(),
				get_columns(tf.column_selector));
	}
}

ColumnDescriptors HerbivoryOutput::get_columns(
		const HerbivoryOutput::ColumnSelector selector){
	ColumnDescriptors result;
	switch (selector){
		case CS_FORAGE:
			// names of forage types
			for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
					ft != FORAGE_TYPES.end(); ft++)
			{
				result += ColumnDescriptor(
						get_forage_type_name(*ft).c_str(),
						COLUMN_WIDTH, precision);
			}
			break;
		case CS_HFT:
			// names of HFTs
			assert(hftlist.get() != NULL);
			for (HftList::const_iterator hft = hftlist->begin();
					hft != hftlist->end(); hft++) {
				result += ColumnDescriptor(
						hft->name.c_str(),
						COLUMN_WIDTH, precision);
			}
			break;
		case CS_HFT_FORAGE:
			// names of HFTs + names of forage types
			assert(hftlist.get() != NULL);
			// It is important to have the inner and outer loop here the
			// same as in write_datapoint()! Otherwise the values are
			// not congruent with the table header.
			for (HftList::const_iterator hft = hftlist->begin();
					hft != hftlist->end(); hft++) 
				for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
						ft != FORAGE_TYPES.end(); ft++)
				{
					const std::string combined = 
						hft->name + CAPTION_SEPARATOR + get_forage_type_name(*ft);
					result += ColumnDescriptor(
							combined.c_str(),
							COLUMN_WIDTH, precision);
				}
			break;
		case CS_HABITAT:
			// In this special case, there is no fixed/independent
			// variable that would define the captions.
			result += ColumnDescriptor(
					"value",
					COLUMN_WIDTH, precision);
			break;
	}
	return result;
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

	if ((interval == DAILY) ||
			(interval == MONTHLY && is_first_day_of_month(day)) ||
			(interval == ANNUAL && day==0) ||
			(interval == DECADAL && year%10==0 && day==0))
	{
		FaunaOut::Aggregator aggregator;

		// Loop through all habitats and aggregate their output
		// to one data point.
		for (std::vector<Fauna::Habitat*>::const_iterator itr=habitats.begin();
				itr != habitats.end(); itr++)
		{
			Habitat& habitat = **itr;
			// add habitat data
			aggregator.add(habitat.retrieve_output());

			// add herbivore data
			// all populations in the habitat (one for each HFT)
			HftPopulationsMap& populations = habitat.get_populations();

			// All herbivores in the habitat
			HerbivoreVector herbivores = populations.get_all_herbivores();

			// loop through all herbivores
			for (HerbivoreVector::iterator itr_h=herbivores.begin();
					itr_h != herbivores.end(); itr_h++) {
				HerbivoreInterface& herbivore = **itr_h;
				// aggregate this herbivore’s output
				aggregator.add(herbivore.get_hft(), 
						herbivore.retrieve_output());
			}
		}
		// Write output
		write_datapoint( longitude, latitude, day, year, aggregator.reset() );
	}
}

void HerbivoryOutput::set_hftlist(const HftList& _hftlist){
	if (hftlist.get() != NULL)
		throw std::logic_error("GuessOutput::HerbivoryOutput::set_hftlist() "
				"HFT list has already been set. "
				"It cannot be set a second time.");
	// copy-construct a new HftList object.
	hftlist = std::auto_ptr<HftList>(new HftList(_hftlist));
}

void HerbivoryOutput::write_datapoint( 
		const double longitude, const double latitude,
		const int day, const int year,
		const FaunaOut::CombinedData& datapoint){
	// The OutputRows object manages the next row of output for each
	// output table
	OutputRows output_rows(
			output_channel, longitude, latitude, 
			year, day);

	// HABITAT TABLES
	// ... more to come

	// FORAGE TABLES
	const ForageMass available_mass   = 
		datapoint.habitat_data.available_forage.get_mass();
	const Digestibility digestibility = 
		datapoint.habitat_data.available_forage.get_digestibility();
	for (std::set<ForageType>::const_iterator ft=FORAGE_TYPES.begin();
			ft!=FORAGE_TYPES.end(); ft++)
	{
		output_rows.add_value(TBL_AVAILABLE_FORAGE.table, 
				available_mass[*ft]);
		output_rows.add_value(TBL_DIGESTIBILITY.table,
				digestibility[*ft]);
		output_rows.add_value(TBL_EATEN_FORAGE.table,
				datapoint.habitat_data.eaten_forage[*ft]);
	}

	// ALL HFT-SPECIFIC TABLES
	assert(hftlist.get() != NULL);
	for (HftList::const_iterator itr_hft = hftlist->begin();
			itr_hft != hftlist->end(); itr_hft++)
	{
		const Hft& hft = *itr_hft;

		// See if we find the HFT in the data point
		//
		// Here it is important to remember that HerbivoryOutput
		// has its own *copy* of an HftList. Therefore, we need to
		// compare Hft objects not by pointer but by their
		// operator==() function.

		const FaunaOut::HerbivoreData* pherbidata = NULL;

		for (std::map<const Hft*, FaunaOut::HerbivoreData>::const_iterator 
				itr2 = datapoint.hft_data.begin();
				itr2 != datapoint.hft_data.end();
				itr2++)
		{
			const Hft& hft2 = *itr2->first;
			if (hft2 == hft){ // call Hft::operator==()
				pherbidata = &itr2->second;
				break;
			}
		}

		if (pherbidata != NULL) {
			// Okay, data for HFT found, we add it to the output
			const FaunaOut::HerbivoreData& herbidata = *pherbidata;

			// HFT TABLES
			output_rows.add_value(TBL_BODYFAT.table, 
					herbidata.bodyfat);
			output_rows.add_value(TBL_EXPENDITURE.table, 
					herbidata.expenditure);
			output_rows.add_value(TBL_INDDENS.table, 
					herbidata.inddens);
			output_rows.add_value(TBL_MASSDENS.table, 
					herbidata.massdens);
			// ** add new HFT variables here **

			{ // STARVATION MORTALITY
				double starvation = 0.0;
				// try to find the starvation mortality factors
				std::map<MortalityFactor, double>::const_iterator 
					itr_starv1 = herbidata.mortality.find(MF_STARVATION_ILLIUS2000);
				std::map<MortalityFactor, double>::const_iterator 
					itr_starv2 = herbidata.mortality.find(MF_STARVATION_THRESHOLD);

				// Sum up the starvation mortality.
				if (itr_starv1 != herbidata.mortality.end())
					starvation += itr_starv1->second;
				if (itr_starv2 != herbidata.mortality.end())
					starvation += itr_starv2->second;

				// If *all* starvation mortality factors were not found,
				// insert a “missing value”.
				if (itr_starv1 == herbidata.mortality.end() &&
						itr_starv2 == herbidata.mortality.end())
					starvation = NA_VALUE;

				output_rows.add_value(TBL_STARVATION.table, starvation);
			}

			// HFT-FORAGE TABLES
			// It is important to have the inner and outer loop here the
			// same as in get_columns()! Otherwise the values are
			// not congruent with the table header.
			for (std::set<ForageType>::const_iterator ft=FORAGE_TYPES.begin();
					ft!=FORAGE_TYPES.end(); ft++)
			{
				output_rows.add_value(TBL_EATEN_IND.table,
						herbidata.eaten_forage[*ft]);
				// ** add new HFT-Forage variables here
			}
		} else {
			// Oops, no data for this HFT. We need to insert NA values.
			// BUT: Only for those variables where it makes sense:
			//      Population density is *zero*, not a missing value.
			for (std::vector<TableFile*>::const_iterator itr_tab = TABLEFILES.begin();
					itr_tab != TABLEFILES.end(); itr_tab++) {
				TableFile& tablefile = **itr_tab;

				// Insert “missing value” if this table is HFT specific.
				if (tablefile.column_selector == CS_HFT ||
						tablefile.column_selector == CS_HFT_FORAGE) 
				{
					if (&tablefile == &TBL_INDDENS || &tablefile == &TBL_MASSDENS)
						output_rows.add_value(tablefile.table, 0.0);
					else 
						output_rows.add_value(tablefile.table, NA_VALUE);
				}
			}
		}
	}
}