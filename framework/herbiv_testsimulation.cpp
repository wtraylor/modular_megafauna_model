///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivory test simulation independent of the LPJ-GUESS %framework.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "shell.h" // for dprintf()
#include "plib.h" // for plib() and itemparsed()
#include "parameters.h" // for declare_parameter()
#include "herbiv_testsimulation.h"
#include "herbiv_framework.h"
#include "herbiv_paramreader.h"
#include <climits> // for INT_MAX
#include <cfloat> // for DBL_MAX

using namespace Fauna;
using namespace FaunaSim;
using namespace GuessOutput;

// Anonymous namespace with definitions local to this file 
namespace {
	/// Creates a vector of pointers from a vector of 
	/// \ref SimulationUnit objects.
	std::vector<SimulationUnit*> simunit_vector_to_pointers(
			std::vector<SimulationUnit> vec_input)
	{
		std::vector<SimulationUnit*> result;
		result.reserve(vec_input.size());
		for (std::vector<SimulationUnit>::iterator itr = vec_input.begin();
				itr != vec_input.end(); itr++)
			result.push_back(&*itr);
		return result;
	}
}


// The name of the log file to which output from all dprintf and fail calls is sent
const xtring file_log = "herbivsim.log";

/** \def NO_GUESS_PARAMETERS
 * \brief Preprocessor flag that the herbivory test simulation is compiled.
 * In the herbivory test simulation, the global options are obsolete, and we need to
 * suppress warning messages about missing parameters.
 */

/// Run the test simulation with parameters read from instruction file
/** \todo Print version, print help */
int main(int argc,char* argv[]) {
	try {
		// Set a shell for dprintf() etc
		set_shell(new CommandLineShell(file_log));

		dprintf("This is the test simulator for the herbivory module "
				"of LPJ-GUESS.\n");

		// The singleton instance of FaunaSim::Manager
		Framework& framework = Framework::get_instance();

		// Read ins file from command line parameters.
		// we expect one argument: the ins file name
		if (argc == 2) {
			if (std::string(argv[1]) == "-help")
				plibhelp();
			else {
				// Read the instruction file to obtain simulation settings
				const char* instruction_filename = argv[1];

				if (!fileexists(instruction_filename))
					fail("Could not open instruction file");

				// let plib parse the instruction script
				try {
					// plib doesn’t use exceptions, it just returns zero on error.
					if (!plib(instruction_filename)) 
						fail("Bad instruction file!");
				} catch (const std::exception e){
					dprintf("An exception occurred while reading the instruction "
							"file: \n%s\n", e.what());
					fail();
				}
			}
		}
		else {
			fprintf(stderr, 
					"Exactly one parameter expected.\n"
					"Usage: %s <instruction-script-filename> | -help\n",
					argv[0]);
			exit(EXIT_FAILURE);
		}

		// store the parameters
		assert(ParamReader::get_instance().parsing_completed());
		const Parameters params = ParamReader::get_instance().get_params();
		const HftList hftlist = ParamReader::get_instance().get_hftlist();

		// Run the simulation with the global parameters
		const bool success = framework.run(params, hftlist);
		if (!success){
			dprintf("Exiting simulation.");
			return EXIT_FAILURE;
		}

	} catch (const std::exception& e) {
		dprintf("Unhandled exception:\n%s", e.what());
		return EXIT_FAILURE;
	}

	dprintf("\nFinished\n");
	return EXIT_SUCCESS;
}





////////////////////////////////////////////////////////

const int Framework::COORDINATES_PRECISION = 0;

std::auto_ptr<Habitat> Framework::create_habitat()const{
	return std::auto_ptr<Habitat>(
			new SimpleHabitat(params.habitat));
}

void Framework::declare_parameters(){

	static bool parameters_declared = false;

	if (!parameters_declared) {
		// General options
		declare_parameter("outputdirectory", 
				&params.outputdirectory, 
				300, // string length
				"Directory for the output files");
		mandatory_parameters.push_back("outputdirectory");

		declare_parameter("nyears",
				&params.nyears,
				1,INT_MAX, // min, max
				"Number of simulation years.");
		mandatory_parameters.push_back("nyears");

		declare_parameter("nhabitat_groups",
				&params.ngroups,
				1, INT_MAX, // min, max
				"Number of habitat groups.");
		mandatory_parameters.push_back("nhabitat_groups");

		declare_parameter("nhabitats_per_group",
				&params.nhabitats_per_group,
				1, INT_MAX, // min, max
				"Number of habitats per group.");
		mandatory_parameters.push_back("nhabitats_per_group");

		// Grass growth 
		declare_parameter("grass_decay",
				&params.habitat.grass.decay,
				0.0, DBL_MAX, // min, max
				"Proportional daily grass decay rate.");
		mandatory_parameters.push_back("grass_decay");

		declare_parameter("grass_digestibility",
				&params.habitat.grass.digestibility,
				DBL_MIN, 1.0, // min, max
				"Fractional grass digestibility.");
		mandatory_parameters.push_back("grass_digestibility");

		declare_parameter("grass_fpc",
				&params.habitat.grass.fpc,
				0.0, 1.0, // min, max
				"Foliar Percentage Cover of the grass.");
		mandatory_parameters.push_back("grass_fpc");

		declare_parameter("grass_growth",
				&params.habitat.grass.growth,
				0.0, DBL_MAX, // min, max
				"Proportional daily grass growth rate.");
		mandatory_parameters.push_back("grass_growth");

		declare_parameter("grass_init_mass",
				&params.habitat.grass.init_mass,
				0.0, DBL_MAX, // min, max
				"Initial grass biomass [kgDM/km²]");
		mandatory_parameters.push_back("grass_init_mass");

		declare_parameter("grass_reserve",
				&params.habitat.grass.reserve,
				0.0, DBL_MAX, // min, max
				"Ungrazable grass reserve [kgDM/km²]");
		mandatory_parameters.push_back("grass_reserve");

		declare_parameter("grass_saturation",
				&params.habitat.grass.saturation,
				0.0, DBL_MAX, // min, max
				"Saturation grass biomass [kgDM/km²]");
		mandatory_parameters.push_back("grass_saturation");

		parameters_declared = true;
	}
}

