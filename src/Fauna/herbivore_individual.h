/**
 * \file
 * \brief Implementation of \ref Fauna::HerbivoreBase as individual animals.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_HERBIVORE_INDIVIDUAL_H
#define FAUNA_HERBIVORE_INDIVIDUAL_H

#include "herbivore_base.h"

namespace Fauna {

/// One herbivore individual
/**
 * \see \ref sec_design_the_herbivore
 */
class HerbivoreIndividual : public HerbivoreBase {
 public:
  // -------- HerbivoreInterface ----------
  virtual double get_ind_per_km2() const {
    assert(area_km2 > 0.0);
    return 1.0 / area_km2 * !is_dead();
  }
  virtual bool is_dead() const { return dead; }
  virtual void kill() { dead = true; }

 public:
  /// Establishment constructor
  /**
   * \param age_days Age in days (must be > 0).
   * \param body_condition Initial fat reserve as fraction
   * of physiological maximum [kg/kg].
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \throw std::invalid_argument if any parameter is invalid
   *
   * \param area_km2 The absolute area of the habitat [km²].
   */
  HerbivoreIndividual(const int age_days, const double body_condition,
                      const Hft* hft, const Sex sex, const double area_km2,
                      const ForageEnergyContent& metabolizable_energy);

  /// Birth constructor
  /**
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \throw std::invalid_argument if any parameter is invalid
   *
   * \param area_km2 The absolute area of the habitat [km²].
   */
  HerbivoreIndividual(const Hft* hft, const Sex sex, const double area_km2,
                      const ForageEnergyContent& metabolizable_energy);

  /// Habitat area [km²]
  double get_area_km2() const { return area_km2; }

 protected:
  // -------- HerbivoreBase ---------------
  virtual void apply_mortality(const double mortality);

 private:
  double area_km2;  // constant
  bool dead;
};
}  // namespace Fauna
#endif  // FAUNA_HERBIVORE_INDIVIDUAL_H
