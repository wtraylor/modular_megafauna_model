#ifndef FAUNA_EXPENDITURE_COMPONENTS_H
#define FAUNA_EXPENDITURE_COMPONENTS_H

#include <assert.h>
#include <cmath>
#include <stdexcept>

namespace Fauna {
/// Energy expenditure [MJ/ind/day] based on cattle from Taylor et al. (1981)
/**
 * Taylor et al. (1981)\cite taylor1981genetic
 * Daily energy expenditure is given by
 * \f[
 * E [\frac{MJ}{day*ind}] = 0.4 * M * M_{ad}^{-0.27}
 * \f]
 * where M is current body mass [kg/ind] and \f$M_{ad}\f$ is
 * adult body mass.
 *
 * \return energy expenditure in MJ for one herbivore individual
 * per day [MJ/day/ind]
 */
inline double get_expenditure_taylor_1981(const double current_bodymass,
                                          const double adult_bodymass) {
  return 0.4 * current_bodymass * pow(adult_bodymass, -0.27);
}

/// Daily expenditure by Zhu et al (2018) \cite zhu2018large.
/**
 * \f[
 * E = \frac{k_2}{e^{k_1*T}} * A^{0.75}
 * \f]
 * - \f$E\f$: Daily energy expenditure [MJ/ind/day].
 * - \f$A\f$: Body mass [kg/ind].
 * - \f$k_1 = 0.0079\f$: Constant, derived from regression analysis of
 *   data from Anderson & Jetz (2005)\cite anderson2005broadscale.
 * - \f$k_2 = 0.36\f$: Constant, calibrated to yield a range close to the
 *   values in Illius & O’Connor (2000)\cite illius2000resource.
 * \param bodymass Current body mass [kg/ind].
 * \param ambient_temperature Long-term mean air temperature [°C].
 * \return Daily energy expenditure per individual [MJ/ind/day].
 * \throw std::invalid_argument If `bodymass <= 0`.
 * \see Fauna::EC_ZHU_2018
 */
inline double get_expenditure_zhu_et_al_2018(const double bodymass,
                                             const double ambient_temperature) {
  if (bodymass <= 0)
    throw std::invalid_argument(
        "Fauna::get_expenditure_zhu_et_al_2018() "
        "Parameter `bodymass` is <=0.");
  const double k1 = 0.0079;
  const double k2 = 0.36;
  return k2 / (exp(k1 * ambient_temperature)) * pow(bodymass, 0.75);
}

/// Convert Watts (=J/s) to MJ/day.
inline double watts_to_MJ_per_day(const double W) {
  return W * 24 * 3600 * 10e-6;
}

/// Get full body conductance [W/°C] after Bradley & Deavers (1980)
/// \cite bradley1980reexamination
/**
 * The formula is taken from Peters (1983)\cite peters1983ecological,
 * which is based on data by Bradley & Deavers (1980).
 * \f[
 * C = 0.224 * M^{0.574}
 * \f]
 * 230 conductance values from 192 mammal species with body weights
 * ranging from 3.5 g to 150 kg.
 * \param bodymass Current body weight [kg/ind].
 * \throw std::invalid_argument If `bodymass <= 0`.
 * \return Full body conductance [W/°C].
 */
inline double get_conductance_bradley_deavers_1980(const double bodymass) {
  if (bodymass <= 0.0)
    throw std::invalid_argument(
        "Fauna::get_conductance_bradley_deavers_1980() "
        "Parameter `bodymass` is <=0.");
  return 0.224 * pow(bodymass, 0.574);
}

/// Selector for winter or summer pelt.
enum FurSeason {
  /// Summer fur
  FS_SUMMER,
  /// Winter fur
  FS_WINTER
};

/// Extrapolate conductance from reindeer fur.
/**
 * Cuyler & Øritsland (2004)\cite cuyler2004rain measured conductivity
 * values of reindeer (Rangifer tarandus) pelts in calm air and dry
 * conditions:
 * - 0.63 W/(°C*m²) in winter
 * - 2.16 W/(°C*m²) in summer
 *
 * We assume a body mass of 60 kg for an adult reindeer
 * (Soppela et al. 1986 \cite soppela1986thermoregulation).
 * Body surface is approximated by a formula from Hudson & White (1985)
 * \cite hudson1985bioenergetics as \f$0.09*M^{0.66}\f$ (in m²).
 *
 * The whole-body conductance in W/°C is then:
 * - for winter \f$0.63 * 0.09 * 60^{0.66} = 0.8\f$
 * - for summer \f$2.16 * 0.09 * 60^{0.66} = 2.9\f$
 *
 * Both Bradley & Deavers (1980)\cite bradley1980reexamination and
 * Fristoe et al. (2014)\cite fristoe2015metabolic suggest that the
 * allometric exponent for body mass for whole-body conductance among
 * mammals is about 0.57.
 * We derive an allometric function for the conductance
 * \f$C = x*M^{0.56}\f$ (in W/°C)
 * that contains the value from reindeer pelts.
 *
 * \f[
 * x_{summer} = 2.9 * 60^{-0.56} = 0.29 \\\\
 * x_{winter} = 0.8 * 60^{-0.56} = 0.08
 * \f]
 *
 * \param bodymass Adult body mass [kg/ind].
 * \param season Whether it’s summer or winter pelt.
 * \return Extrapolated whole-body conductance for an Arctic species
 * [W/°C].
 * \throw std::invalid_argument If `bodymass <= 0`.
 */
inline double get_conductance_cuyler_oeritsland_2004(const double bodymass,
                                                     const FurSeason season) {
  if (bodymass <= 0.0)
    throw std::invalid_argument(
        "Fauna::get_conductance_cuyler_oeritsland_2004() "
        "Parameter `bodymass` is <=0.");
  assert((season == FS_SUMMER) || (season == FS_WINTER));
  if (season == FS_SUMMER)
    return 0.29 * pow(bodymass, 0.57);
  else  // Winter:
    return 0.08 * pow(bodymass, 0.57);
}

/// Calculate additional energy requirements to keep body temperature.
/**
 * Please see \ref sec_thermoregulation for the formulas and
 * concepts.
 *
 * \param thermoneutral_rate Thermoneutral expenditure [MJ/ind/day].
 * \param conductance Whole-body thermal conductance of the animal [W/°C].
 * \param core_temperature Body core temperature [°C].
 * \param ambient_temperature Ambient air temperature [°C].
 * \return Additional expenditure for thermoregulation, i.e. heat loss
 * [MJ/ind/day].
 *
 * \throw std::invalid_argument If any parameter is out of range.
 */
double get_thermoregulatory_expenditure(const double thermoneutral_rate,
                                        const double conductance,
                                        const double core_temperature,
                                        const double ambient_temperature);
}  // namespace Fauna

#endif  // FAUNA_EXPENDITURE_COMPONENTS_H