void Framework::plib_callback(int callback) {
	// Simply check each global parameter in the list
	if (callback == CB_CHECKGLOBAL) {
		for (int i=0; i<mandatory_parameters.size(); i++) {
			const std::string& item = mandatory_parameters[i];
			if (!itemparsed(item.c_str())){
				dprintf("Error: %s was not defined in the instruction file.\n",
						item.c_str());
				fail();
			} 	
		}
	}
}

bool Framework::run(const Fauna::Parameters& global_params,
		const HftList& hftlist){

	// PREPARE OUTPUT
	// Since we only use HerbivoryOutput here, we don’t use the
	// output module registry. 
	// Instead, the relevant functions are called directly.
	if (params.outputdirectory=="") 
		fail("No output directory given in the .ins file!");

	try {
		output_channel = new FileOutputChannel(
				params.outputdirectory.c_str(),
				COORDINATES_PRECISION);
		herbiv_out.set_hftlist(hftlist); 
		herbiv_out.init();
	} catch (const std::exception& e){
		dprintf("Exception during output initialization:\n%s\n",
				e.what());
	}


	// PREPARE VARIABLES

	// The simulator for the habitats
	// Pass the global parameters that were read from the 
	// instruction file.
	Simulator habitat_simulator(global_params, hftlist);

	dprintf("Creating ecosystem with habitats and herbivores.\n");

	// Container for all the groups, each being a vector of
	// simulation units.
	HabitatGroupList groups;
	groups.reserve( params.nhabitats_per_group );
	for (int g=0; g<params.ngroups;g++){

		// This is only for labelling the output:
		const double lon = (double) g;
		const double lat = (double) g;

		HabitatGroup& new_group = groups.add(
				std::auto_ptr<HabitatGroup>(
					new HabitatGroup(lon, lat)));
		
		// Fill one group with habitats and populations
		for (int h=0; h<params.nhabitats_per_group; h++){
			try {
				// create a new pair of habitat and populations
				new_group.add( std::auto_ptr<SimulationUnit>(
							new SimulationUnit(
								create_habitat(),
								habitat_simulator.create_populations())));

			} catch (const std::exception& e){
				dprintf("Exception during habitat creation:\n"
						"group number %d of %d\n"
						"habitat number %d of %d\n"
						"Exception message:\n\t%s",
						g, params.ngroups,
						h, params.nhabitats_per_group,
						e.what());
				return false;
			}
		}

	}

	dprintf("Starting simulation.\n");

	for (int year=0; year < params.nyears; year++) {
		for (int day_of_year=0; day_of_year < 365; day_of_year++){

			// loop through habitat groups
			for (HabitatGroupList::iterator itr_g = groups.begin();
					itr_g != groups.end();
					itr_g++)
			{
				HabitatGroup& group = **itr_g;

				//loop through SimulationUnit objects in this group
				for (HabitatGroup::iterator itr_u = group.begin();
						itr_u != group.end();
						itr_u++)
				{
					SimulationUnit& simulation_unit = **itr_u;

					// VEGATATION AND HERBIVORE SIMULATION
					const bool do_herbivores = 
						global_params.ifherbivory && 
						(year >= global_params.free_herbivory_years);

					try {
						habitat_simulator.simulate_day(
								day_of_year, 
								simulation_unit,
								do_herbivores); 
					} catch (const std::exception& e){
						dprintf("Exception during herbivore simulation:\n\%s",
								e.what());
						return false;
					}
				}

				// Write output
				herbiv_out.outdaily(
						group.get_lon(), // longitude (only for labelling)
						group.get_lat(), // latitude  (only for labelling)
						day_of_year, 
						year, // simulation_year
						year, // calendar_year, the same as there is no calendar
						group.get_vector());
			} // end of habitat group loop

		}// day loop: end of year

		// PRINT PROGRESS
		const int progress_interval = params.nyears / 10; // every 10%
		if (year % progress_interval == 0
				|| year == params.nyears-1)
			dprintf("progress: %d%%\n", (100*year)/(params.nyears-1)); 
	} // year loop

	return true; // success!
}

