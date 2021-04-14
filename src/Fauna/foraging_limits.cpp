// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Different models how to restrict daily forage intake of herbivores.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "foraging_limits.h"
#include <cmath>

using namespace Fauna;

double Fauna::get_digestive_limit_illius_gordon_1992(
    const double bodymass_adult, const double bodymass,
    const double digestibility, const std::array<double, 3>& ijk) {
  if (bodymass_adult <= 0.0)
    throw std::invalid_argument(
        "Fauna::get_digestive_limit_illius_gordon_1992() "
        "Parameter `bodymass_adult` <= zero.");
  if (bodymass <= 0.0)
    throw std::invalid_argument(
        "Fauna::get_digestive_limit_illius_gordon_1992() "
        "Parameter `bodymass` <= zero.");
  if (bodymass > bodymass_adult)
    throw std::invalid_argument(
        "Fauna::get_digestive_limit_illius_gordon_1992() "
        "bodymass > bodymass_adult");

  const double i = ijk[0];
  const double j = ijk[1];
  const double k = ijk[2];

  // rename variables to match formula
  const double d = digestibility;
  const double& M_ad = bodymass_adult;  // [kg]
  const double& M = bodymass;           // [kg]
  const double u_g = pow(M / M_ad, .75);

  // Only for the supported forage types, the result is calculated.
  if (digestibility > 0.0)
    return i * exp(j * d) * pow(M_ad, k * exp(d) + .73) * u_g;
  else
    return 0.0;  // zero digestibility -> zero energy
}

//============================================================
// HalfMaxIntake
//============================================================

HalfMaxIntake::HalfMaxIntake(const double half_max_density,
                             const double max_intake)
    : half_max_density(half_max_density), max_intake(max_intake) {
  if (!(half_max_density > 0.0))
    throw std::invalid_argument(
        "Fauna::HalfMaxIntake::HalfMaxIntake() "
        "Parameter `half_max_density` is not a positive number.");
  if (!(max_intake > 0.0))
    throw std::invalid_argument(
        "Fauna::HalfMaxIntake::HalfMaxIntake() "
        "Parameter `max_intake` is not a positive number.");
}

double HalfMaxIntake::get_intake_rate(const double density) const {
  if (!(density >= 0.0))
    throw std::invalid_argument(
        "Fauna::HalfMaxIntake::get_intake_rate() "
        "Parameter `density` must be zero or a positive number.");
  return max_intake * density / (half_max_density + density);
}
