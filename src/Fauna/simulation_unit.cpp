//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simulation unit: a habitat + herbivore populations.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////

#include "simulation_unit.h"
#include "habitat.h"
#include "population.h"

using namespace Fauna;

SimulationUnit::SimulationUnit(Habitat* habitat, HftPopulationsMap* populations)
    :  // move ownership to private unique_ptr objects
      habitat(habitat),
      populations(populations),
      initial_establishment_done(false) {
  if (habitat == NULL)
    throw std::invalid_argument(
        "Fauna::SimulationUnit::SimulationUnit() Pointer to habitat is NULL.");
  if (populations == NULL)
    throw std::invalid_argument(
        "Fauna::SimulationUnit::SimulationUnit() "
        "Pointer to populations is NULL.");
}

// The destructor needs to be implemented here in the source file and not
// inline in the header file. The reason is that std::unique_ptr needs to call
// the destructor of Fauna::Habitat when it is itself released. But the
// destructor of Fauna::Habitat is incomplete at compile time.
SimulationUnit::~SimulationUnit() = default;

Habitat& SimulationUnit::get_habitat() {
  if (habitat.get() == NULL)
    throw std::logic_error(
        "Fauna::SimulationUnit::get_habitat() "
        "The unique pointer to habitat is NULL. "
        "The SimulationUnit object lost ownership "
        "of the Habitat object.");
  return *habitat;
};

const Habitat& SimulationUnit::get_habitat() const {
  if (habitat.get() == NULL)
    throw std::logic_error(
        "Fauna::SimulationUnit::get_habitat() "
        "The unique pointer to habitat is NULL. "
        "The SimulationUnit object lost ownership "
        "of the Habitat object.");
  return *habitat;
};
