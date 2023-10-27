// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Base class for all forage types.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "forage_base.h"

#include <cassert>

#include "average.h"

using namespace Fauna;

ForageBase& ForageBase::merge_base(const ForageBase& other,
                                   const double this_weight,
                                   const double other_weight) {
  if (this == &other) return *this;
  if (this->get_mass() > 0.0 || other.get_mass() > 0.0)
    set_digestibility(average(
        this->get_digestibility(), other.get_digestibility(),
        this_weight * this->get_mass(), other_weight * other.get_mass()));
  else
    set_digestibility(0.0);  // NA, so to say

  dry_matter_mass =
      average(this->get_mass(), other.get_mass(), this_weight, other_weight);
  nitrogen_mass = average(this->get_nitrogen_mass(), other.get_nitrogen_mass(),
                          this_weight, other_weight);
  assert(dry_matter_mass >= 0.0);
  assert(nitrogen_mass >= 0.0);
  assert(dry_matter_mass >= nitrogen_mass);
  return *this;
}

void ForageBase::set_mass(const double dm) {
  if (dm < 0.0)
    throw std::invalid_argument(
        "Fauna::ForageBase::set_mass(): "
        "Dry matter is smaller than zero.");
  if (get_nitrogen_mass() > dm)
    throw std::logic_error(
        "Fauna::ForageBase::set_mass() "
        "Dry matter is set to a value smaller than "
        "nitrogen mass. Decrease nitrogen mass first.");
  dry_matter_mass = dm;
}

void ForageBase::set_nitrogen_mass(const double n_mass) {
  if (n_mass < 0.0)
    throw std::invalid_argument(
        "Fauna::ForageBase::set_nitrogen_mass(): "
        "Nitrogen mass is smaller than zero.");
  if (n_mass > get_mass())
    throw std::logic_error(
        "Fauna::ForageBase::set_mass() "
        "Nitrogen mass is set to a value greater than "
        "the dry matter mass. Increase dry matter first.");
  nitrogen_mass = n_mass;
}
