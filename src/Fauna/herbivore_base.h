// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief A versatile base class to derive herbivores from.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_HERBIVORE_BASE_H
#define FAUNA_HERBIVORE_BASE_H

#include <memory>
#include "breeding_season.h"
#include "environment.h"
#include "fatmass_energy_budget.h"
#include "get_forage_demands.h"
#include "herbivore_data.h"
#include "herbivore_interface.h"

namespace Fauna {

/// The sex of a herbivore
enum class Sex { Female, Male };

/// Abstract base class for herbivores.
/**
 * Calculations are generally performed per individual.
 */
class HerbivoreBase : public HerbivoreInterface {
 public:
  // -------- HerbivoreInterface ----------
  virtual void eat(const ForageMass& kg_per_km2,
                   const Digestibility& digestibility,
                   const ForageMass& N_kg_per_km2);
  virtual ForageMass get_forage_demands(const HabitatForage& available_forage);
  virtual std::string get_output_group() const;
  virtual double get_kg_per_km2() const;
  virtual const Output::HerbivoreData& get_todays_output() const {
    return current_output;
  }
  virtual void simulate_day(const int day,
                            const HabitatEnvironment& environment,
                            double& offspring);

 public:
  /// Current age in days.
  int get_age_days() const { return age_days; }

  /// Current age in years.
  double get_age_years() const { return age_days / 365.0; }

  /// Proportional body fat in empty body (fat mass/empty body mass).
  /**
   * The current body fraction, \f$bf\f$ is given by the current fat mass,
   * \f$FM\f$, and the structural mass, \f$SM\f$. Structural mass and fat mass
   * together constitute the empty body.
   * \f[
   * bf = \frac{FM}{SM + FM}
   * \f]
   * \see \ref sec_body_mass_and_composition
   */
  double get_bodyfat() const;

  /// Current live weight, including fat, ingesta, blood, etc. [kg/ind].
  /**
   * Current body mass, \f$BM\f$, is the sum of the empty body and the non-body
   * parts, like ingesta, blood, etc. The empty body is given by \f$BM * eb\f$,
   * where \f$eb\f$ is the proportion of empty body mass given by
   * \ref Hft::body_mass_empty.
   *
   * Since the empty body consists of fat mass, \f$FM\f$, and  structural mass,
   * \f$SM\f$, we can write
   * \f[
   * BM = SM + FM + BM * (1 - eb)
   * \f]
   * and simplify that to:
   * \f[
   * BM = \frac{SM + FM}{eb}
   * \f]
   * \see \ref sec_body_mass_and_composition
   */
  virtual double get_bodymass() const;

  /// Body mass at physical maturity [kg/ind].
  /**
   * \ref Hft::body_mass_female for females and \ref Hft::body_mass_male for
   * males.
   */
  double get_bodymass_adult() const;

  /// Get full-body conductance [W/°C/ind].
  /**
   * \see \ref Hft::thermoregulation_conductance
   * \throw std::logic_error If \ref ConductanceModel not implemented.
   */
  double get_conductance() const;

  /// Current fat mass (pure lipids) [kg/ind].
  /**
   * \see \ref sec_body_mass_and_composition
   */
  double get_fatmass() const;

  /// The herbivore functional type (HFT).
  const Hft& get_hft() const {
    assert(hft);
    return *hft;
  }

  /// Physiological maximum of fat mass [kg/ind].
  /**
   * The maximum fat mass, \f$FM_{max}\f$ is given by applying the fraction
   * \ref Hft::body_fat_maximum, \f$bf_{max}\f$, to the empty body. The empty
   * body mass is the sum of structural mass, \f$SM\f$, and current fat mass,
   * which is in here the maximum \f$FM_{max}\f$.
   *
   * \f{eqnarray*}{
   * bf_{max}                       &=& \frac{FM_{max}}{SM + FM_{max}} \\
   * FM_{max}                       &=& SM * bf_{max} + FM_{max} * bf_{max} \\
   * FM_{max} - FM_{max} * bf_{max} &=& SM * bf_{max} \\
   * FM_{max} * (1 - bf_{max})      &=& SM * bf_{max} \\
   * FM_{max}                       &=& \frac{SM * bf_{max}}{1 - bf_{max}}
   * \f}
   * \see \ref sec_body_mass_and_composition
   */
  double get_max_fatmass() const;

  /// Current mass of body tissue without fat, ingesta, blood, etc. [kg/ind]
  /**
   * This is the body component that an ontogenetic growth curve applies to.
   * The growth curve interpolates between the neonate structural mass and the
   * adult structural mass.
   *
   * The neonate structural mass, \f$SM_{birth}\f$, is derived from the birth
   * weight, \f$BM_{birth}\f$ (\ref Hft::body_mass_birth), the body fat at
   * birth, \f$bf_{birth}\f$ (\ref Hft::body_fat_birth), and the empty body
   * fraction, \f$eb\f$ (\ref Hft::body_mass_empty). The structural mass is the
   * empty body, which is \f$BM_{birth}*eb\f$, minus the fractional body fat
   * \f$bf_{birth}\f$:
   * \f[
   * SM_{birth} = BM_{birth} * eb * (1 - bf_{birth})
   * \f]
   *
   * The adult structural mass, \f$SM_{ad}\f$, is calculated in a similar way,
   * only that the body fat fraction for the adult live weight parameter
   * \f$BM_{ad}\f$ (\ref Hft::body_mass_female, \ref Hft::body_mass_male) is
   * assumed to be half of the maximum \f$bf_{max}\f$
   * (\ref Hft::body_fat_maximum):
   * \f[
   * SM_{ad} = BM_{ad} * eb * (1 - \frac{bf_{max}}{2})
   * \f]
   * \see \ref sec_body_mass_and_composition
   */
  double get_structural_mass() const;

