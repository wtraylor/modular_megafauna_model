///////////////////////////////////////////////////////////////////////////////////////
/// \file herbiv_testsimulation.cpp
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
#include "herbiv_output.h"
#include "herbiv_parameters.h"
#include <climits> // for INT_MAX
#include <cfloat> // for DBL_MAX

using namespace Fauna;
using namespace GuessOutput;

// The name of the log file to which output from all dprintf and fail calls is sent
const xtring file_log = "herbivsim.log";

/** \def NO_GUESS_PARAMETERS
 * \brief Preprocessor flag that the herbivory test simulation is compiled.
 * In the herbivory test simulation, the global options are obsolete, and we need to
 * suppress warning messages about missing parameters.
 */

/// Run the test simulation with parameters read from instruction file
/** \todo Print version */
int main(int argc,char* argv[]) {
	// Set a shell for dprintf() etc
	set_shell(new CommandLineShell(file_log));

	dprintf("This is the test simulator for the herbivory module "
			"of LPJ-GUESS.\n");

	/// The singleton instance of the simulator
	TestSimulator& simulator = TestSimulator::get_instance();

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
			if (!plib(instruction_filename)) 
				fail("Bad instruction file!");
		}
	}
	else {
		fprintf(stderr, 
				"Exactly one parameter expected.\n"
				"Usage: %s <instruction-script-filename> | -help\n",
				argv[0]);
		exit(EXIT_FAILURE);
	}

	// Run the simulation
	simulator.run();

	dprintf("\nFinished\n");
	return EXIT_SUCCESS;
}





////////////////////////////////////////////////////////

const int TestSimulator::COORDINATES_PRECISION = 0;

void TestSimulator::declare_parameters(){

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

		declare_parameter("veg_spinup_years",
				&params.veg_spinup_years,
				1,INT_MAX, // min, max
				"Years without herbivory.");
		mandatory_parameters.push_back("veg_spinup_years");

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
				&params.settings.grass.decay,
				0.0, DBL_MAX, // min, max
				"Proportional daily grass decay rate.");
		mandatory_parameters.push_back("grass_decay");

		declare_parameter("grass_digestibility",
				&params.settings.grass.digestibility,
				DBL_MIN, 1.0, // min, max
				"Fractional grass digestibility.");
		mandatory_parameters.push_back("grass_digestibility");

		declare_parameter("grass_fpc",
				&params.settings.grass.fpc,
				0.0, 1.0, // min, max
				"Foliar Percentage Cover of the grass.");
		mandatory_parameters.push_back("grass_fpc");

		declare_parameter("grass_growth",
				&params.settings.grass.growth,
				0.0, DBL_MAX, // min, max
				"Proportional daily grass growth rate.");
		mandatory_parameters.push_back("grass_growth");

		declare_parameter("grass_init_mass",
				&params.settings.grass.init_mass,
				0.0, DBL_MAX, // min, max
				"Initial grass biomass [kgDM/m²]");
		mandatory_parameters.push_back("grass_init_mass");

		declare_parameter("grass_reserve",
				&params.settings.grass.reserve,
				0.0, DBL_MAX, // min, max
				"Ungrazable grass reserve [kgDM/m²]");
		mandatory_parameters.push_back("grass_reserve");

		declare_parameter("grass_saturation",
				&params.settings.grass.saturation,
				0.0, DBL_MAX, // min, max
				"Saturation grass biomass [kgDM/m²]");
		mandatory_parameters.push_back("grass_saturation");

		parameters_declared = true;
	}
}

void TestSimulator::plib_callback(int callback) {
	// Simply check each parameter in the list
	for (int i=0; i<mandatory_parameters.size(); i++) {
		const std::string& item = mandatory_parameters[i];
		if (!itemparsed(item.c_str())){
			dprintf("Error: %s was not defined in the instruction file.\n",
					item.c_str());
			fail();
		} 	
	}
}

void TestSimulator::run(){

	// PREPARE OUTPUT
	// Since we only use HerbivoryOutput here, we don’t use the output
	// module registry. Instead, the relevant functions are called
	// directly.
	if (params.outputdirectory=="") 
		fail("No output directory given in the .ins file!");

	output_channel = new FileOutputChannel(
			params.outputdirectory.c_str(),
			COORDINATES_PRECISION);
	herbiv_out.init();


	// PREPARE VARIABLES

	/// The habitat groups in this run
	std::vector<TestHabitatGroup> habitat_groups;
	for (int g=0; g<params.ngroups; g++){
		// The x and y coordinates are just for labeling the
		// lon/lat columns in the output.
		const double x = (double) g;
		const double y = (double) g;

		// create a habitat group which in turn creates its habitats
		habitat_groups.push_back( 
				TestHabitatGroup(
					x,y,
					params.nhabitats_per_group, 
					params.settings) 
				);
	}

	/// The simulator for the habitats
	// Pass the global parameters that were read from the 
	// instruction file.
	Simulator habitat_simulator(Parameters::get_global());

	/// Time counters
	int year, day_of_year;


	// START THE SIMULATION

	dprintf("Starting simulation.\n");

	for (int year=0; year < params.nyears; year++) {
		for (int day_of_year=0; day_of_year < 365; day_of_year++){

			// loop through habitat groups
			for (int g=0; g<habitat_groups.size(); g++) {
				TestHabitatGroup& group = habitat_groups[g];
				//loop through habitat objects in this group
				std::vector<TestHabitat>& group_habitats = group.get_habitats(); 
				for (int h=0; h<group_habitats.size(); h++)
				{
					TestHabitat &habitat =  group_habitats[h];

					// VEGATATION AND HERBIVORE SIMULATION
					const bool do_herbivores = (year >= params.veg_spinup_years);
					habitat_simulator.simulate_day(day_of_year, habitat, do_herbivores); 
				}
			} 
		}// end of year


		// OUTPUT

		// Write output for all habitat groups
		for (int g=0; g<habitat_groups.size(); g++) {
			TestHabitatGroup& group = habitat_groups[g];
			herbiv_out.outannual(group.get_lon(), group.get_lat(), 
					day_of_year, year,
					group.get_habitat_references());
		}
	}
}
