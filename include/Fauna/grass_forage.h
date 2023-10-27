// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief The forage class for grass.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_GRASS_FORAGE_H
#define FAUNA_GRASS_FORAGE_H

#include <cassert>

#include "Fauna/forage_base.h"

namespace Fauna {
/// Grass forage in a habitat.
class GrassForage : public ForageBase {
 private:
  double fpc;

  /// Maximum imaginable real-world sward density [kgDM/km²].
  static constexpr double MAX_SWARD_DENSITY = 2e7;  // 20 kgDM/m²
 public:
  /// Constructor with zero values
  GrassForage() : ForageBase(), fpc(0.0) {}

  /// Dry matter forage in the area covered by grass [kgDM/km²].
  /**
   * Note that this is always greater or equal than \ref get_mass()
   * \throw std::logic_error If the result is greater than
   * \ref MAX_SWARD_DENSITY. In this case, the \ref fpc value set by the
   * vegetation model is probably unreasonably low.
   */
  double get_sward_density() const {
    if (get_fpc() == 0) return 0.0;
    const double sd = get_mass() / get_fpc();
    assert(sd >= 0.0 && sd >= get_mass());
    // Only throw an error about preposterously high sward density if there is
    // actually substantial grass in the habitat. Otherwise there might be
    // spurious erros if both FPC and total grass density are extremely low due
    // to model artifacts.
    // The value of 50 gDM/m² (= 5e4 kgDM/km²) is chosen arbitrarily.
    if (sd > MAX_SWARD_DENSITY && get_mass() > 5e4) {
      throw std::logic_error(
          "Fauna::GrassForage::get_sward_density() "
          "The grass sward density is unreasonably high (" +
          std::to_string(sd / 1e4) +
          " gDM/m²). "
          "This might be because the vegetation model gave an unrealistically "
          "low FPC value (fraction of habitat covered by grass).\n"
          "\tFPC = " +
          std::to_string(get_fpc()) +
          "\n"
          "\twhole-habitat grass density = " +
          std::to_string(get_mass() / 1e4) +
          " gDM/m²\n"
          "Please check the vegetation model. Consider hard-setting the FPC "
          "value to a constant (e.g. 0.8 in grassland) or a minimum (e.g. "
          "0.1).");
    }
    return sd;
  }

  /// Fraction of habitat covered by grass [fractional].
  /**
   * Grass-covered area as a fraction of the habitat. (FPC = “foliar projective
   * cover” might not be the ideal term, but you can interprect the acronym
   * also as “Fraction of Patch Covered [by grass]”.)
   * \throw std::logic_error If the values of mass and FPC
   * don’t fit together: If mass is zero, FPC must also be zero,
   * and if mass is non-zero, FPC must not be zero.
   * (Call \ref set_mass() and \ref set_fpc() with correct
   * values before calling \ref get_fpc().)
   */
  double get_fpc() const {
    if (get_mass() == 0.0 && fpc != 0.0)
      throw std::logic_error(
          "Fauna::GrassForage::get_fpc() "
          "Mass is zero, but FPC is not zero.");
    if (get_mass() != 0.0 && fpc == 0.0)
      throw std::logic_error(
          "Fauna::GrassForage::get_fpc() "
          "Mass is not zero, but FPC is zero.");
    return fpc;
  }

  /// \copydoc ForageValues::merge()
  GrassForage& merge(const GrassForage& other, const double this_weight,
                     const double other_weight);

  /** \copybrief get_fpc()
   * \note Call \ref get_mass() before this function to
   * avoid any illogical state (see exceptions in \ref get_fpc()).
   * \throw std::invalid_argument If not `0.0<=f<=1.0`.
   * \throw std::logic_error If `f==0.0 && get_mass()==0.0`
   * or `f==0.0 && get_mass()>0.0`.*/
  void set_fpc(const double f) {
    if (!(f >= 0.0 && f <= 1.0))
      throw std::invalid_argument(
          "Fauna::GrassForage::set_fpc() "
          "FPC out of valid range (0.0–1.0).");
    if (get_mass() == 0.0 && f > 0.0)
      throw std::logic_error(
          "Fauna::GrassForage::set_fpc() "
          "FPC must be zero if mass is zero.");
    if (get_mass() > 0.0 && f == 0.0)
      throw std::logic_error(
          "Fauna::GrassForage::set_fpc() "
          "FPC cannot be zero if there is grass mass.");
    fpc = f;
  }
};
}  // namespace Fauna
#endif  // FAUNA_GRASS_FORAGE_H
