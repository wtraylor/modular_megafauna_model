#ifndef POPULATION_LISTS_MATCH_H
#define POPULATION_LISTS_MATCH_H

#include "herbivore_vector.h"
#include "population_interface.h"

using namespace Fauna;

/// \brief Check if the lengths of the modifiable and the
/// read-only population vectors match.
inline bool population_lists_match(PopulationInterface& pop) {
  // FIRST the read-only -> no chance for the population
  // object to change the list.
  ConstHerbivoreVector readonly = ((const PopulationInterface&)pop).get_list();
  HerbivoreVector modifiable = pop.get_list();
  return modifiable.size() == readonly.size();
}

#endif  // POPULATION_LISTS_MATCH_H
