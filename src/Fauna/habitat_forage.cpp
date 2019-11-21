/**
 * \file
 * \brief Forage in a habitat.
 * \copyright ...
 * \date 2019
 */
#include "habitat_forage.h"
#include "forage_values.h"

using namespace Fauna;

Digestibility HabitatForage::get_digestibility() const {
  Digestibility result;
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    result.set(*ft, operator[](*ft).get_digestibility());
  }
  return result;
}

ForageMass HabitatForage::get_mass() const {
  ForageMass result;
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    result.set(*ft, operator[](*ft).get_mass());
  }
  return result;
}

ForageFraction HabitatForage::get_nitrogen_content() {
  ForageFraction n_content;
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    const ForageBase& f = operator[](*ft);
    if (f.get_mass() != 0.0)
      n_content.set(*ft, f.get_nitrogen_mass() / f.get_mass());
  }
  return n_content;
}

ForageBase HabitatForage::get_total() const {
  // Return object
  ForageBase result;

  const ForageMass mass = get_mass();

  result.set_mass(mass.sum());

  // build weighted average only if total mass is greater zero
  if (result.get_mass() > 0.0) {
    // Create weighted average

    double dig_sum_weight = 0.0;
    // loop through each forage type
    for (const auto ft : FORAGE_TYPES) {
      dig_sum_weight += mass[ft] * operator[](ft).get_digestibility();
    }
    result.set_digestibility(dig_sum_weight / mass.sum());
  } else
    result.set_digestibility(0.0);
  return result;
}

HabitatForage& HabitatForage::merge(const HabitatForage& other,
                                    const double this_weight,
                                    const double other_weight) {
  if (this == &other) return *this;

  grass.merge(other.grass, this_weight, other_weight);
  // ADD NEW FORAGE TYPES HERE
  return *this;
}

void HabitatForage::set_nitrogen_content(const ForageFraction& n_content) {
  // loop through each forage type
  for (const auto ft : FORAGE_TYPES) {
    const double dry_matter = (*this)[ft].get_mass();
    if (n_content[ft] == 1.0)
      throw std::invalid_argument(
          "Fauna::HabitatForage::set_nitrogen_content() "
          "Nitrogen content for one forage type is 100%. That is not allowed.");
    (*this)[ft].set_nitrogen_mass(dry_matter * n_content[ft]);
  }
}
