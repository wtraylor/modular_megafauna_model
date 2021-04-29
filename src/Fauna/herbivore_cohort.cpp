// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Implementation of \ref Fauna::HerbivoreBase as annual cohorts.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "herbivore_cohort.h"
#include "fatmass_energy_budget.h"
#include "hft.h"

using namespace Fauna;

HerbivoreCohort::HerbivoreCohort(const int age_days,
                                 const double body_condition,
                                 std::shared_ptr<const Hft> hft, const Sex sex,
                                 const double ind_per_km2,
                                 const ForageEnergyContent& forage_gross_energy)
    : HerbivoreBase(age_days, body_condition, hft, sex, forage_gross_energy),
      ind_per_km2(ind_per_km2) {
  if (ind_per_km2 < 0.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::HerbivoreCohort() "
        "ind_per_km2 <0.0");
}

HerbivoreCohort::HerbivoreCohort(std::shared_ptr<const Hft> hft, const Sex sex,
                                 const double ind_per_km2,
                                 const ForageEnergyContent& forage_gross_energy)
    : HerbivoreBase(hft, sex, forage_gross_energy), ind_per_km2(ind_per_km2) {
  if (ind_per_km2 < 0.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::HerbivoreCohort() "
        "ind_per_km2 <0.0");
}

void HerbivoreCohort::apply_mortality(const double mortality) {
  if (mortality < 0.0 || mortality > 1.0)
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::apply_mortality() "
        "Parameter \"mortality\" out of range.");
  // change of individual density [ind/km²]
  const double ind_change = -mortality * get_ind_per_km2();
  // apply the change and make sure that the density does not
  // drop below zero because of precision artefacts
  ind_per_km2 = std::max(0.0, ind_per_km2 + ind_change);
  assert(ind_per_km2 >= 0.0);
}

bool HerbivoreCohort::is_dead() const { return get_ind_per_km2() <= 0.0; }

void HerbivoreCohort::merge(HerbivoreCohort& other) {
  if (!is_same_age(other))
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::merge() "
        "The other cohort is not the same age.");
  if (!constant_members_match(other))
    throw std::invalid_argument(
        "Fauna::HerbivoreCohort::merge() "
        "The constant member variables of the other cohort don’t all "
        "match the ones from this cohort.");

  // Merge energy budget
  this->get_energy_budget().merge(other.get_energy_budget(),
                                  this->get_ind_per_km2(),
                                  other.get_ind_per_km2());

  // sum up density
  this->ind_per_km2 += other.ind_per_km2;
  // Change density in other object
  other.ind_per_km2 = 0.0;
}
