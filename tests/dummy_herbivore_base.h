// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Implementation of herbivore base class for unit tests.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef TESTS_DUMMY_HERBIVORE_BASE_H
#define TESTS_DUMMY_HERBIVORE_BASE_H
#include "herbivore_base.h"
#include "parameters.h"

namespace Fauna {
/// Dummy class to test \ref HerbivoreBase
class HerbivoreBaseDummy : public HerbivoreBase {
 public:
  virtual double get_ind_per_km2() const { return ind_per_km2; }

  virtual bool is_dead() const { return false; }
  virtual void kill() {}
  /// Establishment Constructor
  HerbivoreBaseDummy(const int age_days, const double body_condition,
                     std::shared_ptr<const Hft> hft, const Sex sex)
      : HerbivoreBase(age_days, body_condition, hft, sex,
                      Parameters().forage_gross_energy),
        ind_per_km2(1.0) {}

  /// Birth Constructor
  HerbivoreBaseDummy(std::shared_ptr<const Hft> hft, const Sex sex)
      : HerbivoreBase(hft, sex, Parameters().forage_gross_energy),
        ind_per_km2(1.0) {}

  HerbivoreBaseDummy(const HerbivoreBaseDummy& other)
      : HerbivoreBase(other), ind_per_km2(other.ind_per_km2) {}
  HerbivoreBaseDummy& operator=(const HerbivoreBaseDummy& rhs) {
    HerbivoreBase::operator=(rhs);
    ind_per_km2 = rhs.ind_per_km2;
    return *this;
  }

 protected:
  virtual void apply_mortality(const double mortality) {
    ind_per_km2 *= mortality;
  }

 private:
  double ind_per_km2;
};
}  // namespace Fauna

#endif  // TESTS_DUMMY_HERBIVORE_BASE_H
