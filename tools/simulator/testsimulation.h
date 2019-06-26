//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Test simulations for the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
//////////////////////////////////////////////////////////////////////////

#ifndef HERBIV_TESTSIMULATION_H
#define HERBIV_TESTSIMULATION_H

#include <vector>
#include "outputmodule.h"  // for HerbivoryOutput
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

  /// Run a simulation
  /** Call this only after ins file has been read.
   * At all critical points, exceptions are caught, but there
   * is no guarantee that no exception *slips through the cracks*.
   * \param global_params Instruction file parameters from
   * the herbivory module not specific to the test simulation.
   * \param hftlist List of HFTs
   * \return true on success, false on failure
   */
  bool run(const Fauna::Parameters& global_params,
           const Fauna::HftList& hftlist);

  /// Parameter check (called from \ref parameters.cpp).
  /**
   * Check if all mandatory parameters have been read, terminates on error.
   * This is a substitute for \ref plib_callback() in \ref parameters.cpp.
   * Uses \ref fail() to terminate.
   */
  void plib_callback(int callback);

  /// Declare instruction file parameters (called from \ref parameters.cpp).
  /**
   * Registers also mandatory parameters in \ref mandatory_parameters.
   * - \ref declare_parameter() from \ref parameters.h is used for
   *   regular parameters.
   * - \ref declareitem from \ref plib.h is used for items with multiple
   *   values.
   */
  void plib_declare_parameters();

 private:
  /// Create a new habitat according to preferences.
  std::auto_ptr<Fauna::Habitat> create_habitat() const;

  /// Output module
  GuessOutput::HerbivoryOutput herbiv_out;

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

#endif  // HERBIV_TESTSIMULATION_H
