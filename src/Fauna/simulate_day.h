//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Function objects to perform simulations in the herbivore model.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////
#ifndef SIMULATE_DAY_H
#define SIMULATE_DAY_H

#include <map>              // for ForageDistribution
#include <vector>           // for HerbivoreVector
#include "combined_data.h"  // for Output::CombinedData
#include "forageclasses.h"  // for HabitatForage

namespace Fauna {
// Forward declarations
class HerbivoreInterface;
class SimulationUnit;
class Habitat;
class Hft;
class FeedHerbivores;

/// A vector of herbivore pointers.
/** Originally defined in \ref population.h */
typedef std::vector<HerbivoreInterface*> HerbivoreVector;

/// Function object to simulate one day in one habitat.
/**
 * This class is very high in the framework hierarchy and should
 * therefore be kept as slim as possible. It should only call well
 * encapsulated other functions and classes.
 */
class SimulateDay {
 public:
  /// Constructor
  /**
   * \param day_of_year Current day of year (0 = Jan 1st)
   * \param simulation_unit The habitat and herbivores to
   * simulate.
   * \param feed_herbivores Function object used to give forage to the
   * herbivores.
   * \throw std::invalid_argument If day_of_year not in [0,364].
   */
  SimulateDay(const int day_of_year, SimulationUnit& simulation_unit,
              const FeedHerbivores& feed_herbivores);

  /// Simulate one day.
  /**
   * - Initiailize habitat.
   * - Establish herbivores if needed.
   * - Simulate herbivores.
   * - Feed herbivores.
   * - Aggregate output.
   * - Create potential offspring.
   * - Delete dead herbivores.
   *
   * \param do_herbivores Whether the herbivore objects shall be
   * simulated. Otherwise only the habitat is initialized and the
   * output aggregated.
   * \param establish_if_needed Whether (re-)establishment shall be
   * done for those HFTs that have no population. (This needs
   * `do_herbivores==true`.)
   */
  void operator()(const bool do_herbivores, const bool establish_if_needed);

 private:  // HELPER FUNCTIONS
  /// Merge HFT and habitat output into output of simulation unit.
  void aggregate_output();

  /// Create the offspring counted in \ref total_offspring.
  /**
   * For each HFT, let the PopulationInterface object create herbivores.
   * These new herbivores will be counted in the output next simulation
   * cycle.
   */
  void create_offspring();

  /// Iterate through HFTs and (re-)establish if they are not there.
  void do_establishment();

  /// Read available forage and set it to zero if it is very low.
  /**
   * Set any marginally small values to zero in order to avoid errors
   * caused by rounding inaccuracy.
   * This is done here and not in \ref Habitat for the sake of
   * decoupling: The \ref Habitat shouldn’t be concerned with the
   * herbivore feeding.
   */
  static HabitatForage get_corrected_forage(const Habitat&);

  /// Iterate over all \ref herbivores and let them do their simulation.
  /**
   * Call \ref HerbivoreInterface::simulate_day() in each alive herbivore
   * object. Also collect offspring and excreted nitrogen.
   * If an herbivore is dead, all its nitrogen will be returned to the
   * \ref Habitat.
   */
  void simulate_herbivores();

 private:  // MEMBER VARIABLES
  /// Julian day of year (0 = Jan 1st).
  const int day_of_year;

  /// The current abiotic conditions.
  const HabitatEnvironment environment;

  /// Total nitrogen excreted by herbivores today [kgN/km²].
  double excreted_nitrogen;

  /// Function object doing the feeding.
  const FeedHerbivores& feed_herbivores;

  /// Available forage in habitat before feeding.
  const HabitatForage forage_before_feeding;

  /// Pointers to all herbivores in the habitat.
  HerbivoreVector herbivores;

  /// Habitat and herbivore output for this day.
  Output::CombinedData todays_datapoint;

  /// All offspring for each HFT today [ind/km²]
  std::map<const Hft*, double> total_offspring;

  /// Reference to the simulation unit.
  SimulationUnit& simulation_unit;
};
}  // namespace Fauna

#endif  // SIMULATE_DAY_H
