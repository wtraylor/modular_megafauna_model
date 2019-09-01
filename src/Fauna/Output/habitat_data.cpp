///////////////////////////////////////////////////////////////////
/// \file
/// \brief Habitat output data.
/// \author Wolfgang Traylor, Senckenberg BiK-F
/// \date June 2019
/// \see \ref sec_output
////////////////////////////////////////////////////////////////////
#include "habitat_data.h"
#include "average.h"

using namespace Fauna::Output;

HabitatData& HabitatData::merge(const HabitatData& other,
                                const double this_weight,
                                const double other_weight) {
  if (!(this_weight >= 0.0))
    throw std::invalid_argument(
        "Fauna::Output::HabitatData::merge() "
        "Parameter `this_weight` is not a >=0.0");
  if (!(other_weight >= 0.0))
    throw std::invalid_argument(
        "Fauna::Output::HabitatData::merge() "
        "Parameter `other_weight` is not a >=0.0");
  if (this_weight == 0.0 && other_weight == 0.0)
    throw std::invalid_argument(
        "Fauna::Output::HabitatData::merge() "
        "Both objects have zero weight");

  // If objects are identical, do nothing.
  if (&other == this) return *this;

  // Don’t do any calculations if one partner is weighed with zero.
  if (other_weight == 0.0) return *this;
  if (this_weight == 0.0) {
    *this = other;  // copy all values
    return *this;
  }

  // Build average for each variable:
  eaten_forage.merge(other.eaten_forage, this_weight, other_weight);
  available_forage.merge(other.available_forage, this_weight, other_weight);

  environment.snow_depth =
      average(this->environment.snow_depth, other.environment.snow_depth,
              this_weight, other_weight);

  // ADD NEW VARIABLES HERE

  return *this;
}
