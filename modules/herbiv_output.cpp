///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Output module for the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_output.h"
#include "guess.h"               // for Date and Gridcell
#include "herbiv_hft.h"          // for Hft
#include "herbiv_patchhabitat.h" // for Fauna::PatchHabitat
#include <cassert>               // for assert()
#ifndef NO_GUESS_PARAMETERS
#include "parameters.h"        // for declare_parameter()
#endif // NO_GUESS_PARAMETERS

using namespace GuessOutput;
using namespace Fauna;

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

	// output variables
	declare_parameter("file_forage_avail", &file_forage_avail, 300, 
			"File for herbivory output: "
			"Available forage [kgDM/m²]");
	declare_parameter("file_digestibility", &file_digestibility, 300,
			"File for herbivory output: "
			"Forage digestibility [frac]");
	declare_parameter("file_forage_eaten", &file_forage_eaten, 300, 
			"File for herbivory output: "
			"Eaten forage [kgDM/m²]");

	declare_parameter("file_hft_dens_ind", &file_hft_dens_ind, 300, 
			"File for herbivory output: "
			"Herbivore density [ind/km²]");
	declare_parameter("file_hft_dens_mass", &file_hft_dens_mass, 300, 
			"File for herbivory output: "
			"Herbivore density [kg/km²]");
}

HerbivoryOutput& HerbivoryOutput::get_instance(){
	if (global_instance == NULL)
		throw std::logic_error("GuessOutput::HerbivoryOutput::get_instance(): "
				"No instance for this class created yet.");
	return *global_instance;
}

void HerbivoryOutput::init() {
	if (!isactive) return;

	// TODO
	if (hftlist == NULL)
		throw std::logic_error("GuessOutput::HerbivoryOutput::init(): "
				"hftlist not declared. Call set_hftlist() before.");

	if (interval_xtring == "annual")
		interval = ANNUAL;
	else if (interval_xtring == "monthly")
		interval = MONTHLY;
	else if (interval_xtring == "daily")
		interval = DAILY;
	else if (interval_xtring == ""){
		dprintf("HerbivoryOutput: parameter herbiv_output_interval "
				"is missing in the instruction file.");
		fail();
	} else {
		dprintf("HerbivoryOutput: parameter herbiv_output_interval "
				"has invalid value: %s", (char*) interval_xtring);
		fail();
	}

	define_output_tables();
}

const ColumnDescriptors HerbivoryOutput::get_forage_columns()const {
	ColumnDescriptors forage_columns;
	forage_columns += ColumnDescriptor(get_forage_type_name(FT_GRASS).c_str(), 
			column_width, precision);
	// add other forage types here
	forage_columns += ColumnDescriptor("total", column_width, precision);
	return forage_columns;
}

const ColumnDescriptors HerbivoryOutput::get_hft_columns()const {
	assert(hftlist != NULL); // checked for in init() already
	ColumnDescriptors hft_columns;
	HftList::const_iterator itr = get_hftlist().begin();
	while (itr != get_hftlist().end()){
		hft_columns += ColumnDescriptor(itr->name.c_str(),
				column_width, precision);
		itr++;
	}
	hft_columns += ColumnDescriptor("total", column_width, precision);
	return hft_columns;
}

void HerbivoryOutput::define_output_tables() {
	assert(column_width >= 1);
	assert(precision    >= 0);

	// Create commonly used columns
	const ColumnDescriptors forage_columns = get_forage_columns();
	const ColumnDescriptors hft_columns = get_hft_columns();

	// Create the columns for each output table
	create_output_table(out_forage_avail , file_forage_avail.c_str() , forage_columns);
	assert( !out_forage_avail.invalid() );
	create_output_table(out_forage_eaten , file_forage_eaten.c_str() , forage_columns);
	create_output_table(out_digestibility, file_digestibility.c_str(), forage_columns);

	create_output_table(out_hft_dens_ind, file_hft_dens_ind.c_str(), hft_columns);
	create_output_table(out_hft_dens_mass, file_hft_dens_mass.c_str(), hft_columns);
}

void HerbivoryOutput::outannual(Gridcell& gridcell){
	if (!isactive) return;

	// References to all Habitat objects in the gridcell.
	std::vector<const Habitat*> habitats;

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
			const Patch& patch = stand.getobj();
			habitats.push_back(&patch.get_habitat());
			stand.nextobj(); // next patch
		} // patch loop
		++gc_itr; // next stand
	} // stand loop

	// Use the more general outannual function to do the rest
	outannual(gridcell.get_lon(), gridcell.get_lat(), 
			date.year, habitats);
}	

