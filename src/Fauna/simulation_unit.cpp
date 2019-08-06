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