  /// Current day of the year, as set in \ref simulate_day().
  /** \throw std::logic_error If current day not yet set by an
   * initial call to \ref simulate_day(). */
  int get_today() const;

  /// The sex of the herbivore
  Sex get_sex() const { return sex; }

 protected:
  /// Establishment constructor.
  /**
   * \param age_days age in days
   * \param body_condition Initial fat reserve as fraction
   * of physiological maximum [kg/kg].
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \param forage_gross_energy The (constant) gross energy content for the
   * forage types [MJ/kgDM]. See: \ref Parameters::forage_gross_energy
   * \throw std::invalid_argument If `hft==NULL` or
   * `age_days <= 0` or `body_condition` not in [0,1].
   */
  HerbivoreBase(const int age_days, const double body_condition,
                std::shared_ptr<const Hft> hft, const Sex sex,
                const ForageEnergyContent& forage_gross_energy);

  /// Birth constructor.
  /**
   * Herbivores are born with \ref Hft::body_fat_birth.
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \param forage_gross_energy The (constant) gross energy content
   * for the forage types [MJ/kgDM]. See: \ref Parameters::forage_gross_energy
   * \throw std::invalid_argument If `hft==NULL`.
   */
  HerbivoreBase(std::shared_ptr<const Hft> hft, const Sex sex,
                const ForageEnergyContent& forage_gross_energy);

  /// Virtual destructor, which will be called by derived classes.
  virtual ~HerbivoreBase() = default;

  /// Apply a fractional mortality.
  /**
   * \param mortality Fraction of individual density/count
   * that is dying.
   * \throw std::invalid_argument if not `0.0≤mortality≤1.0`
   */
  virtual void apply_mortality(const double mortality) = 0;

  /// The herbivore’s energy budget object.
  FatmassEnergyBudget& get_energy_budget() { return energy_budget; }

  /// The herbivore’s energy budget object.
  const FatmassEnergyBudget& get_energy_budget() const { return energy_budget; }

  /// Current abiotic conditions in the habitat.
  /**
   * \throw std::logic_error If \ref simulate_day() hasn’t been called
   * yet to set the \ref HabitatEnvironment object.
   */
  const HabitatEnvironment& get_environment() const { return environment; }

  /// Class-internal read/write access to current output.
  Output::HerbivoreData& get_todays_output() { return current_output; }

  /// Check whether the constant member variables match those of another object.
  bool constant_members_match(const HerbivoreBase& other) const {
    return sex == other.sex && hft.get() == other.hft.get() &&
           forage_gross_energy == other.forage_gross_energy &&
           breeding_season == other.breeding_season;
  }

 private:  // private member functions
  /// Calculate mortality according to user-selected mortality factors
  /**
   * Calls \ref apply_mortality(), which is implemented by
   * child classes.
   * \see \ref Hft::mortality_factors
   */
  void apply_mortality_factors_today();

  /// Check whether the HFT pointer is NULL and throws an exception.
  /** This helper function is only neededd because an exception for a
   * NULL pointer must be thrown from the constructor. But in the
   * initialization list there are objects that need the HFT for their
   * own constructor. Therefore, the HFT pointer must be checked in a
   * function first, before the HerbivoreBase constructor function body.
   * \throw std::invalid_argument If HFT pointer is NULL. */
  std::shared_ptr<const Hft> check_hft_pointer(std::shared_ptr<const Hft>);

  /// Get forage energy content [MJ/kgDM] using selected net energy model.
  /**
   * \param digestibility Forage digestibility (for ruminants). This will be
   * adjusted with \ref Hft::digestion_digestibility_multiplier for
   * non-ruminants.
   * \return Net energy content in MJ/kgDM.
   * \throw std::logic_error If the net energy model is not implemented.
   * \see \ref Hft::digestion_net_energy_model
   * \see \ref NetEnergyModel
   */
  ForageEnergyContent get_net_energy_content(Digestibility digestibility) const;

  /// Calculate energy expenditure as sum of given expenditure components.
  /** \return Today’s energy needs [MJ/ind/day]
   * \see \ref Hft::expenditure_components */
  double get_todays_expenditure() const;

  /// Get the proportional offspring for today using selected model.
  /**
   * Reads \ref Hft::reproduction_model and performs calculations accordingly.
   * \return Number of offspring per individual [ind/ind/day].
   * Zero if this herbivore is male, or has not yet reached
   * reproductive maturity (\ref Hft::life_history_sexual_maturity).
   * \throw std::logic_error If \ref Hft::reproduction_model
   * is not implemented.
   */
  double get_todays_offspring_proportion() const;

 private:
  /// @{ \name Constants
  std::shared_ptr<const Hft> hft;  // pointer to const Hft; initialized first!
  const Sex sex;
  BreedingSeason breeding_season;
  ForageEnergyContent forage_gross_energy;
  /** @} */  // constants

  /// @{ \name State Variables
  int age_days;
  FatmassEnergyBudget energy_budget;
  HabitatEnvironment environment;  // set in simulate_day()
  int today;
  /** @} */  // state variables

  /// @{ \name Helper Classes
  /// Body condition over the past x months (only females).
  /** Body condition is current fat mass / max. fat mass. The record
   * spans the lenght of a potential pregnancy, counting back from
   * current day. This object is empty for male herbivores. */
  PeriodAverage body_condition_gestation;

  Output::HerbivoreData current_output;
  GetForageDemands get_forage_demands_per_ind;
  /** @} */  // Helper Classes
};

}  // namespace Fauna
#endif  // FAUNA_HERBIVORE_BASE_H
