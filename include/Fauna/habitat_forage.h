// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Forage in a habitat.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_HABITAT_FORAGE_H
#define FAUNA_HABITAT_FORAGE_H

#include "Fauna/forage_types.h"
#include "Fauna/forage_values.h"
#include "Fauna/grass_forage.h"

namespace Fauna {

/// All values for large herbivore forage in a \ref Habitat.
class HabitatForage {
 public:
  /// Constructor with zero values
  HabitatForage() {}

  // TODO: Move this member variable to private and implement a getter.
  /// The grass forage in the habitat.
  GrassForage grass;

  // ADD NEW FORAGE TYPES (E.G. BROWSE) HERE.

  /// Get digestibility [fractional] for all edible forage types.
  /**
   * If mass is zero, digestibility is undefined.
   * \see \ref sec_energy_content
   */
  Digestibility get_digestibility() const;

  /// Get dry matter mass [kgDM/km²] for all edible forage types.
  ForageMass get_mass() const;

  /// Fraction of nitrogen in dry matter [kgN/kgDM].
  ForageFraction get_nitrogen_content();

  /// Total forage in the habitat.
  /** Digestibility is weighted average, forage mass is sum.
   * If mass is zero, digestibility is also zero.*/
  ForageBase get_total() const;

  /// \copydoc ForageValues::merge()
  HabitatForage& merge(const HabitatForage& other, const double this_weight,
                       const double other_weight);

  /// Reference to forage object by forage type.
  /**
   * \param ft Forage type.
   * \return Polymorphic reference to forage class object.
   * \throw std::logic_error if `ft` is not implemented. */
  const ForageBase& operator[](const ForageType ft) const {
    switch (ft) {
      case ForageType::Grass:
        return grass;
        // ADD NEW FORAGE TYPES HERE.
      case ForageType::Inedible:
        return inedible;
      default:
        throw std::logic_error((std::string)
											 "Fauna::HabitatForage::operator[]()const "
											 "Forage type \"" +
											 get_forage_type_name(ft) + "\" "
											 "is not implemented.");
    }
  }

  /** \copydoc operator[]() */
  ForageBase& operator[](const ForageType ft) {
    switch (ft) {
      case ForageType::Grass:
        return grass;
        // ADD NEW FORAGE TYPES HERE.
      case ForageType::Inedible:
        return inedible;
      default:
        throw std::logic_error(
            "Fauna::HabitatForage::operator[]() "
            "Forage type \"" +
            get_forage_type_name(ft) +
            "\" "
            "is not implemented.");
    }
  }

  /// Set the nitrogen content [kgN/kgDM] for all forage types.
  /** \throw std::invalid_argument If one value of `nitrogen_content`
   * equals 1.
   */
  void set_nitrogen_content(const ForageFraction& nitrogen_content);

 private:
  /// Inedible forage in the habitat.
  /** This is currently not used, but only here to return something
   * if \ref ForageType::Inedible is passed to the [] operator. */
  ForageBase inedible;
};
}  // namespace Fauna
#endif  // FAUNA_HABITAT_FORAGE_H
