///////////////////////////////////////////////////////////////////////////////////////
/// \file herbiv_output.h
/// \brief Output module for the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_output.h"
#include "guess.h"             // for Date class
#ifndef NO_GUESS_PARAMETERS
#include "parameters.h"        // for declare_parameter()
#endif // NO_GUESS_PARAMETERS

using namespace GuessOutput;
using namespace Fauna;

REGISTER_OUTPUT_MODULE("herbivory", HerbivoryOutput)

// initialize static variable
bool HerbivoryOutput::isactive = true;


HerbivoryOutput::HerbivoryOutput():
	interval(ANNUAL)
{
	declare_parameter("herbiv_output_interval", 
			&interval_xtring, 128,
			"Interval for herbivory output: \"annual\", \"monthly\"\n");

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
}


void HerbivoryOutput::init() {
	if (!isactive) return;

	if (interval_xtring == "annual")
		interval = ANNUAL;
	else if (interval_xtring == "monthly")
		interval = MONTHLY;
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

const ColumnDescriptors HerbivoryOutput::get_forage_columns(const int width, const int precision)const {
	ColumnDescriptors forage_columns;
	forage_columns += ColumnDescriptor(get_forage_type_name(FT_GRASS).c_str(), 
			width, precision);
	// add other forage types here
	forage_columns += ColumnDescriptor("total", width, precision);
	return forage_columns;
}

const ColumnDescriptors HerbivoryOutput::get_hft_columns(const int width, const int precision)const {
	ColumnDescriptors hft_columns;
	// TODO
	return hft_columns;
}

void HerbivoryOutput::define_output_tables() {
	/// Width for one column
	const int default_width = 8;
	/// Decimal precision for the columns
	const int default_precision = 3;

	// Create commonly used columns
	const ColumnDescriptors forage_columns = get_forage_columns(
			default_width, default_precision);
	const ColumnDescriptors hft_columns = get_hft_columns(
			default_width, default_precision);

	// Create the columns for each output file
	create_output_table(out_forage_avail , file_forage_avail.c_str() , forage_columns);
	assert( !out_forage_avail.invalid() );
	create_output_table(out_forage_eaten , file_forage_eaten.c_str() , forage_columns);
	create_output_table(out_digestibility, file_digestibility.c_str(), forage_columns);
}

bool HerbivoryOutput::include_date(const Date& d) const{
	return d.year >= nyear_spinup;
}

void HerbivoryOutput::outannual(Gridcell& gridcell){
	if (!isactive) return;

	// Abort if there is no output this year.
	if (!include_date(date))
		return;

	/// References to all Habitat objects in the gridcell.
	std::vector<const Habitat*> habitats;

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
			date.day, date.year, habitats);
}	


void HerbivoryOutput::outannual(
		const double longitude, const double latitude,
		const int day_of_year, const int year,
		const std::vector<const Fauna::Habitat*> habitats) const
{
	if (!isactive) return;

	assert(interval == MONTHLY || interval == ANNUAL);

	/////////////////////////////////////////////////////////////
	// PREPARE VARIABLES
	/////////////////////////////////////////////////////////////
	
	/// Vector of habitat output data objects.
	typedef std::vector<HabitatOutputData> OutputVector;
	
	/// Vector holding the annual output for each habitat.
	OutputVector annual_habitat;
	// reserve space in the vector (without filling it yet).
	annual_habitat.reserve(habitats.size());

	/// Vector of 12 vectors, each of which holds one month’s output of all patches.
	std::vector<OutputVector> monthly_habitat;
	monthly_habitat.resize(12); // create 12 new vector objects
	// reserve space for the patches
	for (int i=0; i<12; i++)
		monthly_habitat[i].reserve(habitats.size()); 

	/////////////////////////////////////////////////////////////
	// GATHER OUTPUT DATA
	/////////////////////////////////////////////////////////////

	for (int i=0; i<habitats.size(); i++)
	{
		const Habitat& habitat = *(habitats[i]);
		
		// Add this habitat’s annual output to the vector
		annual_habitat.push_back(habitat.get_annual_output());
		assert( annual_habitat.back().is_valid );

		// Add this habitat’s monthly output by appending the
		// data month by month to the big list.
		const OutputVector habitats_monthly = habitat.get_monthly_output();
		for (int i=0; i<12; i++){
			assert( habitats_monthly[i].is_valid );
			monthly_habitat[i].push_back(habitats_monthly[i]); 
		}
	}

	/////////////////////////////////////////////////////////////
	// MERGE VECTORS TO SINGLE DATA POINTS
	/////////////////////////////////////////////////////////////

	/// Annual output from all habitats in this gridcell merged in one object.
	const HabitatOutputData merged_annual = 
		HabitatOutputData::merge(annual_habitat);

	/// Monthly output from all habitats in one object per month.
	std::vector<HabitatOutputData> merged_monthly;
	merged_monthly.reserve(12); // make space for 12 objects
	// merge vector of data for each month and add it
	for (int i=0; i<12; i++){
		merged_monthly.push_back(HabitatOutputData::merge(monthly_habitat[i]));
	}
	
	/////////////////////////////////////////////////////////////
	// ADD DATA TO TABLES
	/////////////////////////////////////////////////////////////

	switch (interval) {
		case ANNUAL:
			add_output_object(
					OutputRows(output_channel,
						longitude, 
						latitude,
						year,
						0), // day of year
					merged_annual );
			break;
		case MONTHLY:
			assert(merged_monthly.size() == 12);
			/// Day of the year at the beginning of the month
			int first_day_of_month = 0;
			for (int i=0; i<12; i++) {
				add_output_object(
						OutputRows(output_channel,
							longitude, 
							latitude,
							year,
							first_day_of_month),
						merged_monthly[i]);
				// set it to the next day of the month
				first_day_of_month += date.ndaymonth[i];
			}
			break;
			// TODO: DAILY
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
	out.add_value(out_forage_eaten, data.eaten_forage.grass);
	out.add_value(out_forage_eaten, data.eaten_forage.sum());
}
