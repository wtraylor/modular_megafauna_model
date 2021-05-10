// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Herbivore energy model with fat reserves and anabolism/catabolism.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_FATMASS_ENERGY_BUDGET_H
#define FAUNA_FATMASS_ENERGY_BUDGET_H

namespace Fauna {

/// A herbivore’s energy budget with fat reserves
/**
 * Terminology:
 * - Anabolism  = build up fat mass
 * - Catabolism = burn fat mass
 * - Metabolism = burn food energy directly and use surplus for anabolism
 */
class FatmassEnergyBudget {
 public:
  /// Constructor
  /**
   * \param initial_fatmass Initial fat mass [kg/ind]
   * \param maximum_fatmass Maximum fat mass [kg/ind]
   * \param anabolism_coefficient Conversion factor from net forage energy to
   * fat mass [MJ/kg]. See Section \ref sec_energy_content for how this value
   * can be derived.
   * \param catabolism_coefficient Conversion factor from fat mass to net
   * energy [MJ/kg]. This is basically gross (combustion) energy of fat mass
   * multiplied by conversion efficiency.
   * \throw std::invalid_argument If one parameter is <=0.0
   * \throw std::logic_error `initial_fatmass > maximum_fatmass`
   */
  FatmassEnergyBudget(const double initial_fatmass,
                      const double maximum_fatmass,
                      const double anabolism_coefficient,
                      const double catabolism_coefficient);

  /// Increase energy needs
  /** \param energy Additional energy needs [MJ/ind]
   * \throw std::invalid_argument if `energy<0.0` */
  void add_energy_needs(const double energy);

  /// Burn fat reserves and reduce unmet energy needs.
  void catabolize_fat();

  /// Set the proportional fat reserves.
  /**
   * \param body_condition Current fat mass divided by potential
   * maximum fat mass.
   * \throw std::invalid_argument If `body_condition` not in interval
   * [0,1]
   */
  void force_body_condition(const double body_condition);

  /// Current energy needs [MJ/ind]
  double get_energy_needs() const { return energy_needs; }

  /// Current fat mass [kg/ind].
  double get_fatmass() const { return fatmass; }

  /// Get the maximum energy [MJ/ind/day] that could be anabolized in a day
  double get_max_anabolism_per_day() const;

  /// Maximum fat mass [kg/ind].
  double get_max_fatmass() const { return max_fatmass; }

  /// Merge this object with another one by building weighted averages.
  /**
   * \param other The other object.
   * \param this_weight Weight for this object.
   * \param other_weight Weight for the other object.
   */
  void merge(const FatmassEnergyBudget& other, const double this_weight,
             const double other_weight);

  /// Update budget and fat mass by receiving energy (through feeding)
  /**
   * If `energy` exceeds current energy needs, the surplus is stored
   * as fat (anabolism).
   * \param energy Input energy [MJ/ind].
   * \throw std::invalid_argument If `energy<0.0`.
   * \throw std::logic_error if `energy` exceeds current
   * energy needs and maximum anabolism.
   */
  void metabolize_energy(double energy);

  /// Set the maximum allowed fat mass [kg/ind] and fat gain [kg/ind/day].
  /**
   * \param max_fatmass Current maximum fat mass [kg/ind].
   * \param max_gain Maximum fat mass gain [kg/ind/day]. A value of
   * zero indicates no limit.
   * \throw std::logic_error if `max_fatmass` is smaller than
   * current fat mass
   * \throw std::invalid_argument If `max_fatmass<=0.0` or
   * if `max_gain < 0`.*/
  void set_max_fatmass(const double max_fatmass, const double max_gain);

 private:
  double anabolism_coefficient;   // MJ/kg
  double catabolism_coefficient;  // MJ/kg
  double energy_needs = 0.0;      // MJ/ind
  double fatmass;                 // kg/ind
  double max_fatmass;             // kg/ind
  double max_fatmass_gain = 0.0;  // kg/ind/day
};
}  // namespace Fauna
#endif  // FAUNA_FATMASS_ENERGY_BUDGET_H
