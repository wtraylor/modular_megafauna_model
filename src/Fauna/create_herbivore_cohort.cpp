// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Construct new herbivore cohort objects.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "create_herbivore_cohort.h"

#include "parameters.h"

using namespace Fauna;

HerbivoreCohort CreateHerbivoreCohort::operator()(const double ind_per_km2,
                                                  const int age_years,
                                                  Sex sex) const {
  if (ind_per_km2 <= 0.0)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreCohort::operator()() "
        "ind_per_km2 <= 0.0");
  if (age_years < 0)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreCohort::operator()() "
        "age_years < 0");

  const int age_days = age_years * 365;
  if (age_days == 0)
    // Call birth constructor
    return HerbivoreCohort(hft, sex, ind_per_km2,
                           get_params().forage_gross_energy);
  else
    // Call establishment constructor
    return HerbivoreCohort(age_days, get_body_condition(age_days), hft, sex,
                           ind_per_km2, get_params().forage_gross_energy);
}
