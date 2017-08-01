///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Reproduction of herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_REPRODUCTION_H
#define HERBIV_REPRODUCTION_H

namespace Fauna{

	/// Reproduction model following Illius & O’Connor (2000)
	/**
	 *
	 * The formula is derived from the model by 
	 * Illius & O’Connor (2000) \cite illius_resource_2000.
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
	 * by Pachzelt et al. (2013) \cite pachzelt_coupling_2013 and 
	 * Pachzelt et al. (2015) \cite pachzelt_potential_2015
	 * \todo How does I&O determine the month?
	 */
	class ReproductionIllius2000{
		public:
			/// Constructor.
			/**
			 * \param breeding_season_start The day of the year (0=Jan 1st)
			 * at which the breeding season starts.
			 * \param breeding_season_length Length of the breeding season
			 * in days [1--365].
			 * \param max_annual_increase Highest possible (i.e. under
			 * optimal nutrition) offspring count of one female on 
			 * average. 1.0 means, a female begets one child every
			 * year.
			 * \throw std::invalid_argument if a parameter is out
			 * of range.
			 */
			ReproductionIllius2000( 
					const int breeding_season_start,
					const int breeding_season_length,
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
			double get_offspring_density(
					const int day_of_year,
					const double body_condition)const;
		private:
			int breeding_start; // day of year (0=Jan 1st)
			int breeding_length;
			double max_annual_increase;
	};
}

#endif // HERBIV_REPRODUCTION_H
