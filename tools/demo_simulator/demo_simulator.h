#ifndef FAUNA_DEMO_SIMULATOR_H
#define FAUNA_DEMO_SIMULATOR_H

#include <vector>
#include "simple_habitat.h"

namespace Fauna {
// forward declaration
class Habitat;
class HftList;
class Parameters;

namespace Demo {
/// Performs demo simulations for the Modular Megafauna Model.
/**
 * \see \ref sec_singleton for an explanation of the design pattern used.
 * \see \ref sec_testsimulations
 */
class Framework {
 public:
  /// Constructor.
  Framework() {}

  /// Get singleton instance of the class.
  /** Creates the object on first call. */
  static Framework& get_instance() {
    static Framework instance;
    return instance;
  }

  /// Print the help text to STDOUT.
  void print_help();

  /// Print the short usage text to STDERR.
  void print_usage();

  /// Run a simulation.
  /**
   * At all critical points, exceptions are caught.
   * \param insfile_fauna Path to the instruction file for the megafauna model.
   * \param insfile_demo Instruction file for the demo simulator framework.
   * \return true on success, false on failure
   */
  bool run(const std::string insfile_fauna, const std::string insfile_demo);

 private:
  /// Parameter values from instruction file
  /** The initialization values are just arbitrary. */
  struct {
    std::string outputdirectory = "./";
    int nyears = 100;
    int nhabitats_per_group = 4;
    int ngroups = 3;
    SimpleHabitat::Parameters habitat;
  } params;

  /// List of mandatory instruction file parameters.
  std::vector<std::string> mandatory_parameters;

  /// Number of decimal places in output tables.
  static const int COORDINATES_PRECISION;

  /// Deleted copy constructor
  Framework(Framework const&);  // don’t implement, it’s deleted
  /// Deleted assignment constructor
  void operator=(Framework const&);  // don’t implement, it’s deleted
};
}  // namespace Demo
}  // namespace Fauna

#endif  // FAUNA_DEMO_SIMULATOR_H
