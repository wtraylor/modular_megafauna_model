#ifndef WORLD_H
#define WORLD_H

#include <list>
#include <memory>

namespace Fauna {
// Forward declarations
class Date;
class Habitat;
class Hft;
class HftList;
class InsfileContent;
class Parameters;
class PopulationInterface;
class SimulationUnit;
class WorldConstructor;

/// Central class to construct and own megafauna habitats and populations.
class World {
 public:
  /// Constructor: Read parameters and HFTs from instruction file.
  /**
   * \todo Shall this throw an exception on malformed instruction file? The way
   * the framework() function in LPJ-GUESS is constructed it odes not work well
   * to catch an exception. So perhaps Fauna::World::World() should just write
   * to stderr and *then* throw an exception?
   *
   * \param instruction_filename Path to the instruction file for the megafauna
   * model. It contains global settings and herbivore parameters.
   */
  World(const std::string instruction_filename);

  /// Default destructor.
  ~World();

  /// Compose a new simulation from an external habitat and new populations.
  /**
   * The new simulation unit will be owned by \ref World and released as soon
   * as the \ref Habitat instance is marked as to be deleted from extern.
   * \param habitat Pointer to an instance of a class that implements \ref
   * Habitat, given externally by the vegetation model. The World object takes
   * over ownership of the pointer, but will not delete it before it is marked
   * as dead (\ref Habitat::kill()) externally.
   * \throw std::invalid_argument If `habitat` is NULL.
   */
  void create_simulation_unit(Habitat* habitat);

  /// Get global simulation parameters.
  /**
   * The global megafauna parameters are public because they might be required
   * by the calling vegetation model to check back against vegetation-specific
   * settings.
   * \throw std::logic_error If the member variable hasn’t been set.
   */
  const Parameters& get_params() const;

  /// Iterate through all simulation units and perform simulation for this day.
  /**
   * This is the central access point to start the perform the herbivore
   * simulations.
   *
   * If a \ref Habitat instance is marked as dead, the corresponding simulation
   * unit will be released from memory.
   * \param date The current simulation day.
   * \param do_herbivores Whether to perform herbivore simulations. If false,
   * only the output data of the habitats are updated.
   * \throw std::logic_error If `date` has not been correctly incremented by one day since the last call.
   */
  void simulate_day(const Date& date, const bool do_herbivores);

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

  /// Number of days since extinct populations were re-established.
  int days_since_last_establishment;

  /// List of all the simulation units in the world.
  /**
   * All objects are owned by \ref World.
   */
  std::list<SimulationUnit> sim_units;

  /// Helper class to construct various elements of the megafauna world.
  const std::unique_ptr<WorldConstructor> world_constructor;
};
}  // namespace Fauna
#endif  // WORLD_H
