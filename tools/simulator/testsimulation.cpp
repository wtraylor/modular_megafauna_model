//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Herbivory test simulation independent of the LPJ-GUESS %framework.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
//////////////////////////////////////////////////////////////////////////

#include "testsimulation.h"
#include <cassert>
#include <cfloat>   // for DBL_MAX
#include <climits>  // for INT_MAX
#include "config.h"
#include "framework.h"
#include "parameters.h"  // for declare_parameter()
#include "paramreader.h"
#include "plib.h"   // for plib() and itemparsed() and declareitem()
#include "shell.h"  // for dprintf()
#include "simulation_unit.h"

using namespace Fauna;
using namespace FaunaSim;
using namespace GuessOutput;

// Anonymous namespace with definitions local to this file
namespace {
/// Creates a vector of pointers from a vector of
/// \ref SimulationUnit objects.
std::vector<SimulationUnit*> simunit_vector_to_pointers(
    std::vector<SimulationUnit> vec_input) {
  std::vector<SimulationUnit*> result;
  result.reserve(vec_input.size());
  for (std::vector<SimulationUnit>::iterator itr = vec_input.begin();
       itr != vec_input.end(); itr++)
    result.push_back(&*itr);
  return result;
}

/// Helper to read parameter \ref LogisticGrass::Parameters::digestibility.
double param_grass_digestibility[12];
/// Helper to read parameter \ref LogisticGrass::Parameters::decay_monthly.
double param_monthly_grass_decay[12];
/// Helper to read parameter \ref LogisticGrass::Parameters::growth_monthly.
double param_monthly_grass_growth[12];
/// Helper to read parameter \ref LogisticGrass::Parameters::snow_depth_monthly.
double param_monthly_snow_depth[12];
}  // namespace

// The name of the log file to which output from all dprintf and fail calls is
// sent
const xtring file_log = "herbivsim.log";

