// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Output data of a habitat.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_OUTPUT_HABITAT_DATA_H
#define FAUNA_OUTPUT_HABITAT_DATA_H

#include "Fauna/environment.h"
#include "Fauna/habitat_forage.h"

namespace Fauna {
namespace Output {
/// Habitat output data for one time unit.
/**
 * \see The section on output classes in the MMM software design documentation.
 */
struct HabitatData {
  /// Available forage in the habitat.
  Fauna::HabitatForage available_forage;

  /// Forage mass [kgDM/km²/day] eaten by herbivores.
  /**
   * This is the sum of all forage that has been eaten by all herbivores
   * together.
   */
  Fauna::ForageMass eaten_forage;

  /// Abiotic conditions in the habitat.
  Fauna::HabitatEnvironment environment;

  //------------------------------------------------------------
  /** @{ \name Aggregation Functionality */
  /// Aggregate data of this object with another one.
  /**
   * This does no calculations if the partners are the same object, or
   * one of the weights is zero.
   * \param other The other object to be merged into this one.
   * \param this_weight Weight of this object in average building.
   * \param other_weight Weight of `other` in average building.
   * \return This object.
   * \see \ref Fauna::average(),
   *      \ref Fauna::HabitatForage::merge(),
   *      \ref Fauna::ForageValues::merge()
   * \throw std::invalid_argument If either weight is not a positive
   * number or if both are zero.
   */
  HabitatData& merge(const HabitatData& other, const double this_weight,
                     const double other_weight);

  /// Reset member variables to initial values.
  void reset() {
    available_forage = Fauna::HabitatForage();
    eaten_forage = 0.0;
    environment = Fauna::HabitatEnvironment();
  }
  /** @} */  // Aggregation Functionality
};

}  // namespace Output
}  // namespace Fauna

#endif  // FAUNA_OUTPUT_HABITAT_DATA_H