void HerbivoryOutput::outannual(
		const double longitude, const double latitude,
		const int year,
		const std::vector<const Fauna::Habitat*> habitats) const
{
	if (!isactive) return;
	assert(include_date.get() != NULL);
	
	// Vector of habitat output data objects.
	typedef std::vector<HabitatOutputData> OutputVector;

	// Abort already if there is no output for the whole year
	if (interval == ANNUAL && (*include_date)(0,year)) {

		// Vector holding the annual output for each habitat.
		OutputVector annual_habitat;
		// reserve space in the vector (without filling it yet).
		annual_habitat.reserve(habitats.size());

		// GATHER DATA
		for (int i=0; i<habitats.size(); i++) {
			const Habitat& habitat = *(habitats[i]);

			// Add this habitat’s annual output to the vector
			annual_habitat.push_back(habitat.get_annual_output());
		}

		// MERGE DATA

		// Annual output from all habitats in this gridcell merged in one object.
		const HabitatOutputData merged_annual = 
			HabitatOutputData::merge(annual_habitat);

		// WRITE DATA
		add_output_object(
				OutputRows(output_channel,
					longitude, 
					latitude,
					year,
					0), // day of year
				merged_annual );

	} else if (interval == MONTHLY) {

		// Array of 12 vectors, each of which holds one month’s output of all patches.
		OutputVector monthly_habitat[12];

		// reserve space for the patches
		for (int i=0; i<12; i++)
			monthly_habitat[i].reserve(habitats.size()); 

		// GATHER DATA

		for (int i=0; i<habitats.size(); i++) { 
			const Habitat& habitat = *(habitats[i]);

			// Add this habitat’s monthly output by appending the
			// data month by month to the big list.
			const OutputVector habitats_monthly = habitat.get_monthly_output();
			for (int i=0; i<12; i++){
				monthly_habitat[i].push_back(habitats_monthly[i]); 
			}
		}

		// MERGE DATA
		
		// Monthly output from all habitats in one object per month.
		HabitatOutputData merged_monthly[12]; 

		// merge vector of data for each month and add it
		for (int i=0; i<12; i++)
			merged_monthly[i] = HabitatOutputData::merge(monthly_habitat[i]);
		

		// WRITE DATA

		// Day of the year at the beginning of the month
		int first_day_of_month = 0;

		for (int i=0; i<12; i++) {
			// check if output is included
			if ((*include_date)(first_day_of_month, year)) {
				add_output_object(
						OutputRows(output_channel,
							longitude, 
							latitude,
							year,
							first_day_of_month),
						merged_monthly[i]);
			}
			// set it to the next day of the month
			first_day_of_month += date.ndaymonth[i];
		}

	} else if (interval == DAILY) {

		// loop through each day
		for (int d=0; d<Date::MAX_YEAR_LENGTH; d++) {

			// Skip this day if it’s not included
			if (!(*include_date)(d, year))
				continue;

			// Vector holding this day’s data for each habitat
			OutputVector days_habitat_data = OutputVector();
			days_habitat_data.reserve(habitats.size());
			
			// GATHER DATA for this day from all habitats 
			for (int i=0; i<habitats.size(); i++) { 
				const Habitat& habitat = *(habitats[i]);

				days_habitat_data.push_back(habitat.get_daily_output(d));
			}
			
			// MERGE AND WRITE DATA 
			add_output_object(
					OutputRows(output_channel,
						longitude,
						latitude,
						year,
						d),
					HabitatOutputData::merge(days_habitat_data));
		}
		

	}
}

void HerbivoryOutput::add_output_object(OutputRows out, const HabitatOutputData& data) const{

	// AWARENESS: Be sure to add the forage types in the same order as the columns
	// that are defined in get_forage_columns!

	// digestibility
	out.add_value(out_digestibility, data.available_forage.grass.get_digestibility());
	out.add_value(out_digestibility, data.available_forage.get_total().get_digestibility());

	// available DM mass
	out.add_value(out_forage_avail, data.available_forage.grass.get_mass());
	out.add_value(out_forage_avail, data.available_forage.get_total().get_mass());

	// eaten forage
	out.add_value(out_forage_eaten, data.eaten_forage.get_grass());
	out.add_value(out_forage_eaten, data.eaten_forage.sum());

	// HFTs
	double total_dens_ind, total_dens_mass = 0.0;
	// iterate throug hftlist, assuming the order has not changed.
	// TODO: Have custom vector of HFT pointers to be sure order
	// does not change.
	for (HftList::const_iterator itr = get_hftlist().begin();
			itr != get_hftlist().end(); itr++) {
		const Hft& hft = *itr;

		// TODO Replace this dirty fix with a good output collector
		// individual density
		{ 
			std::map<const Hft*, double>::const_iterator find 
				= data.density_ind.find(&hft);
			if ( find != data.density_ind.end() ){
				out.add_value(out_hft_dens_ind, find->second);
				total_dens_ind += find->second;
			} else
				out.add_value(out_hft_dens_ind, 0.0);
		}

		// mass density
		{ 
			std::map<const Hft*, double>::const_iterator find 
				= data.density_mass.find(&hft);
			if ( find!= data.density_mass.end() ){
				out.add_value(out_hft_dens_mass, find->second);
				total_dens_mass += find->second;
			} else
				out.add_value(out_hft_dens_mass, 0.0);
		}
	}
	// print totals
	out.add_value(out_hft_dens_ind, total_dens_ind);
	out.add_value(out_hft_dens_mass, total_dens_mass);
}