/// Run the test simulation with parameters read from instruction file
/** \todo Print version, print help */
int main(int argc, char* argv[]) {
  try {
    // Set a shell for dprintf() etc
    set_shell(new CommandLineShell(file_log));

    dprintf(
        "This is the test simulator for the herbivory module "
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
          if (!plib(instruction_filename)) fail("Bad instruction file!");
        } catch (const std::exception e) {
          dprintf(
              "An exception occurred while reading the instruction "
              "file: \n%s\n",
              e.what());
          fail();
        }
      }
    } else {
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
    if (!success) {
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

std::auto_ptr<Habitat> Framework::create_habitat() const {
  return std::auto_ptr<Habitat>(new SimpleHabitat(params.habitat));
}

void Framework::plib_declare_parameters() {
  static bool parameters_declared = false;

  if (!parameters_declared) {
    // General options
    declare_parameter("outputdirectory", &params.outputdirectory,
                      300,  // string length
                      "Directory for the output files");
    mandatory_parameters.push_back("outputdirectory");

    declare_parameter("nyears", &params.nyears, 1, INT_MAX,  // min, max
                      "Number of simulation years.");
    mandatory_parameters.push_back("nyears");

    declare_parameter("nhabitat_groups", &params.ngroups, 1,
                      INT_MAX,  // min, max
                      "Number of habitat groups.");
    mandatory_parameters.push_back("nhabitat_groups");

    declare_parameter("nhabitats_per_group", &params.nhabitats_per_group, 1,
                      INT_MAX,  // min, max
                      "Number of habitats per group.");
    mandatory_parameters.push_back("nhabitats_per_group");

    declareitem(
        "grass_decay", param_monthly_grass_decay, 0.0, DBL_MAX,  // min, max
        12,                                                      // value count
        CB_NONE, "12 proportional daily grass decay rates for each month.");
    mandatory_parameters.push_back("grass_decay");

    declareitem("grass_digestibility", param_grass_digestibility, DBL_MIN,
                1.0,  // min, max
                12,   // value count
                CB_NONE,
                "12 fractional grass digestibility values, for each month.");
    mandatory_parameters.push_back("grass_digestibility");

    declare_parameter("grass_fpc", &params.habitat.grass.fpc, 0.0,
                      1.0,  // min, max
                      "Foliar Percentage Cover of the grass.");
    mandatory_parameters.push_back("grass_fpc");

    declareitem(
        "grass_growth", param_monthly_grass_growth, 0.0, DBL_MAX,  // min, max
        12,  // value count
        CB_NONE, "12 proportional daily grass growth rates for each month.");
    mandatory_parameters.push_back("grass_growth");

    declare_parameter("grass_init_mass", &params.habitat.grass.init_mass, 0.0,
                      DBL_MAX,  // min, max
                      "Initial grass biomass [kgDM/km²]");
    mandatory_parameters.push_back("grass_init_mass");

    declare_parameter("grass_reserve", &params.habitat.grass.reserve, 0.0,
                      DBL_MAX,  // min, max
                      "Ungrazable grass reserve [kgDM/km²]");
    mandatory_parameters.push_back("grass_reserve");

    declare_parameter("grass_saturation", &params.habitat.grass.saturation, 0.0,
                      DBL_MAX,  // min, max
                      "Saturation grass biomass [kgDM/km²]");
    mandatory_parameters.push_back("grass_saturation");

    declareitem("snow_depth", param_monthly_snow_depth, 0.0,
                DBL_MAX,  // min, max
                12,       // value count
                CB_NONE, "12 snow depth values [cm] for each month.");
    mandatory_parameters.push_back("grass_growth");

    parameters_declared = true;
  }
}

void Framework::plib_callback(int callback) {
  // Simply check each global parameter in the list
  if (callback == CB_CHECKGLOBAL) {
    for (int i = 0; i < mandatory_parameters.size(); i++) {
      // Complain if a mandatory parameter is missing
      const std::string& item = mandatory_parameters[i];
      if (!itemparsed(item.c_str())) {
        dprintf("Error: %s was not defined in the instruction file.\n",
                item.c_str());
        fail();
      }
    }

    // Copy the monthly array values to std::vector
    params.habitat.grass.decay_monthly.clear();
    params.habitat.grass.digestibility.clear();
    params.habitat.grass.growth_monthly.clear();
    params.habitat.snow_depth_monthly.clear();
    for (int i = 0; i < 12; i++) {
      params.habitat.grass.decay_monthly.push_back(
          param_monthly_grass_decay[i]);
      params.habitat.grass.digestibility.push_back(
          param_grass_digestibility[i]);
      params.habitat.grass.growth_monthly.push_back(
          param_monthly_grass_growth[i]);
      params.habitat.snow_depth_monthly.push_back(param_monthly_snow_depth[i]);
    }
    assert(params.habitat.grass.decay_monthly.size() == 12);
    assert(params.habitat.grass.digestibility.size() == 12);
    assert(params.habitat.grass.growth_monthly.size() == 12);
    assert(params.habitat.snow_depth_monthly.size() == 12);
  }
}

bool Framework::run(const Fauna::Parameters& global_params,
                    const HftList& hftlist) {
  // PREPARE OUTPUT
  // Since we only use HerbivoryOutput here, we don’t use the
  // output module registry.
  // Instead, the relevant functions are called directly.
  if (params.outputdirectory == "")
    fail("No output directory given in the .ins file!");

  try {
    output_channel = new FileOutputChannel(params.outputdirectory.c_str(),
                                           COORDINATES_PRECISION);
    herbiv_out.set_hftlist(hftlist);
    herbiv_out.init();
  } catch (const std::exception& e) {
    dprintf("Exception during output initialization:\n%s\n", e.what());
  }

  // PREPARE VARIABLES

  // The simulator for the habitats
  // Pass the global parameters that were read from the
  // instruction file.
  Simulator habitat_simulator(global_params);

  dprintf("Creating ecosystem with habitats and herbivores.\n");

  // Container for all the groups, each being a vector of
  // simulation units.
  HabitatGroupList groups;
  groups.reserve(params.nhabitats_per_group);
  for (int g = 0; g < params.ngroups; g++) {
    // This is only for labelling the output:
    const double lon = (double)g;
    const double lat = (double)g;

    HabitatGroup& new_group =
        groups.add(std::auto_ptr<HabitatGroup>(new HabitatGroup(lon, lat)));

    // Fill one group with habitats and populations
    for (int h = 0; h < params.nhabitats_per_group; h++) {
      try {
        // Create herbivore populations for this patch
        std::auto_ptr<Fauna::HftPopulationsMap> pops;
        if (global_params.one_hft_per_patch) {
          // Create only one HFT in the habitat.
          pops = habitat_simulator.create_populations(
              &hftlist[h % hftlist.size()]);
        } else {
          // Create all HFTs in the habitat.
          pops = habitat_simulator.create_populations(hftlist);
        }

        // create a new pair of habitat and populations
        new_group.add(std::auto_ptr<SimulationUnit>(
            new SimulationUnit(create_habitat(), pops)));

      } catch (const std::exception& e) {
        dprintf(
            "Exception during habitat creation:\n"
            "group number %d of %d\n"
            "habitat number %d of %d\n"
            "Exception message:\n\t%s",
            g, params.ngroups, h, params.nhabitats_per_group, e.what());
        return false;
      }
    }
  }

  dprintf("Starting simulation.\n");

  for (int year = 0; year < params.nyears; year++) {
    for (int day_of_year = 0; day_of_year < 365; day_of_year++) {
      // loop through habitat groups
      for (HabitatGroupList::iterator itr_g = groups.begin();
           itr_g != groups.end(); itr_g++) {
        HabitatGroup& group = **itr_g;

        // loop through SimulationUnit objects in this group
        for (HabitatGroup::iterator itr_u = group.begin(); itr_u != group.end();
             itr_u++) {
          SimulationUnit& simulation_unit = **itr_u;

          // VEGATATION AND HERBIVORE SIMULATION
          const bool do_herbivores =
              global_params.ifherbivory &&
              (year >= global_params.free_herbivory_years);

          try {
            habitat_simulator.simulate_day(day_of_year, simulation_unit,
                                           do_herbivores);
          } catch (const std::exception& e) {
            dprintf("Exception during herbivore simulation:\n\%s", e.what());
            return false;
          }
        }

        // Write output
        herbiv_out.outdaily(
            group.get_lon(),  // longitude (only for labelling)
            group.get_lat(),  // latitude  (only for labelling)
            day_of_year,
            year,  // simulation_year
            year,  // calendar_year, the same as there is no calendar
            group.get_vector());
      }  // end of habitat group loop

    }  // day loop: end of year

    // PRINT PROGRESS
    const int progress_interval = params.nyears / 10;  // every 10%
    if (year % progress_interval == 0 || year == params.nyears - 1)
      dprintf("progress: %d%%\n", (100 * year) / (params.nyears - 1));
  }  // year loop

  return true;  // success!
}
