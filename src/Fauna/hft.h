/**
 * \file
 * \brief Herbivore Functional Type (HFT) class and its dependents.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_HFT_H
#define FAUNA_HFT_H

#include <array>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Fauna {

// forward declarations
class Parameters;

/// Coefficient and exponent for an allometric relationship.
/**
 * The allometric relationship:
 * \f[
 * x = c * M^e
 * \f]
 * - $c$ = \ref coefficient
 * - $e$ = \ref exponent
 */
struct AllometryParameters {
  /// Constructor.
  AllometryParameters(const double coefficient, const double exponent)
      : coefficient(coefficient), exponent(exponent) {}

  /// Factor `c` in `x=c*M^e`.
  double coefficient;

  /// Exponent `e` in `x=c*M^e`.
  double exponent;

  /// Calculate the result of the formula.
  double calc(const double M) const { return coefficient * pow(M, exponent); }
};

/// Selector for a function of how to calculate whole-body conductance.
/** \see \ref sec_thermoregulation */
enum class ConductanceModel {
  /// Allometric formula from data by Bradley & Deavers (1980)
  /// \cite bradley1980reexamination.
  /** \see \ref get_conductance_bradley_deavers_1980() */
  BradleyDeavers1980,

  /// Allometric formula extrapolating from reindeer fur conductivity.
  /** \see \ref get_conductance_cuyler_oeritsland_2004 */
  CuylerOeritsland2004
};

/// Model to define a herbivore’s diet in a multi-forage scenario.
enum class DietComposer {
  /// Eat exclusively grass.
  PureGrazer
};

/// Algorithm to calculate the daily digestive capacity of a herbivore.
enum class DigestiveLimit {
  /// No digestive limit.
  None,

  /// Dry-matter ingestion is limited to a fraction of live herbivore body mass.
  /** \see Hft::digestion_allometric */
  Allometric,

  /// Dry-matter ingestion is limited to a fixed fraction of live body mass.
  /**
   * The parameter \ref Hft::digestion_fixed_fraction (\f$f_{ad}\f$)
   * defines the maximum daily intake in dry matter as a fraction of
   * body mass. It applies only to adult animals. Younger animals have a
   * higher metabolic rate per body mass and thus also need a higher
   * digestive limit. Since the metabolic rate generally scales roughly
   * with \f$M^{0.75}\f$ (Kleiber 1961\cite kleiber1961fire), the
   * metabolic rate *per body mass* scales with \f$M^{-0.25}\f$. It is
   * assumed that the maximum intake (\f$DMI\f$) for physically immature
   * animals scales in the same way. This is not necessarily the case
   * (and also depends on the chosen energy expenditure model), but the
   * assumption may suffice for the purpose of assuring that juveniles
   * don’t succumb due to a mismatch between intake and expenditure. The
   * case of lactation is not considered here, but even newborns are
   * modelled to eat fresh forage. Hence the increased DMI of juveniles
   * can be considered a surrogate for the support of a lactating
   * mother.
   * \f[
   * DMI =
   *   \\begin{cases}
   *   f_{ad} * M \\text{if adult: } M = M_{ad}\\\\
   *   f_{ad}*M_{ad}^{-0.75} * M^{-0.75} \\text{if } M < M_{ad}
   *   \\end{cases}
   * \f]
   * \image html scale_diglimit.svg "Scaling of digestive limit for
   * non-mature animals."
   */
  FixedFraction,

  /// Limit digestive limit with \ref get_digestive_limit_illius_gordon_1992().
  IlliusGordon1992
};

/// Algorithm to calculate a herbivore’s daily energy needs.
enum class ExpenditureComponent {
  /// Simple exponential expenditure function based on current body mass.
  /**
   * \f[
   * E = c * M^e
   * \f]
   * - $E$: Daily energy expenditure [MJ/ind/day]
   * - $c$: Coefficient (\ref AllometryParameters::coefficient in
   *   member \ref Hft::expenditure_allometric)
   * - $M$: Current body mass [kg/ind]
   * - $e$: Allometric exponent (\ref AllometryParameters::exponent in
   *   member \ref Hft::expenditure_allometric)
   */
  Allometric,

