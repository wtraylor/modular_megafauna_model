// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Herbivore energy model with fat reserves and anabolism/catabolism.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "fatmass_energy_budget.h"

#include <cassert>
#include <stdexcept>

using namespace Fauna;

FatmassEnergyBudget::FatmassEnergyBudget(const double initial_fatmass,
                                         const double maximum_fatmass,
                                         const double anabolism_coefficient,
                                         const double catabolism_coefficient)
    : anabolism_coefficient(anabolism_coefficient),
      catabolism_coefficient(catabolism_coefficient),
      fatmass(initial_fatmass),
      max_fatmass(maximum_fatmass) {
  if (anabolism_coefficient <= 0.0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::FatmassEnergyBudget() "
        "anabolism_coefficient <= 0.0");
  if (catabolism_coefficient <= 0.0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::FatmassEnergyBudget() "
        "catabolism_coefficient <= 0.0");
  if (initial_fatmass < 0.0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::FatmassEnergyBudget() "
        "initial_fatmass < 0.0");
  // Allow for floating point imprecision by comparing to 10 g/ind.
  if ((int)(initial_fatmass * 100) > (int)(maximum_fatmass * 100))
    throw std::logic_error(
        "Fauna::FatmassEnergyBudget::FatmassEnergyBudget() "
        "initial_fatmass > maximum_fatmass");
  if (maximum_fatmass <= 0.0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::FatmassEnergyBudget() "
        "maximum_fatmass <= 0.0");
}

void FatmassEnergyBudget::add_energy_needs(const double energy) {
  if (energy < 0.0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::add_energy_needs() "
        "energy < 0.0");
  energy_needs += energy;
}

void FatmassEnergyBudget::catabolize_fat() {
  assert(energy_needs >= 0.0);
  assert(fatmass >= 0.0);
  if (energy_needs == 0.0) return;

  // fat mass [kg] to burn in order to meet energy needs
  const double burned_fatmass = energy_needs / catabolism_coefficient;

  /// Fat mass never drops below zero.
  fatmass = std::max(0.0, fatmass - burned_fatmass);
  assert(fatmass >= 0.0);

  energy_needs = 0.0;
}

void FatmassEnergyBudget::force_body_condition(const double body_condition) {
  if (body_condition > 1.0 || body_condition < 0.0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::force_body_condition() "
        "Parameter `body_condition` out of bounds.");
  fatmass = get_max_fatmass() * body_condition;
}

double FatmassEnergyBudget::get_max_anabolism_per_day() const {
  assert(max_fatmass_gain >= 0.0);

  // Fat mass increment [kg/ind/day] without limit.
  double increment = max_fatmass - fatmass;

  // If there is a limit set, decrease `increment`.
  if (max_fatmass_gain != 0.0)
    increment = std::min(max_fatmass_gain, increment);

  return increment * anabolism_coefficient;
}

void FatmassEnergyBudget::merge(const FatmassEnergyBudget& other,
                                const double this_weight,
                                const double other_weight) {
  energy_needs =
      (energy_needs * this_weight + other.energy_needs * other_weight) /
      (this_weight + other_weight);
  fatmass = (fatmass * this_weight + other.fatmass * other_weight) /
            (this_weight + other_weight);
  max_fatmass = (max_fatmass * this_weight + other.max_fatmass * other_weight) /
                (this_weight + other_weight);
}

void FatmassEnergyBudget::metabolize_energy(double energy) {
  if (energy < 0.0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::metabolize_energy() "
        "energy < 0.0");
  assert(energy_needs >= 0.0);
  assert(fatmass >= 0.0);

  if (energy <= energy_needs) {
    energy_needs -= energy;  // just meet immediate energy needs
  } else {
    // meet immediate energy needs
    energy -= energy_needs;
    energy_needs = 0.0;

    // store surplus as fat (anabolism) [kg/ind]
    const double fatmass_gain = energy / anabolism_coefficient;

    // Check if fat mass gain is too high, but allow for some rounding
    // errors.
    if (fatmass + fatmass_gain > 1.001 * max_fatmass)
      throw std::logic_error(
          "Fauna::FatmassEnergyBudget::metabolize_energy() "
          "Received energy exceeds maximum allowed fat anabolism.");

    // increase fat reserves
    // If fat mass gain exceeds maximum fat mass (rounding errors), only
    // increase up to the maximum.
    fatmass = std::min(fatmass + fatmass_gain, max_fatmass);
  }
}

void FatmassEnergyBudget::set_max_fatmass(const double _max_fatmass,
                                          const double max_gain) {
  if (_max_fatmass < fatmass)
    throw std::logic_error(
        "Fauna::FatmassEnergyBudget::set_max_fatmass() "
        "Maximum fat mass is lower than current fat mass.");
  if (_max_fatmass <= 0.0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::set_max_fatmass() "
        "Received maximum fat mass smaller than zero");
  if (max_gain < 0)
    throw std::invalid_argument(
        "Fauna::FatmassEnergyBudget::set_max_fatmass() "
        "Maximum fat mass gain must not be negative.");
  max_fatmass = _max_fatmass;
  max_fatmass_gain = max_gain;
}
