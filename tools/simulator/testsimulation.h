//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Test simulations for the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
//////////////////////////////////////////////////////////////////////////

#ifndef TESTSIMULATION_H
#define TESTSIMULATION_H

#include <vector>
#include "testhabitat.h"   // for SimpleHabitat

namespace Fauna {
// forward declaration
class Habitat;
class HftList;
class Parameters;
}  // namespace Fauna
namespace FaunaSim {
/// Performs test simulations for herbivores outside of the LPJ-GUESS vegetation
/// model
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
   * \param insfile_testsim Instruction file for the testsimulator framework.
   * \return true on success, false on failure
   */
  bool run(const std::string insfile_fauna, const std::string insfile_testsim);

 private:
  /// Create a new habitat according to preferences.
  std::auto_ptr<Fauna::Habitat> create_habitat() const;

  /// Parameter values from instruction file
  struct {
    std::string outputdirectory;
    int nyears;
    int nhabitats_per_group, ngroups;
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
}  // namespace FaunaSim

#endif  // TESTSIMULATION_H