  /// Formula for field metabolic rate in cattle:
  /// \ref get_expenditure_taylor_1981()
  Taylor1981,

  /// Increased expenditure in lower temperature, see
  /// \ref sec_thermoregulation.
  Thermoregulation,

  /// Formula for temperature-dependent field metabolic rate:
  /// \ref get_expenditure_zhu_et_al_2018()
  Zhu2018
};

/// A factor limiting a herbivore’s daily forage harvesting.
enum class ForagingLimit {
  /// Type-II functional response applied “on top” of all other constraints.
  /**This has no empirical basis, but serves as a way to implement an
   * “artificial” smooth negative feedback from forage to fat mass.
   *
   * Note that this model is functionally similar to
   * \ref ForagingLimit::IlliusOConnor2000, but Illius & O’Connor (2000)
   * \cite illius2000resource use a value for
   * \ref Hft::foraging_half_max_intake_density that is empirical-based.
   *
   * Here, it shall be explicitely stated that the half-max intake
   * density is *not* based on observation, but solely used as a means
   * to create a workable forage–intake feedback.
   *
   * \see \ref Hft::foraging_half_max_intake_density, \ref HalfMaxIntake
   */
  GeneralFunctionalResponse,

  /// Foraging is limited by a functional response towards digestion limit.
  /**
   * Illius & O’Connor (2000) \cite illius2000resource describe daily
   * food intake rate as a Holling Type II functional response
   * (compare \ref HalfMaxIntake).
   * As the maximum daily energy intake they choose the digestive
   * capacity (compare \ref get_digestive_limit_illius_gordon_1992()).
   *
   * Like in the model of Pachzelt et al. (2013)
   * \cite pachzelt2013coupling, the grass forage density of the
   * whole patch (habitat) is used (not the sward density
   *
   * \note This model for maximum foraging works only for pure grazers
   * (⇒ \ref DietComposer::PureGrazer).
   * \ref GrassForage::get_sward_density()).
   * \see \ref Hft::foraging_half_max_intake_density
   */
  IlliusOConnor2000
};

/// How forage net energy content is calculated.
enum class NetEnergyModel {
  /// Use \ref get_net_energy_content_default()
  Default
};

/// One way how a herbivore can die.
enum class MortalityFactor {
  /// Independent background mortality for juveniles and adults.
  /** \see \ref GetBackgroundMortality */
  Background,

  /// A herbivore dies if its age exceeds \ref Hft::life_history_lifespan.
  /** \see \ref Fauna::GetSimpleLifespanMortality */
  Lifespan,

  /// Starvation death following Illius & O’Connor (2000).
  /** \see \ref Fauna::GetStarvationIlliusOConnor2000 */
  StarvationIlliusOConnor2000,

  /// Starvation death at a minimum bodyfat threshold.
  StarvationThreshold
};

/// Algorithm to calculate herbivore reproduction time and success.
enum class ReproductionModel {
  /// Disable reproduction all together.
  None,
  /// Use class \ref ReproductionConstMax for reproduction.
  ConstantMaximum,
  /// Use class \ref ReprIlliusOconnor2000 to calculate reproduction.
  IlliusOConnor2000,
  /// Use class \ref ReproductionLinear for reproduction.
  Linear
};

/// One herbivore functional type (i.e. one species).
/**
 * Parametes are sorted alphabetically and grouped by semantic categories,
 * which are indicated with a prefix to the member name.
 *
 * The member variable names shall be spelled exactly the same as the keys in
 * the TOML instruction file. The categories correspond to the tables in the
 * TOML file. The prefix (separated with "_") is spelled the same as the TOML
 * table.
 *
 * The initialization values are the same as in the example file under
 * `examples/megafauna.toml`. They must be valid in the context of the
 * global parameters of the instruction file.
 */
struct Hft {
  /// Check if all variables are okay
  /**
   * \param[in] params The global simulation parameters.
   * \param[out] msg Warning or error messages for output.
   * \return true if the object has valid values */
  bool is_valid(const Parameters& params, std::string& msg) const;

  /// Check if all variables are okay
  /**
   * \param[in] params The global simulation parameters.
   * \return true if the object has valid values */
  bool is_valid(const Parameters& params) const;

