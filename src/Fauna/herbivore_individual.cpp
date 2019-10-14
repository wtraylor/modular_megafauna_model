/**
 * \file
 * \brief Implementation of \ref Fauna::HerbivoreBase as individual animals.
 * \copyright ...
 * \date 2019
 */
#include "herbivore_individual.h"
#include "stochasticity.h"

using namespace Fauna;

HerbivoreIndividual::HerbivoreIndividual(const int age_days,
                                         const double body_condition,
                                         const Hft* hft, const Sex sex,
                                         const double area_km2)
    : HerbivoreBase(age_days, body_condition, hft, sex),
      area_km2(area_km2),
      dead(false) {
  if (area_km2 <= 0.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreIndividual::HerbivoreIndividual() "
        "area_km2 <=0.0");
}

HerbivoreIndividual::HerbivoreIndividual(const Hft* hft, const Sex sex,
                                         const double area_km2)
    : HerbivoreBase(hft, sex), area_km2(area_km2), dead(false) {
  if (area_km2 <= 0.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreIndividual::HerbivoreIndividual() "
        "area_km2 <=0.0");
}

HerbivoreIndividual::HerbivoreIndividual(const HerbivoreIndividual& other)
    : HerbivoreBase(other), area_km2(other.area_km2), dead(other.dead) {}

HerbivoreIndividual& HerbivoreIndividual::operator=(
    const HerbivoreIndividual& other) {
  HerbivoreBase::operator=(other);
  area_km2 = other.area_km2;
  dead = other.dead;
  return *this;
}

void HerbivoreIndividual::apply_mortality(const double mortality) {
  if (mortality < 0.0 || mortality > 1.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::apply_mortality() "
        "Parameter \"mortality\" out of range.");
  // Save some calculations for the simple cases of 0.0 and 1.0
  if (mortality == 0.0) return;
  if (mortality == 1.0) {
    dead = true;
    return;
  }
  // Death is a stochastic event
  const int seed = get_today();
  if (get_random_fraction(seed) < mortality) dead = true;
}

