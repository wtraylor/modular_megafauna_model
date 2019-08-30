//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simulation unit: a habitat + herbivore populations.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////
#ifndef SIMULATION_UNIT_H
#define SIMULATION_UNIT_H

#include <memory>

namespace Fauna {
// forward declaration
class Habitat;
class PopulationList;

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
  SimulationUnit(Habitat* habitat, PopulationList* populations);

  /// Default Destructor
  ~SimulationUnit();

  /// The habitat where the populations live.
  /** \throw std::logic_error If the private pointer is NULL. */
  Habitat& get_habitat();

  /// Get a readonly reference to the habitat.
  /** \throw std::logic_error If the private pointer is NULL. */
  const Habitat& get_habitat() const;

  /// The herbivores that live in the habitat.
  /** \throw std::logic_error If the private pointer is NULL. */
  PopulationList& get_populations();

  /// The read-only handle to all herbivores that live in the habitat.
  /** \throw std::logic_error If the private pointer is NULL. */
  const PopulationList& get_populations() const;

  /// Whether the flag for initial establishment has been set.
  bool is_initial_establishment_done() const {
    return initial_establishment_done;
  }

  /// Set the flag that initial establishment has been performed.
  void set_initial_establishment_done() { initial_establishment_done = true; }
 private:
  std::unique_ptr<Habitat> habitat;
  bool initial_establishment_done;
  std::unique_ptr<PopulationList> populations;
};

}  // namespace Fauna
#endif  // SIMULATION_UNIT_H
