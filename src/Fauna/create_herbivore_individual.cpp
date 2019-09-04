/**
 * \file
 * \brief Construct new herbivore individual objects.
 * \copyright ...
 * \date 2019
 */
#include "create_herbivore_individual.h"
#include "herbivore_individual.h"
#include "parameters.h"

using namespace Fauna;

double CreateHerbivoreIndividual::get_area_km2() const {
  return get_params().habitat_area_km2;
}

HerbivoreIndividual CreateHerbivoreIndividual::operator()(const int age_days,
                                                          Sex sex) const {
  if (age_days < 0)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreIndividual::operator()() "
        "age_days < 0");

  assert(get_area_km2() > 0.0);
  if (age_days == 0)
    // Call birth constructor
    return HerbivoreIndividual(&get_hft(), sex, get_area_km2());
  else
    // Call establishment constructor
    return HerbivoreIndividual(age_days, get_body_condition(age_days),
                               &get_hft(), sex, get_area_km2());
}

