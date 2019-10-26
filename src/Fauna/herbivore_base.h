/**
 * \file
 * \brief A versatile base class to derive herbivores from.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_HERBIVORE_BASE_H
#define FAUNA_HERBIVORE_BASE_H

#include <memory>
#include "environment.h"
#include "fatmass_energy_budget.h"
#include "get_forage_demands.h"
#include "herbivore_data.h"
#include "herbivore_interface.h"
#include "net_energy_models.h"
#include "nitrogen.h"

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
  virtual double get_bodymass() const;
  virtual ForageMass get_forage_demands(const HabitatForage& available_forage);
  virtual const Hft& get_hft() const {
    assert(hft != NULL);
    return *hft;
  }
  virtual double get_kg_per_km2() const;
  virtual const Output::HerbivoreData& get_todays_output() const {
    return current_output;
  }
  virtual void simulate_day(const int day,
                            const HabitatEnvironment& environment,
                            double& offspring);
  virtual double take_nitrogen_excreta();

 public:
  /// Current age in days.
  int get_age_days() const { return age_days; }

  /// Current age in years.
  double get_age_years() const { return age_days / 365.0; }

  /// Proportional body fat (fat mass/total body mass).
  double get_bodyfat() const;

  /// Body mass at physical maturity [kg/ind].
  double get_bodymass_adult() const;

  /// Get full-body conductance [W/°C/ind].
  /** \see \ref Hft::thermoregulation_conductance
   * \throw std::logic_error If \ref ConductanceModel not implemented. */
  double get_conductance() const;

  /// Current fat mass [kg/ind].
  double get_fatmass() const;

  /// Get fat-free body mass [kg/ind].
  /**
   * Use \ref Hft::body_mass_male or \ref Hft::body_mass_female
   * if older than \ref Hft::life_history_physical_maturity_male or
   * \ref Hft::life_history_physical_maturity_female, respectively.
   * Otherwise interpolate linearly from \ref Hft::body_mass_birth.
   */
  virtual double get_lean_bodymass() const;

  /// Physiological maximum of fat mass [kg/ind].
  double get_max_fatmass() const;

  /// The potential (maximum) body mass [kg/ind] with full fat reserves.
  double get_potential_bodymass() const;

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
   * \throw std::invalid_argument If `hft==NULL` or
   * `age_days <= 0` or `body_condition` not in [0,1].
   */
  HerbivoreBase(const int age_days, const double body_condition, const Hft* hft,
                const Sex sex);

  /// Birth constructor.
  /**
   * Herbivores are born with \ref Hft::body_fat_birth.
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \throw std::invalid_argument If `hft==NULL`.
   */
  HerbivoreBase(const Hft* hft, const Sex sex);

  /// Destructor
  ~HerbivoreBase() = default;

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

  /// Access for derived classes to nitrogen management.
  NitrogenInHerbivore& get_nitrogen() { return nitrogen; }

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
  Hft const* check_hft_pointer(const Hft*);

  /// Forage net energy content given by the selected algorithm
  /// \ref Hft::foraging_net_energy_model.
  /**
   * \return New instance of \ref GetNetEnergyContentInterface.
   * \throw std::logic_error If the selected model is not implemented.
   */
  GetNetEnergyContentInterface* create_net_energy_content_model() const;

  /// Calculate energy expenditure as sum of given expenditure components.
  /** \return Today’s energy needs [MJ/ind/day]
   * \see \ref Hft::expenditure_components */
  double get_todays_expenditure() const;

  /// Get the proportional offspring for today using selected model.
  /**
   * Reads \ref Hft::reproduction_model and performs
   * calculations accordingly.
   * \return Number of offspring per individual [ind/ind/day].
   * Zero if this herbivore is male, or has not yet reached
   * reproductive maturity (\ref Hft::life_history_sexual_maturity).
   * \throw std::logic_error If \ref Hft::reproduction_model
   * is not implemented.
   */
  double get_todays_offspring_proportion() const;

 private:
  /// @{ \name Constants
  Hft const* hft;  // pointer to const Hft; initialized first!
  Sex sex;
  std::shared_ptr<GetNetEnergyContentInterface> get_net_energy_content;
  /** @} */  // constants

  /// @{ \name State Variables
  int age_days;
  FatmassEnergyBudget energy_budget;
  HabitatEnvironment environment;  // set in simulate_day()
  NitrogenInHerbivore nitrogen;
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
