//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simulation unit: a habitat + herbivore populations.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////

#include "simulation_unit.h"
#include "config.h"
#include "habitat.h"     // for Habitat and Population
#include "population.h"  // for HftPopulationsMap

using namespace Fauna;

//============================================================
// SimulationUnit
//============================================================

SimulationUnit::SimulationUnit(std::auto_ptr<Habitat> _habitat,
                               std::auto_ptr<HftPopulationsMap> _populations)
    :  // move ownership to private auto_ptr objects
      habitat(_habitat),
      populations(_populations),
      initial_establishment_done(false) {
  if (habitat.get() == NULL)
    throw std::invalid_argument(
        "Fauna::SimulationUnit::SimulationUnit() "
        "Pointer to habitat is NULL.");
  if (populations.get() == NULL)
    throw std::invalid_argument(
        "Fauna::SimulationUnit::SimulationUnit() "
        "Pointer to populations is NULL.");
}
