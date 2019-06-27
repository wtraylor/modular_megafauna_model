//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Basic classes encapsulating forage amounts & fractions.
/// \author Wolfgang Traylor, Senckenberg BiK-F
/// \date June 2019
//////////////////////////////////////////////////////////////////////////
#include "forage_values.h"

using namespace Fauna;

ForageValues<POSITIVE_AND_ZERO> Fauna::foragefractions_to_foragevalues(
    const ForageFraction& fractions) {
  ForageValues<POSITIVE_AND_ZERO> result;
  for (ForageFraction::const_iterator i = fractions.begin();
       i != fractions.end(); i++)
    result.set(i->first, i->second);
  return result;
}

ForageFraction Fauna::foragevalues_to_foragefractions(
    const ForageValues<POSITIVE_AND_ZERO> values, const double tolerance) {
  if (tolerance < 0.0)
    throw std::invalid_argument(
        "Fauna::foragevalues_to_foragefractions() "
        "Parameter `tolerance` is negative.");

  ForageFraction result;
  for (ForageFraction::const_iterator i = values.begin(); i != values.end();
       i++) {
    double v = i->second;
    if (v > 1.0) {
      if (v <= 1.0 + tolerance)
        v = 1.0;
      else
        throw std::invalid_argument(
            "Fauna::foragevalues_to_foragefractions() "
            "One forage value exceeds 1.0 and cannot be converted to "
            "a fraction.");
    }

    result.set(i->first, v);
  }
  return result;
}

ForageFraction Fauna::convert_mj_to_kg_proportionally(
    const ForageEnergyContent& mj_per_kg,
    const ForageFraction& mj_proportions) {
  ForageValues<POSITIVE_AND_ZERO> kg_proportions =
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

