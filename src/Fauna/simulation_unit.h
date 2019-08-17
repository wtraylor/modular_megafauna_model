//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simulation unit: a habitat + herbivore populations.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////
#ifndef SIMULATION_UNIT_H
#define SIMULATION_UNIT_H

#include <memory>
#include "combined_data.h"

namespace Fauna {
// forward declaration
class Habitat;
class HftPopulationsMap;

/// A habitat with the herbivores that live in it.
/** \see \ref sec_designoverview */
class SimulationUnit {
 public:
  /// Constructor
  /**
   * \param habitat Pointer to the habitat object. SimulationUnit will take
   * over exclusive ownership of the pointer.
   * \param populations Pointer to the habitat object. SimulationUnit will take
   * over exclusive ownership of the pointer.
   * \throw std::invalid_argument If one of the parameters is NULL.
   */
  SimulationUnit(Habitat* habitat, HftPopulationsMap* populations);

  /// Default Destructor
  ~SimulationUnit();

  /// The habitat where the populations live.
  /** \throw std::logic_error If the private pointer is NULL. */
  Habitat& get_habitat();

  /// The herbivores that lives in the habitat.
  /** \throw std::logic_error If the private pointer is NULL. */
  HftPopulationsMap& get_populations() {
    if (populations.get() == NULL)
      throw std::logic_error(
          "Fauna::SimulationUnit::get_populations() "
          "The unique pointer to populations is NULL. "
          "The SimulationUnit object lost ownership "
          "of the HftPopulationsMap object.");
    return *populations;
  }

  /// Whether the flag for initial establishment has been set.
  bool is_initial_establishment_done() const {
    return initial_establishment_done;
  }

  /// Set the flag that initial establishment has been performed.
  void set_initial_establishment_done() { initial_establishment_done = true; }

  /// @{ \brief Get temporally aggregated habitat and herbivore output.
  Output::CombinedData& get_output() { return current_output; }
  const Output::CombinedData& get_output() const { return current_output; }
  /**@}*/  // Output Functions
 private:
  Output::CombinedData current_output;
  std::unique_ptr<Habitat> habitat;
  bool initial_establishment_done;
  std::unique_ptr<HftPopulationsMap> populations;
};

}  // namespace Fauna
#endif  // SIMULATION_UNIT_H
