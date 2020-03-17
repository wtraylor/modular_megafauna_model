/**
 * \file
 * \brief Abstract class for any population of herbivores.
 * \copyright ...
 * \date 2019
 */
#include "population_interface.h"
#include "herbivore_interface.h"

using namespace Fauna;

const double PopulationInterface::get_kg_per_km2() const {
  double sum = 0.0;
  const ConstHerbivoreVector vec = get_list();
  for (ConstHerbivoreVector::const_iterator itr = vec.begin(); itr != vec.end();
       itr++)
    sum += (*itr)->get_kg_per_km2();
  return sum;
}

const double PopulationInterface::get_ind_per_km2() const {
  double sum = 0.0;
  const ConstHerbivoreVector vec = get_list();
  for (ConstHerbivoreVector::const_iterator itr = vec.begin(); itr != vec.end();
       itr++)
    sum += (*itr)->get_ind_per_km2();
  return sum;
}

void PopulationInterface::kill_all() {
  HerbivoreVector vec = get_list();
  for (HerbivoreVector::const_iterator itr = vec.begin(); itr != vec.end();
       itr++)
    (*itr)->kill();
}

