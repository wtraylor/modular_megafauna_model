/**
 * \file
 * \brief Implementation of \ref HerbivoreBase as annual cohorts.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_HERBIVORE_COHORT_H
#define FAUNA_HERBIVORE_COHORT_H

#include "herbivore_base.h"

namespace Fauna {

/// A herbivore cohort (age-class)
/**
 * Any state variables describe mean values across all
 * individuals.
 * All individuals have the same age.
 * \see \ref sec_herbivoredesign
 */
class HerbivoreCohort : public HerbivoreBase {
 public:
  // -------- HerbivoreInterface ----------
  virtual double get_ind_per_km2() const { return ind_per_km2; }
  /// A cohort is dead if its density is zero.
  virtual bool is_dead() const;
  virtual void kill() { ind_per_km2 = 0.0; }

 public:
  /// Establishment constructor.
  /**
   * \param age_days age in days
   * \param body_condition Initial fat reserve as fraction
   * of physiological maximum [kg/kg].
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \throw std::invalid_argument if any parameter is invalid
   *
   * \param ind_per_km2 Initial individual density [ind/km²].
   * Can be 0.0, but must not be negative.
   */
  HerbivoreCohort(const int age_days, const double body_condition,
                  const Hft* hft, const Sex sex, const double ind_per_km2);

  /// Birth constructor
  /**
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \throw std::invalid_argument if any parameter is invalid
   *
   * \param ind_per_km2 Initial individual density [ind/km²].
   * Can be 0.0, but must not be negative.
   */
  HerbivoreCohort(const Hft* hft, const Sex sex, const double ind_per_km2);

  /// Copy constructor.
  HerbivoreCohort(const HerbivoreCohort& other);

  /// Copy assignment operator.
  HerbivoreCohort& operator=(const HerbivoreCohort& other);

  /// Destructor.
  ~HerbivoreCohort(){};

  /// Check if this and the other cohort are of the same age
  /**
   * Two cohorts are considered coeval if they are in the
   * same year of life:
   * - First year:  `0<=age_days<365`
   * - Second year: `365<=age_days<730`
   * - etc.
   */
  bool is_same_age(const HerbivoreCohort& other) const {
    return (this->get_age_days() / 365 == other.get_age_days() / 365);
  }

  /// Merge another cohort into this one.
  /**
   * All state variables are averaged between the two cohorts
   * by the weight of population density.
   * \param other The other cohort that is merged into `this`.
   * The density of `other` will be reduced.
   * \throw std::invalid_argument If `fraction` not in [0,1].
   * \throw std::logic_error If the other cohort is not
   * compatible: different age, different HFT, or different
   * sex.
   */
  void merge(HerbivoreCohort& other);

 protected:
  // -------- HerbivoreBase ---------------
  virtual void apply_mortality(const double mortality);

 private:
  double ind_per_km2;
};

}  // namespace Fauna
#endif  // FAUNA_HERBIVORE_COHORT_H
