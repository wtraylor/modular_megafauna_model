#ifndef WORLD_H
#define WORLD_H

#include <list>
#include <memory>
#include "parameters.h"

namespace Fauna {
// Forward declarations
class Habitat;
class Hft;
class HftList;
class PopulationInterface;
class SimulationUnit;
class InsfileContent;

/// Central class to construct and own megafauna habitats and populations.
class World {
 public:
  /// Constructor: Read parameters and HFTs from instruction file.
  World(const std::string instruction_filename);

  /// Compose a new simulation from an external habitat and new populations.
  /**
   * The new simulation unit will be owned by \ref World and released as soon
   * as the \ref Habitat instance is marked as to be deleted from extern.
   * \param habitat Habitat instance given externally by the vegetation model.
   */
  void create_simulation_unit(std::auto_ptr<Habitat> habitat);

  /// Get global simulation parameters.
  /**
   * The global megafauna parameters are public because they might be required
   * by the calling vegetation model to check back against vegetation-specific
   * settings.
   * \throw std::logic_error If the member variable hasn’t been set.
   */
  const Parameters& get_params();

  /// Iterate through all simulation units and perform simulation for this day.
  /**
   * If a \ref Habitat instance is marked as dead, the corresponding simulation
   * unit will be released.
   * \param day_of_year Current day of year (0 = Jan 1st).
   * \param do_herbivores Whether to perform herbivore simulations. If false,
   * only the output data of the habitats are updated.
   * \throw std::invalid_argument If day_of_year not in [0,364].
   */
  void simulate_day(const int day_of_year, const bool do_herbivores);

 private:
  /// Get the immutable list of herbivore functional types.
  const HftList& get_hfts();

  /// The herbivore functional types as read from the instruction file.
  /**
   * There should be only one immutable list of HFTs and one set of parameters
   * in the whole simulation. We use `unique_pointer` here so that we don’t need
   * to include \ref hft.h here in the library interface. We implement it as a
   * `struct` so both `hftlist` and `params` can be set at once in the
   * constructor member initialization.
   *
   * Use \ref get_params() and \ref get_hfts() to access content.
   */
  const std::unique_ptr<InsfileContent> insfile_content;

  /// List of all the simulation units in the world.
  /**
   * All objects are owned by \ref World.
   */
  std::list<SimulationUnit> sim_units;
};
}  // namespace Fauna
#endif  // WORLD_H