  /// Unique name of the herbivore type.
  std::string name = "example";

  /** @{ \name "body_fat": Body fat parameters. */
  /// Proportional fat mass at birth [kg/kg].
  double body_fat_birth = 0.2;

  /// Standard deviation in body condition for
  /// \ref GetStarvationIlliusOConnor2000.
  /**
   * Body condition is the proportion of current body fat relative to
   * physiological maximum.
   * For herbivore individuals, the standard deviation refers to the
   * whole population. In cohort mode, it refers only to one cohort.
   *
   * \note For juveniles (1st year of life), body fat variation is
   * always zero in order to avoid artificially high death rates if
   * body fat is low at birth.
   */
  double body_fat_deviation = 0.125;

  /// Maximum proportional fat mass [kg/kg].
  double body_fat_maximum = 0.3;

  /// Maximum rate of fat mass gain in kg fat per kg body mass per day.
  /** A value of zero indicates no limit. */
  double body_fat_maximum_daily_gain = 0.05;
  /** @} */

  /** @{ \name "body_mass": Body mass parameters. */
  /// Body mass [kg] at birth for both sexes.
  int body_mass_birth = 5;

  /// Body mass [kg] of an adult female individual (with full fat reserves).
  int body_mass_female = 50;

  /// Body mass [kg] of an adult male individual (with full fat reserves).
  int body_mass_male = 70;
  /** @} */

  /** @{ \name "breeding_season": Time period of giving birth. */
  /// Length of breeding season in days.
  int breeding_season_length = 30;

  /// Julian day of the beginning of the breeding season (0=Jan 1st).
  int breeding_season_start = 121;
  /** @} */

  /** @{ \name "digestion": Digestion-related parameters. */
  /// Parameters for \ref DigestiveLimit::Allometric
  AllometryParameters digestion_allometric = {0.05, 0.76};

  /// Conversion factor from net forage energy to fat mass [MJ/kg].
  /** The default value is from Peters (1983)\cite peters1983ecological. */
  double digestion_anabolism_coefficient = 54.6;

  /// Conversion factor from fat mass to net energy [MJ/kg].
  /** The default value is from Peters (1983)\cite peters1983ecological. */
  double digestion_catabolism_coefficient = 39.3;

  /// Factor for reducing forage net energy content for non-ruminants.
  /**
   * The default model for net energy content
   * (\ref get_net_energy_content_default())
   * is designed for ruminant digestion. A constant factor may be applied to
   * account for less efficient digestive systems, e.g. hindgut fermentation.
   *
   * For hindgut fermenters there are various factors in the literature, e.g.:
   * - Johnson et al. (1982) give a value of 0.89 \cite johnson1982intake
   * - Foose (1982) gives a value of 0.84 \cite foose1982trophic
   * - The model by Illius & Gordon (1992) gives a value of 0.93 \cite
   * illius1992modelling \see \ref NetEnergyModel::Default \see \ref
   * get_net_energy_content_default() \see \ref
   * HerbivoreBase::get_net_energy_content()
   */
  double digestion_efficiency = 1.0;

  /// Constants i, j, k for \ref DigestionLimit::IlliusGordon1992 (grass only).
  /**
   * Shipley et al. (1999)\cite shipley1999predicting derived the parameters i,
   * j, and k from regression analysis with 12 mammalian herbivores (0.05--547
   * kg) and are specific to hindguts and ruminants.
   *
   * |     | Hindgut | Ruminant |
   * |-----|---------|----------|
   * | i   | 0.108   | 0.034    |
   * | j   | 3.284   | 3.565    |
   * | k   | 0.080   | 0.077    |
   *
   * \see \ref get_digestive_limit_illius_gordon_1992()
   * \see \ref DigestionLimit::IlliusGordon1992
   */
  std::array<double, 3> digestion_i_g_1992_ijk = {0.034, 3.565, 0.077};

  /// Daily dry matter intake per kg body mass for
  /// \ref DigestiveLimit::FixedFraction.
  double digestion_fixed_fraction = 0.05;

  /// Constraint for maximum daily forage intake.
  DigestiveLimit digestion_limit = DigestiveLimit::FixedFraction;
  /** @} */

