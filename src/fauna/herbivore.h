//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Herbivore interfaces and classes.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
//////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_HERBIVORE_H
#define HERBIV_HERBIVORE_H

#include <memory>           // for std::auto_ptr
#include "forageclasses.h"  // for ForageMass
#include "nitrogen.h"       // for NitrogenInHerbivore
#include "utils.h"          // for Sex

// Forward declarations
namespace FaunaOut {
class HerbivoreData;
}

namespace Fauna {

// Forward declaration of classes in the same namespace
class FatmassEnergyBudget;
class GetForageDemands;
class GetNetEnergyContentInterface;
class HabitatEnvironment;
class Hft;

/// Interface of a herbivore of a specific \ref Hft.
/**
 * Derived classes will define the model mechanics.
 * Each herbivore class has a corresponding implementation of
 * \ref PopulationInterface which creates and manages the
 * herbivores.
 * \see \ref sec_herbiv_new_herbivore_class
 * \see \ref PopulationInterface
 * \see \ref sec_herbiv_herbivoredesign
 */
struct HerbivoreInterface {
  /// Virtual Destructor
  /** Destructor must be virtual in an interface. */
  virtual ~HerbivoreInterface() {}

  /// Feed the herbivore dry matter forage.
  /**
   * \param kg_per_km2 Dry matter forage mass [kgDM/km²].
   * \param N_kg_per_km2 Nitrogen in the forage [kgN/km²].
   * \param digestibility Proportional forage digestibility.
   * \throw std::logic_error If `forage` exceeds
   * intake constraints of maximal foraging and digestion.
   * \throw std::logic_error If this herbivore is dead or
   * has no individuals.
   */
  virtual void eat(const ForageMass& kg_per_km2,
                   const Digestibility& digestibility,
                   const ForageMass& N_kg_per_km2 = ForageMass(0)) = 0;

  /// Body mass of one individual [kg/ind].
  virtual double get_bodymass() const = 0;

  /// Get the forage the herbivore would like to eat today.
  /**
   * Call this after \ref simulate_day().
   * \note This may be called multiple times a day in order to allow
   * switching to another forage type!
   *
   * \param available_forage Available forage in the habitat
   * [kgDM/km²].
   * \return Dry matter forage *per m²* that the herbivore
   * would eat without any food competition [kgDM/km²].
   */
  virtual ForageMass get_forage_demands(
      const HabitatForage& available_forage) = 0;

  /// Get the herbivore functional type
  virtual const Hft& get_hft() const = 0;

  /// Individuals per km²
  virtual double get_ind_per_km2() const = 0;

  /// Get herbivore biomass density [kg/km²]
  virtual double get_kg_per_km2() const = 0;

  /// Read current output.
  virtual const FaunaOut::HerbivoreData& get_todays_output() const = 0;

  /// Whether the herbivore object is dead.
  virtual bool is_dead() const = 0;

  /// Mark this herbivore as dead (see \ref is_dead()).
  /** This function is needed in order to take nitrogen back from “dead”
   * herbivores before removing them from memory. */
  virtual void kill() = 0;

  /// Simulate daily events.
  /**
   * Call this before \ref get_forage_demands().
   * \param[in] day Current day of year, 0=Jan. 1st.
   * \param[in] environment Current environmental conditions in the habitat.
   * \param[out] offspring Number of newborn today [ind/km²].
   * \throw std::invalid_argument If `day` not in [0,364].
   * \throw std::logic_error If this herbivore is dead.
   */
  virtual void simulate_day(const int day,
                            const HabitatEnvironment& environment,
                            double& offspring) = 0;

  /// Get how much nitrogen is excreted, and reset.
  /**
   * Through feeding, plant nitrogen is taken up. Any nitrogen that has
   * been excreted again can be queried with this function. This function
   * also resets the accumulated nitrogen to zero.
   * This way, the nitrogen cycle from plant to animal and back is
   * completely closed.
   *
   * If the herbivore is dead, *all* remaining nitrogen in the body
   * (including tissue) ought to be returned.
   *
   * Sidenote: The function name doesn’t start with `get_` because the
   * function changes the internal state of the object.
   * \return Excreted nitrogen [kgN/km²] (+ tissue nitrogen if dead).
   * \see \ref sec_herbiv_nitrogen_excretion
   */
  virtual double take_nitrogen_excreta() = 0;
};

/// Abstract base class for herbivores.
/**
 * Calculations are generally performed *per* individual.
 *
 * \note Several member variables are declared as std::auto_ptr. This
 * is done in order to reduce header includes here.
 * \see \ref sec_herbiv_herbivoredesign
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
  virtual const FaunaOut::HerbivoreData& get_todays_output() const;
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
  /** \see \ref Hft::conductance
   * \throw std::logic_error If \ref ConductanceModel not implemented. */
  double get_conductance() const;

  /// Current fat mass [kg/ind].
  double get_fatmass() const;

  /// Get fat-free body mass [kg/ind].
  /**
   * Use \ref Hft::bodymass_male or \ref Hft::bodymass_female
   * if older than \ref Hft::maturity_age_phys_male or
   * \ref Hft::maturity_age_phys_female, respectively.
   * Otherwise interpolate linearly from \ref Hft::bodymass_birth.
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
   * Herbivores are born with \ref Hft::bodyfat_birth.
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \throw std::invalid_argument If `hft==NULL`.
   */
  HerbivoreBase(const Hft* hft, const Sex sex);

