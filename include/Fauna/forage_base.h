// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Base class for all forage types.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_FORAGE_BASE_H
#define FAUNA_FORAGE_BASE_H

#include <stdexcept>

namespace Fauna {

/// Base class for herbivore forage in a habitat.
class ForageBase {
 private:
  double digestibility, dry_matter_mass, nitrogen_mass;

 public:
  /// Constructor with zero values
  ForageBase() : digestibility(0.0), dry_matter_mass(0.0), nitrogen_mass(0.0) {}

  /// Fractional digestibility of the biomass for ruminants.
  /** Digestibility as measured *in-vitro* with rumen liquor. */
  double get_digestibility() const { return digestibility; }

  /// Dry matter forage biomass over the whole area [kgDM/km²].
  double get_mass() const { return dry_matter_mass; }

  /// Nitrogen mass per area [kgN/km²].
  double get_nitrogen_mass() const { return nitrogen_mass; }

  /** \copydoc get_digestibility()
   * \throw std::invalid_argument if not `0.0<=d<=1.0`*/
  void set_digestibility(const double d) {
    if (d < 0.0 || d > 1.0)
      throw std::invalid_argument(
          "Fauna::ForageBase::set_digestibility(): "
          "Digestibility out of range.");
    digestibility = d;
  }

  /** \copydoc get_mass()
   * \throw std::invalid_argument If `dm<0.0`
   * \throw std::logic_error If nitrogen mass > dry matter mass.
   * */
  void set_mass(const double dm);

  /** \copydoc get_nitrogen_mass()
   * \throw std::invalid_argument If `n_mass < 0.0`.
   * \throw std::logic_error If nitrogen mass > dry matter mass.
   */
  void set_nitrogen_mass(const double n_mass);

 protected:
  /** \copydoc ForageValues::merge() */
  ForageBase& merge_base(const ForageBase& other, const double this_weight,
                         const double other_weight);
};
}  // namespace Fauna
#endif  // FAUNA_FORAGE_BASE_H
