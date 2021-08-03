// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Central management of all herbivores and habitats.
 * \copyright LGPL-3.0-or-later
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
struct Parameters;
class SimulationUnit;
class WorldConstructor;

// Repeat typedef from hft.h
typedef std::vector<std::shared_ptr<const Hft> > HftList;

namespace Output {
class Aggregator;
class WriterInterface;
}  // namespace Output

/// Mode of the whole simulation program: What is it started for?
enum class SimMode {
  /// Only check (“lint”) the instruction file, don’t perform simulations.
  /** \see linter.cpp */
  Lint,
  /// Default mode: We want to simulate herbivores.
  Simulate
};

/// Central class to construct and own megafauna habitats and populations.
class World {
 public:
  /// Constructor: Read parameters and HFTs from instruction file.
  /**
   * \param instruction_filename Path to the instruction file for the megafauna
   * model. It contains global settings and herbivore parameters.
   * \param mode Whether we are only checking the instruction file or running a
   * simulation. If set to \ref SimMode::Lint, no files will be created.
   *
   * \throw std::logic_error If a selected instruction file parameter is not
   * implemented.
   */
  World(const std::string instruction_filename,
        const SimMode mode = SimMode::Simulate);

  /// Constructor for unit tests
  /**
   * \warning This function may be changed in the future depending on the needs
   * of unit tests.
   * \param params Global simulation parameters that would normally be parsed
   * from the instruction file.
   * \param hftlist HFT parameters that would normally be parsed from the
   * instruction file.
   * \throw std::invalid_argument If either of the shared pointers is NULL.
   */
  World(const std::shared_ptr<const Parameters> params,
        const std::shared_ptr<const HftList> hftlist);

  /// Constructor: Create deactivated `World` object.
  /**
   * Even if the megafauna model should be completely deactivated, there might
   * be a need to create a "dummy" \ref World instance. An object created with
   * this constructor will not simulate anything.
   * \deprecated Rather don’t create a Fauna::World object at all instead of a
   * “dummy” one. For example, use `std::shared_ptr<Fauna::World>` and leave
   * the pointer `NULL` if you don’t need it.
   */
  World();

  /// Default destructor.
  ~World();

  /// Compose a new simulation from an external habitat and new populations.
  /**
   * The new simulation unit will be owned by \ref World and released as soon
   * as the \ref Habitat instance is marked as to be deleted from extern.
   *
   * If the \ref World class was constructed without an instruction file, this
   * function will do nothing, but will still throw an error if the parameter
   * is NULL.
   *
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
   * \throw std::logic_error If the member variable hasn’t been set (if the
   * \ref World class was created without an instruction file).
   */
  const Parameters& get_params() const;

  /// List of all the simulation units in the world.
  /**
   * This is read-only. Unit tests can use it to check if
   * \ref create_simulation_unit() works.
   */
  const std::list<SimulationUnit>& get_sim_units() const { return sim_units; }

  /// Whether this \ref World object is in \ref SimMode::Simulate mode.
  const bool is_activated() const { return mode == SimMode::Simulate; }

  /// Options passed to \ref simulate_day()
  struct SimDayOptions {
    /// Constructor
    SimDayOptions() {}

    /// Whether to perform herbivore simulations.
    /** If false, only the output data of the habitats are updated. */
    bool do_herbivores = true;

    /// Whether to reset the simulation so we start from the beginning.
    /**
     * Enable this if you want to move on to simulate another area for
     * instance. Then you restart from the beginning of your simulation time
     * and set the date accordingly. (\ref simulate_day() will not throw an
     * error about non-consecutive dates.)
     *
     * \note When you reset the date, it is advisable to clear the simulation
     * units (\ref SimulationUnit) in the \ref World object. Do that by marking
     * all habitats as “dead” (\ref Habitat::is_dead()). Simulation units with
     * dead habitats will automatically be cleared.
     */
    bool reset_date = false;
  };

  /// Iterate through all simulation units and perform simulation for this day.
  /**
   * This is the central access point to start the perform the herbivore
   * simulations.
   *
   * If a \ref Habitat instance is marked as dead (\ref Habitat::is_dead()),
   * the corresponding simulation unit will be released from memory.
   *
   * If the \ref World class was constructed without parameters, this function
   * will do nothing.
   *
   * \param date The current simulation day.
   * \param opts Options for today’s simulation.
   *
   * \throw std::invalid_argument If `date` has not been correctly incremented
   * by one day since the last call. However, no exception will be thrown if
   * \ref SimDayOptions::reset_date is `true`.
   *
   * \throw logic_error If \ref Parameters::one_hft_per_habitat, but for at
   * least one aggregation unit (\ref Habitat::get_aggregation_unit()) the
   * number of associated habitats is not an integer multiple of the number of
   * HFTs.
   *
   * \throw logic_error If the aggregation units
   * (\ref Habitat::get_aggregation_unit()) created with
   * \ref create_simulation_unit() do not all have the same number of habitats
   * each. It would bias the output if the means for each aggregation unit
   * would have different “sample counts.”
   */
  void simulate_day(const Date& date,
                    const SimDayOptions& opts = SimDayOptions());

  /// \copybrief simulate_day(const Date&, const SimDayOptions)
  /**
   * \param do_herbivores see \ref SimDayOptions::do_herbivores
   * \deprecated Use simulate_day(const Date&, const SimDayOptions)
   */
  void simulate_day(const Date& date, const bool do_herbivores) {
    SimDayOptions opts;
    opts.do_herbivores = do_herbivores;
    simulate_day(date, opts);
  }

 private:
  /// Get the number of habitats per aggregation unit.
  /**
   * \throw std::logic_error If the number of habitats differs between
   * aggregation units.
   * \see \ref Parameters::one_hft_per_habitat
   */
  int get_habitat_count_per_agg_unit() const;

  /// Get the immutable list of herbivore functional types.
  const HftList& get_hfts() const;

  /// Create \ref Output::WriterInterface implementation according to params.
  /**
   * \throw std::logic_error If \ref Parameters::output_format is not
   * implemented.
   * \see \ref output_writer
   */
  Output::WriterInterface* construct_output_writer() const;

  /// Whether this object is going to simulate or just lint an instruction file.
  const SimMode mode = SimMode::Simulate;

  /// Whether the habitat counts per aggregation unit have been checked.
  /**
   * By setting this variable, we don’t need to check on every call of
   * \ref simulate_day(), which might save some calculations. Instead, habitat
   * counts are only checked when they have changed through
   * \ref create_simulation_unit().
   * \see \ref get_habitat_count_per_agg_unit()
   */
  bool simulation_units_checked = false;

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

  /// The date from the last call to \ref simulate_day()
  /**
   * This is to check that the simulation days are coming in correctly from the
   * vegetation/host model.
   */
  std::unique_ptr<Fauna::Date> last_date;

  /// Collects output data per time interval and aggregation unit.
  const std::unique_ptr<Output::Aggregator> output_aggregator;

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