  /// Copy constructor.
  HerbivoreBase(const HerbivoreBase& other);

  /// Copy assignment
  HerbivoreBase& operator=(const HerbivoreBase& other);

  /// Destructor
  // Note that std::auto_ptr cleans up itself, no need to do
  // implement anything in the destructor.
  ~HerbivoreBase() {}

  /// Apply a fractional mortality.
  /**
   * \param mortality Fraction of individual density/count
   * that is dying.
   * \throw std::invalid_argument if not `0.0≤mortality≤1.0`
   */
  virtual void apply_mortality(const double mortality) = 0;

  /// @{ \brief The herbivore’s energy budget object.
  FatmassEnergyBudget& get_energy_budget() {
    assert(energy_budget.get() != NULL);
    return *energy_budget;
  }
  const FatmassEnergyBudget& get_energy_budget() const {
    assert(energy_budget.get() != NULL);
    return *energy_budget;
  }
  /**@}*/

  /// Current abiotic conditions in the habitat.
  /**
   * \throw std::logic_error If \ref simulate_day() hasn’t been called
   * yet to set the \ref HabitatEnvironment object.
   */
  const HabitatEnvironment& get_environment() const;

  /// Class-internal read/write access to current output.
  FaunaOut::HerbivoreData& get_todays_output();

  /// Access for derived classes to nitrogen management.
  NitrogenInHerbivore& get_nitrogen() { return nitrogen; }

 private:  // private member functions
  /// Calculate mortality according to \ref Hft::mortality_factors.
  /** Calls \ref apply_mortality(), which is implemented by
   * child classes.*/
  void apply_mortality_factors_today();

  /// Check whether the HFT pointer is NULL and throws an exception.
  /** This helper function is only neededd because an exception for a
   * NULL pointer must be thrown from the constructor. But in the
   * initialization list there are objects that need the HFT for their
   * own constructor. Therefore, the HFT pointer must be checked in a
   * function first, before the HerbivoreBase constructor function body.
   * \throw std::invalid_argument If HFT pointer is NULL. */
  Hft const* check_hft_pointer(const Hft*);

  /// Forage net energy content given by the selected algorithm \ref
  /// Hft::net_energy_model.
  /**
   * \param digestibility Proportional digestibility.
   * \return Net energy content [MJ/kgDM].
   * \throw std::logic_error If the selected model is not
   * implemented. */
  ForageEnergyContent get_net_energy_content(
      const Digestibility& digestibility) const;

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
   * reproductive maturity (\ref Hft::maturity_age_sex).
   * \throw std::logic_error If \ref Hft::reproduction_model
   * is not implemented.
   */
  double get_todays_offspring_proportion() const;

 private:  // private member variables
  /// @{ \name Constants
  Hft const* hft;  // pointer to const Hft; initialized first!
  Sex sex;
  std::auto_ptr<GetNetEnergyContentInterface> net_energy_content;
  /** @} */  // constants

  /// @{ \name State Variables
  int age_days;
  // use auto_ptr to reduce dependencies:
  std::auto_ptr<FatmassEnergyBudget> energy_budget;
  std::auto_ptr<HabitatEnvironment> environment;  // set in simulate_day()
  NitrogenInHerbivore nitrogen;
  int today;
  /** @} */  // state variables

  /// @{ \name Helper Classes
  /// Body condition over the past x months (only females).
  /** Body condition is current fat mass / max. fat mass. The record
   * spans the lenght of a potential pregnancy, counting back from
   * current day. This object is empty for male herbivores. */
  PeriodAverage body_condition_gestation;

  // use auto_ptr to reduce dependencies:
  std::auto_ptr<FaunaOut::HerbivoreData> current_output;
  std::auto_ptr<GetForageDemands> get_forage_demands_per_ind;
  /** @} */  // Helper Classes
};

/// One herbivore individual
/**
 * \see \ref sec_herbiv_herbivoredesign
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
                      const Hft* hft, const Sex sex, const double area_km2);

  /// Birth constructor
  /**
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore.
   * \throw std::invalid_argument if any parameter is invalid
   *
   * \param area_km2 The absolute area of the habitat [km²].
   */
  HerbivoreIndividual(const Hft* hft, const Sex sex, const double area_km2);

  /// Copy Constructor.
  HerbivoreIndividual(const HerbivoreIndividual& other);

  /// Copy assignment operator.
  HerbivoreIndividual& operator=(const HerbivoreIndividual& other);

  /// Destructor
  ~HerbivoreIndividual(){};

  /// Habitat area [km²]
  double get_area_km2() const { return area_km2; }

 protected:
  // -------- HerbivoreBase ---------------
  virtual void apply_mortality(const double mortality);

 private:
  double area_km2;  // constant
  bool dead;
};

/// A herbivore cohort (age-class)
/**
 * Any state variables describe mean values across all
 * individuals.
 * All individuals have the same age.
 * \see \ref sec_herbiv_herbivoredesign
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
#endif  // HERBIV_HERBIVORE_H
