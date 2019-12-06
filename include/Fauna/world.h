/**
 * \file
 * \brief Central management of all herbivores and habitats.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_WORLD_H
#define FAUNA_WORLD_H

#include <list>
#include <memory>
#include <vector>

namespace Fauna {
// Forward declarations
class Date;
class Habitat;
class Hft;
class Parameters;
class SimulationUnit;
class WorldConstructor;

// Repeat typedef from hft.h
typedef std::vector< std::shared_ptr<const Hft> > HftList;

namespace Output {
class Aggregator;
class WriterInterface;
}  // namespace Output

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
   *
   * \throw std::logic_error If selected \ref Parameters::output_format is not
   * implemented.
   */
  World(const std::string instruction_filename);

  /// Default destructor.
  ~World();

  /// Compose a new simulation from an external habitat and new populations.
  /**
   * The new simulation unit will be owned by \ref World and released as soon
   * as the \ref Habitat instance is marked as to be deleted from extern.
   * \param habitat Pointer to an instance of a class that implements \ref
   * Habitat, given externally by the vegetation model. The World object will
   * release it after it has been marked as dead (\ref Habitat::kill())
   * externally.
   * \throw std::invalid_argument If `habitat` is NULL.
   */
  void create_simulation_unit(std::shared_ptr<Habitat> habitat);

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
   * \throw std::logic_error If `date` has not been correctly incremented by one
   * day since the last call.
   */
  void simulate_day(const Date& date, const bool do_herbivores);

 private:
  /// Get the immutable list of herbivore functional types.
  const HftList& get_hfts();

  /// All simulation instructions from the TOML instruction file.
  /**
   * There should be only one immutable list of HFTs and one set of parameters
   * in the whole simulation. We implement it as a `struct` so both
   * `hftlist` and `params` can be set at once in the constructor member
   * initialization.
   *
   * Use \ref get_params() and \ref get_hfts() to access content.
   */
  struct InsfileContent {
    /// Global, immutable list of herbivore functional types.
    const std::shared_ptr<const HftList> hftlist;
    /// Global, immutable set of simulation parameters.
    const std::shared_ptr<const Parameters> params;
  } insfile;

  /// Number of days since extinct populations were re-established.
  int days_since_last_establishment;

  /// Collects output data per time interval and aggregation unit.
  std::unique_ptr<Output::Aggregator> output_aggregator;

  /// Output writer as selected by \ref Parameters::output_format.
  std::unique_ptr<Output::WriterInterface> output_writer;

  /// Helper function to initialize World::InsfileContent object.
  static InsfileContent read_instruction_file(const std::string& filename);

  /// List of all the simulation units in the world.
  /**
   * All objects are owned by \ref World.
   */
  std::list<SimulationUnit> sim_units;

  /// Helper class to construct various elements of the megafauna world.
  const std::unique_ptr<WorldConstructor> world_constructor;
};
}  // namespace Fauna
#endif  // FAUNA_WORLD_H