  /** @{ \name "establishment": Spawning new herbivores in empty habitats. */
  /// Youngest and oldest age [years] for herbivore establishment.
  std::pair<int, int> establishment_age_range = {1, 15};

  /// Total population density for establishment in one habitat [ind/km²]
  double establishment_density = 1.0;
  /** @} */

  /** @{ \name "expenditure": Energy expenditure parameters. */
  /// Parameters for \ref ExpenditureComponent::Allometric.
  AllometryParameters expenditure_allometric = {0.4, 0.75};

  /// Energy expenditure components, summing up to actual expenditure.
  std::set<ExpenditureComponent> expenditure_components = {
      ExpenditureComponent::Allometric};
  /** @} */

  /** @{ \name "foraging": Parameters regulating food intake. */
  /// Model defining the herbivore’s diet composition.
  DietComposer foraging_diet_composer = DietComposer::PureGrazer;

  /// Constraints for maximum daily forage procurement.
  std::set<ForagingLimit> foraging_limits = {};

  /// Grass density [gDM/m²] where intake is half of its maximum.
  /**
   * Grass (sward) density at which intake rate reaches half
   * of its maximum (in a Holling Type II functional response).
   * Required by specific foraging limits.
   */
  double foraging_half_max_intake_density = 20;

  /// Algorithm for forage energy content.
  NetEnergyModel foraging_net_energy_model = NetEnergyModel::Default;
  /** @} */

  /** @{ \name "life_history": Life stages for herbivore individuals. */
  /// Maximum age in years [1–∞).
  int life_history_lifespan = 16;

  /// Age of physical maturity in years for females.
  int life_history_physical_maturity_female = 3;

  /// Age of physical maturity in years for males.
  int life_history_physical_maturity_male = 3;

  /// Age of female sexual maturity in years.
  int life_history_sexual_maturity = 2;
  /** @} */

  /** @{ \name "mortality": Parameters to define death of herbivores. */
  /// Annual background mortality rate [0.0–1.0) after first year of life.
  double mortality_adult_rate = 0.1;

  /// Ways how herbivores can die.
  std::set<MortalityFactor> mortality_factors = {
      MortalityFactor::Background, MortalityFactor::Lifespan,
      MortalityFactor::StarvationIlliusOConnor2000};

  /// Annual background mortality rate [0.0–1.0) in the first year of life.
  double mortality_juvenile_rate = 0.3;

  /// Minimum viable density of one HFT population (all cohorts) [frac.].
  /** Given as fraction of \ref establishment_density.
   * \see \ref sec_minimum_density_threshold */
  double mortality_minimum_density_threshold = 0.5;

  /// Whether to shift mean cohort body condition on starvation mortality.
  /** See \ref GetStarvationIlliusOConnor2000.*/
  bool mortality_shift_body_condition_for_starvation = true;
  /** @} */

  /** @{ \name "reproduction": Parameters relating to annual reproduction. */
  /// Maximum annual reproduction rate for females (0.0–∞)
  double reproduction_annual_maximum = 1.0;

  /// Duration of pregnancy [number of months].
  int reproduction_gestation_length = 9;

  /// Algorithm to calculate herbivore reproduction.
  ReproductionModel reproduction_model = ReproductionModel::ConstantMaximum;
  /** @} */

  /** @{ \name "thermoregulation": Expenditure through heat loss. */
  /// Algorithm to calculate whole-body conductance for thermoregulation.
  ConductanceModel thermoregulation_conductance =
      ConductanceModel::BradleyDeavers1980;

  /// Body core temperature [°C].
  /** Default is 38°C (Hudson & White, 1985\cite hudson1985bioenergetics) */
  double thermoregulation_core_temperature = 38;
  /** @} */

  /** @{ \name Comparison operators*/
  /** Comparison operators are solely based on string
   * comparison of the \ref name.
   */
  bool operator==(const Hft& rhs) const { return name == rhs.name; }
  bool operator!=(const Hft& rhs) const { return name != rhs.name; }
  bool operator<(const Hft& rhs) const { return name < rhs.name; }
  /** @} */  // Comparison
};

}  // namespace Fauna
#endif  // FAUNA_HFT_H
