// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Different models to calculate energy expenditures of herbivores.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "expenditure_components.h"
using namespace Fauna;

double Fauna::get_thermoregulatory_expenditure(const double thermoneutral_rate,
                                               const double conductance,
                                               const double core_temp,
                                               const double ambient_temp) {
  if (thermoneutral_rate < 0.0)
    throw std::invalid_argument(
        "Fauna::get_thermoregulatory_expenditure() "
        "Parameter `thermoneutral_rate` is negative.");
  if (conductance <= 0.0)
    throw std::invalid_argument(
        "Fauna::get_thermoregulatory_expenditure() "
        "Parameter `conductance` is negative or zero.");
  if (core_temp < 0.0)
    throw std::invalid_argument(
        "Fauna::get_thermoregulatory_expenditure() "
        "Parameter `core_temp` is negative.");

  // Conductance as MJ/ind/day/°C
  const double cond_MJ = watts_to_MJ_per_day(conductance);

  const double critical_temp = core_temp - thermoneutral_rate / cond_MJ;
  const double heat_loss = cond_MJ * std::max(critical_temp - ambient_temp,
      0.0);
  assert(heat_loss >= 0.0);
  return heat_loss;
}
