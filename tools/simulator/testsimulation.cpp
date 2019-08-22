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
  std::string insfile_fauna;
  std::string insfile_testsim;
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
        insfile_fauna = argv[1];
        insfile_testsim = argv[2];
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
  for (int g = 0; g < params.ngroups; g++) {
    // Fill one group with habitats and populations
    for (int h = 0; h < params.nhabitats_per_group; h++) {
      try {
        // We only pass the pointer to the new habitat to the megafauna
        // library, so special care is needed that it will stay valid.
        fauna_world.create_simulation_unit(new SimpleHabitat(params.habitat));
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
      // VEGATATION AND HERBIVORE SIMULATION
      const bool do_herbivores = true;
      try {
        // The Fauna::World class will take care to iterate over all habitat
        // groups.
        fauna_world.simulate_day(day_of_year, do_herbivores);
      } catch (const std::exception& e) {
        std::cerr << "Exception during herbivore simulation:\n"
                  << e.what() << std::endl;
        return false;
      }
    }  // day loop: end of year

    // PRINT PROGRESS
    const int progress_interval = params.nyears / 10;  // every 10%
    if (year % progress_interval == 0 || year == params.nyears - 1)
      std::cerr << "Progress: " << (100 * year) / (params.nyears - 1)
                << std::endl;
  }  // year loop

  return true;  // success!
}
