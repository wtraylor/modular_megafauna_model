// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Basic classes encapsulating forage amounts & fractions.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "forage_values.h"

using namespace Fauna;

ForageValues<ForageValueTag::PositiveAndZero>
Fauna::foragefractions_to_foragevalues(const ForageFraction& fractions) {
  ForageValues<ForageValueTag::PositiveAndZero> result;
  for (const auto ft : FORAGE_TYPES) result.set(ft, fractions[ft]);
  return result;
}

ForageFraction Fauna::foragevalues_to_foragefractions(
    const ForageValues<ForageValueTag::PositiveAndZero> values,
    const double tolerance) {
  if (tolerance < 0.0)
    throw std::invalid_argument(
        "Fauna::foragevalues_to_foragefractions() "
        "Parameter `tolerance` is negative. "
        "(value == " +
        std::to_string(tolerance) + ")");

  ForageFraction result;
  for (const auto ft : FORAGE_TYPES) {
    double v = values[ft];
    if (v > 1.0) {
      if (v <= 1.0 + tolerance)
        v = 1.0;
      else
        throw std::invalid_argument(
            "Fauna::foragevalues_to_foragefractions() "
            "One forage value exceeds 1.0 and cannot be converted to "
            "a fraction:"
            "values[" +
            get_forage_type_name(ft) + "] == " + std::to_string(v));
    }
    result.set(ft, v);
  }
  return result;
}

ForageFraction Fauna::convert_mj_to_kg_proportionally(
    const ForageEnergyContent& mj_per_kg,
    const ForageFraction& mj_proportions) {
  ForageValues<ForageValueTag::PositiveAndZero> kg_proportions =
      foragefractions_to_foragevalues(mj_proportions)
          .divide_safely(mj_per_kg, 0.0);

  const double kg_prop_sum = kg_proportions.sum();
  const double mj_prop_sum = mj_proportions.sum();

  if (kg_prop_sum > 0.0) {
    kg_proportions = kg_proportions * (mj_prop_sum / kg_prop_sum);

    return foragevalues_to_foragefractions(kg_proportions, 0.01);
  } else
    return ForageFraction(0.0);
}
