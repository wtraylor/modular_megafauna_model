//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Reproduction of herbivores.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
//////////////////////////////////////////////////////////////////////////

#include "reproduction_models.h"
#include <cassert>
#include <cmath>
#include <stdexcept>

using namespace Fauna;

// ##################################################################
// ######################### ReprIlliusOconnor2000 #################
// ##################################################################

ReprIlliusOconnor2000::ReprIlliusOconnor2000(BreedingSeason breeding_season,
                                             const double max_annual_increase)
    : max_annual_increase(max_annual_increase),
      breeding_season(breeding_season) {
  if (max_annual_increase < 0.0)
    throw std::invalid_argument(
        "Fauna::ReprIlliusOconnor2000::ReprIlliusOconnor2000() "
        "max_annual_increase below zero.");
}

double ReprIlliusOconnor2000::get_offspring_density(
    const int day_of_year, const double body_condition) const {
  if (day_of_year < 0 || day_of_year >= 365)
    throw std::invalid_argument(
        "Fauna::ReprIlliusOconnor2000::get_offspring_density() "
        "day_of_year is out of range.");
  if (body_condition < 0.0 || body_condition > 1.0)
    throw std::invalid_argument(
        "Fauna::ReprIlliusOconnor2000::get_offspring_density() "
        "body_condition is out of range.");

  // No reproduction if we are not in season.
  if (!breeding_season.is_in_season(day_of_year)) return 0.0;

  // Yes, we are in breeding season and just apply the formula.
  static const double b = 15.0;
  static const double c = 0.3;
  const double k = max_annual_increase;

  // see doxygen documentation of the class for formula
  // explanation

  // annual rate
  const double annual = (k / (1.0 + exp(-b * (body_condition - c))));
  assert(annual <= max_annual_increase);
  assert(annual >= 0.0);

  return breeding_season.annual_to_daily_rate(annual);
}

// ##################################################################
// ######################### ReproductionConstMax ###################
// ##################################################################

ReproductionConstMax::ReproductionConstMax(BreedingSeason breeding_season,
                                           const double annual_increase)
    : breeding_season(breeding_season), annual_increase(annual_increase) {
  if (annual_increase < 0.0)
    throw std::invalid_argument(
        "Fauna::ReproductionConstMax::ReproductionConstMax() "
        "Parameter `annual_increase` is below zero.");
}

double ReproductionConstMax::get_offspring_density(const int day) const {
  if (breeding_season.is_in_season(day))
    return breeding_season.annual_to_daily_rate(annual_increase);
  else
    return 0.0;
}

// ##################################################################
// ######################### ReproductionLinear #####################
// ##################################################################

ReproductionLinear::ReproductionLinear(BreedingSeason breeding_season,
                                       const double max_annual_increase)
    : max_annual_increase(max_annual_increase),
      breeding_season(breeding_season) {
  if (max_annual_increase < 0.0)
    throw std::invalid_argument(
        "Fauna::ReproductionLinear::ReproductionLinear() "
        "max_annual_increase below zero.");
}

double ReproductionLinear::get_offspring_density(
    const int day_of_year, const double body_condition) const {
  if (day_of_year < 0 || day_of_year >= 365)
    throw std::invalid_argument(
        "Fauna::ReproductionLinear::get_offspring_density() "
        "day_of_year is out of range.");
  if (body_condition < 0.0 || body_condition > 1.0)
    throw std::invalid_argument(
        "Fauna::ReproductionLinear::get_offspring_density() "
        "body_condition is out of range.");

  // No reproduction if we are not in season.
  if (!breeding_season.is_in_season(day_of_year)) return 0.0;

  // Yes, we are in breeding season and just apply the formula.
  const double annual = max_annual_increase * body_condition;

  // annual rate
  assert(annual <= max_annual_increase);
  assert(annual >= 0.0);

  return breeding_season.annual_to_daily_rate(annual);
}
