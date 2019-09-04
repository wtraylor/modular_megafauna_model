/**
 * \file
 * \brief Helper function for unit tests to check if population lists match.
 * \copyright ...
 * \date 2019
 */
#ifndef TESTS_POPULATION_LISTS_MATCH_H
#define TESTS_POPULATION_LISTS_MATCH_H

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

#endif  // TESTS_POPULATION_LISTS_MATCH_H
