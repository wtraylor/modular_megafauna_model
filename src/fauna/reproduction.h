//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Reproduction of herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
//////////////////////////////////////////////////////////////////////////
#ifndef REPRODUCTION_H
#define REPRODUCTION_H

namespace Fauna {

/// Helper class for reproduction within a breeding season.
class BreedingSeason {
 public:
  /// Constructor.
  /**
   * \param breeding_season_start The day of the year (0=Jan 1st)
   * at which the breeding season starts.
   * \param breeding_season_length Length of the breeding season
   * in days [1--365].
   */
  BreedingSeason(const int breeding_season_start,
                 const int breeding_season_length);

  /// Whether given day (0=Jan 1st) is in the breeding season.
  /**
   * \throw std::invalid_argument If `day` is not in [0,364].
   */
  bool is_in_season(const int day) const;

  /// Convert an annual reproduction rate to a daily one in season.
  double annual_to_daily_rate(const double annual) const;

 private:
  int start;   // day of year (0=Jan 1st)
  int length;  // number of days
};

/// Reproduction model following Illius & O’Connor (2000)
/**
 * The formula is derived from the model by
 * Illius & O’Connor (2000) \cite illius2000resource.
 * Their variable names are used here:
 * \f[
 * B_{annual}[ind/year]
 * = \frac{k}{1 + e^{-b(\frac{F}{F_{max}} - c)}}
 * \f]
 * - B_annual: offspring count per year for one mature female
 * - k: maximum annual offspring count of one female
 * - F: current fat mass
 * - F_max: maximum fat mass
 * - b and c are parameters controlling the effect of body
 *   condition (F/F_max) on the reproductive rate and are set
 *   in Illius and O’Connor (2000) to 15 and 0.3, respectively
 *   (unfortunately without explanation).
 * - 50% of adults will breed when F/F_max=0.3
 * - 95% will breed when F/F_max=0.5
 *
 * \image html reproduction_illiusoconnor2000.png
 *
 * The annual rate is then converted to a daily rate over the
 * breeding season length:
 * \f[
 * B_{daily}[ind/day]
 * = \frac{B_{annual}}{L}
 * \f]
 * - L: length of breeding season [days]
 *
 *
 * \note This reproduction model principle also used
 * by Pachzelt et al. (2013) \cite pachzelt2013coupling and
 * Pachzelt et al. (2015) \cite pachzelt2015potential
 * \see \ref Fauna::ReproductionModel
 * \todo How does I&O determine the month?
 */
class ReprIlliusOconnor2000 {
 public:
  /// Constructor.
  /**
   * \param max_annual_increase Highest possible (i.e. under
   * optimal nutrition) offspring count of one female on
   * average. A value of 1.0 means, a female begets one child every
   * year.
   * \param breeding_season When parturition occurs.
   * \throw std::invalid_argument If `max_annual_increase` is negative.
   */
  ReprIlliusOconnor2000(BreedingSeason breeding_season,
                        const double max_annual_increase);

  /// Get the amount of offspring for one day in the year.
  /**
   * \param day_of_year Day of year (0=Jan 1st).
   * \param body_condition Current fat mass divided by
   * potential maximum fat mass [kg/kg].
   * \return The average number of children a female gives
   * birth to at given day.
   * \throw std::invalid_argument If `day_of_year` not in [0,364]
   * or `body_condition` not in [0,1].
   */
  double get_offspring_density(const int day_of_year,
                               const double body_condition) const;

 private:
  BreedingSeason breeding_season;  // const
  double max_annual_increase;      // const
};

/// Use a constant annual increase rate for herbivore reproduction.
/**
 * \see \ref Fauna::ReproductionModel
 */
class ReproductionConstMax {
 public:
  /// Constructor.
  /**
   * \param annual_increase Constant annual offspring count for
   * one female.
   * \param breeding_season When parturition occurs.
   * \throw std::invalid_argument If `annual_increase` is negative.
   */
  ReproductionConstMax(BreedingSeason breeding_season,
                       const double annual_increase);

  /// Get the amount of offspring for one day in the year.
  /**
   * \param day_of_year Day of year (0=Jan 1st).
   * \return The average number of children a female gives
   * birth to at given day.
   * \throw std::invalid_argument If `day_of_year` not in [0,364].
   */
  double get_offspring_density(const int day_of_year) const;

 private:
  BreedingSeason breeding_season;  // const
  double annual_increase;          // const
};

/// Reproduction rate increases linearly with fat reserves up to maximum.
/**
 */
class ReproductionLinear {
 public:
  /// Constructor
  /**
   * \param max_annual_increase Maximum annual offspring count for
   * one female under full fat reserves.
   * \param breeding_season When parturition occurs.
   * \throw std::invalid_argument If `max_annual_increase` is negative.
   */
  ReproductionLinear(BreedingSeason breeding_season,
                     const double max_annual_increase);

  /// Get the amount of offspring for one day in the year.
  /**
   * \param day_of_year Day of year (0=Jan 1st).
   * \param body_condition Current fat mass divided by
   * potential maximum fat mass [kg/kg].
   * \return The average number of children a female gives birth to at
   * given day.
   * \throw std::invalid_argument If `day_of_year` not in [0,364]
   * or `body_condition` not in [0,1].
   */
  double get_offspring_density(const int day_of_year,
                               const double body_condition) const;

 private:
  BreedingSeason breeding_season;  // const
  double max_annual_increase;      // const
};
}  // namespace Fauna

#endif  // REPRODUCTION_H
