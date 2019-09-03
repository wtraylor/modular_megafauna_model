#include "demo_simulator.h"
#include <cassert>
#include <cfloat>
#include <climits>
#include <iostream>
#include "megafauna.h"
#include "simple_habitat.h"

using namespace Fauna;
using namespace Fauna::Demo;

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

/// Run the demo simulation with parameters read from instruction file
/** \todo Print version, print help */
int main(int argc, char* argv[]) {
  std::string insfile_fauna;
  std::string insfile_demo;
  try {
    std::cerr << "This is the demo simulator for the Modular Megafauna Model."
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
        insfile_demo = argv[2];
      }
    } else {
      framework.print_usage();
      return EXIT_FAILURE;
    }

    // Run the simulation with the global parameters
    const bool success = framework.run(insfile_fauna, insfile_demo);
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
  megafauna_demo_simulator <fauna_instruction_file> <simulation_instruction_file>
  megafauna_demo_simulator -help
)EOF";
}

bool Framework::run(const std::string insfile_fauna,
                    const std::string insfile_demo) {
  Fauna::Parameters global_params;
  World fauna_world(insfile_fauna);

  std::cerr << "Creating ecosystem with habitats and herbivores." << std::endl;

  // Container for all the groups, each being a vector of
  // simulation units.
  for (int g = 0; g < params.ngroups; g++) {
    const std::string aggregation_unit = std::to_string(g);
    // Fill one group with habitats and populations
    for (int h = 0; h < params.nhabitats_per_group; h++) {
      try {
        // We only pass the pointer to the new habitat to the megafauna
        // library, so special care is needed that it will stay valid.
        fauna_world.create_simulation_unit(
            new SimpleHabitat(params.habitat, aggregation_unit));
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
      const Date date(day_of_year, year);
      try {
        // The Fauna::World class will take care to iterate over all habitat
        // groups.
        fauna_world.simulate_day(date, do_herbivores);
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
