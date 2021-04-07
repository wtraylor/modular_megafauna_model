// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Function object to perform simulations in the herbivore model.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_SIMULATE_DAY_H
#define FAUNA_SIMULATE_DAY_H

#include <map>
#include "environment.h"
#include "habitat_forage.h"
#include "herbivore_vector.h"
#include "population_list.h"

namespace Fauna {
// Forward declarations
class FeedHerbivores;
class Habitat;
class PopulationInterface;
class SimulationUnit;

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
   * 1. Initialize habitat.
   * 2. Simulate herbivores.
   * 3. Feed herbivores.
   * 4. Create potential offspring.
   * 5. Delete dead herbivores.
   *
   * \param do_herbivores Whether the herbivore objects shall be
   * simulated. Otherwise only the habitat is initialized.
   * \param establish_as_needed Whether to (re-)establish herbivores in those
   * populations that are empty. This only has effect if also
   * `do_herbivores==true`
   */
  void operator()(const bool do_herbivores, const bool establish_as_needed);

 private:  // HELPER FUNCTIONS
  /// Create the offspring counted in \ref total_offspring.
  /**
   * For each HFT, let the PopulationInterface object create herbivores.
   * These new herbivores will be counted in the output next simulation
   * cycle.
   */
  void create_offspring();

  /// Read available forage and set it to zero if it is very low.
  /**
   * Set any marginally small values to zero in order to avoid errors
   * caused by rounding inaccuracy.
   * This is done here and not in \ref Habitat for the sake of
   * decoupling: The \ref Habitat shouldn’t be concerned with the
   * herbivore feeding.
   */
  static HabitatForage get_corrected_forage(const Habitat&);

  /// Get references to all herbivores in the list of populations.
  static std::map<PopulationInterface*, HerbivoreVector> get_herbivores(
      const PopulationList&);

  /// Iterate over all \ref herbivores and let them do their simulation.
  /**
   * Call \ref HerbivoreInterface::simulate_day() in each alive herbivore
   * object. Also collect offspring.
   */
  void simulate_herbivores();

 private:  // MEMBER VARIABLES
  /// Julian day of year (0 = Jan 1st).
  const int day_of_year;

  /// The current abiotic conditions.
  const HabitatEnvironment environment;

  /// Function object doing the feeding.
  const FeedHerbivores& feed_herbivores;

  /// Pointers to all herbivores in the habitat.
  std::map<PopulationInterface*, HerbivoreVector> herbivores;

  /// All offspring for each population today [ind/km²]
  std::map<PopulationInterface*, double> total_offspring;

  /// Reference to the simulation unit.
  SimulationUnit& simulation_unit;
};
}  // namespace Fauna

#endif  // FAUNA_SIMULATE_DAY_H
