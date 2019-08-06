//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Herbivory test simulation independent of the LPJ-GUESS %framework.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
//////////////////////////////////////////////////////////////////////////

#include "testsimulation.h"
#include <cassert>
#include <cfloat>   // for DBL_MAX
#include <climits>  // for INT_MAX
#include <iostream>
#include "megafauna.h"

using namespace Fauna;
using namespace FaunaSim;

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

/// Run the test simulation with parameters read from instruction file
/** \todo Print version, print help */
int main(int argc, char* argv[]) {
  try {
    std::cerr << "This is the test simulator for the Modular Megafauna Model."
              << std::endl;

    // The singleton instance of FaunaSim::Manager
    Framework& framework = Framework::get_instance();

    // Read ins file from command line parameters.
    // We expect two arguments: the two instruction files.
    if (argc == 3) {
      if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-help")
        framework.print_help();
      else {
        // Read the instruction file to obtain simulation settings
        const std::string insfile_fauna = argv[1];
        const std::string insfile_testsim = argv[2];

        std::cerr << "Instruction file is not yet supported." << std::endl;
        return EXIT_FAILURE;
      }
    } else {
      framework.print_usage();
      return EXIT_FAILURE;
    }

    // Run the simulation with the global parameters
    const bool success = framework.run(insfile_fauna, insfile_testsim);
    if (!success) {
      std::cerr << "Exiting simulation." << std::endl;
      return EXIT_FAILURE;
    }

  } catch (const std::exception& e) {
    std::cerr << "Unhandled exception:\n" << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  std::cerr << "Successfully finished." << std::endl;
  return EXIT_SUCCESS;
}

////////////////////////////////////////////////////////

std::auto_ptr<Habitat> Framework::create_habitat() const {
  return std::auto_ptr<Habitat>(new SimpleHabitat(params.habitat));
}

void Framework::print_help() {
  // We use C++11 raw string literals like a Bash Here Document.
  // TODO: complete help message
  std::cout << R"EOF(
This is a stub help message.
)EOF";
}

void Framework::print_usage() {
  // We use C++11 raw string literals like a Bash Here Document.
  std::cerr << R"EOF(
Usage:
  megafauna_test_simulator <fauna_instruction_file> <simulation_instruction_file>
  megafauna_test_simulator -help
)EOF";
}

bool Framework::run(const std::string insfile_fauna,
                    const std::string insfile_testsim) {
  Fauna::Parameters global_params;
  World fauna_world(insfile_fauna);

  std::cerr << "Creating ecosystem with habitats and herbivores." << std::endl;

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
        std::cerr << "Exception during habitat creation:" << std::endl
                  << "group number " << g << " of " << params.ngroups << '\n'
                  << "habitat number " << h << " of "
                  << params.nhabitats_per_group << '\n'
                  << "Exception message:\n"
                  << std::endl
                  << e.what();
        return false;
      }
    }
  }

  std::cerr << "Starting simulation." << std::endl;

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
          const bool do_herbivores = true;

          try {
            habitat_simulator.simulate_day(day_of_year, simulation_unit,
                                           do_herbivores);
          } catch (const std::exception& e) {
            std::cerr << "Exception during herbivore simulation:\n"
                      << e.what() << std::endl;
            return false;
          }
        }

        // Write output
        // As a makeshift we just print something to STDOUT.
        std::cout << group.get_lon() << '\t' << group.get_lat() << '\t'
                  << day_of_year << '\t' << year << '\t';
        for (const auto& i : group.get_vector())
          std::cout
              << i->get_output().habitat_data.available_forage.grass.get_mass();
        std::cout << std::endl;
      }  // end of habitat group loop

    }  // day loop: end of year

    // PRINT PROGRESS
    const int progress_interval = params.nyears / 10;  // every 10%
    if (year % progress_interval == 0 || year == params.nyears - 1)
      std::cerr << "Progress: " << (100 * year) / (params.nyears - 1)
                << std::endl;
  }  // year loop

  return true;  // success!
}
