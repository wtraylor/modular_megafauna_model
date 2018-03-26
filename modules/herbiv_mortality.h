//////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivore mortality factor implementations.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
/////////////////////////////////////////////////////////////////
#ifndef HERBIV_MORTALITY_H
#define HERBIV_MORTALITY_H

#include "herbiv_utils.h" // for get_random_fraction()
#include <stdexcept>

namespace Fauna{

	/// Function object to calculate constant herbivore background mortality.
	class GetBackgroundMortality{
		public:
			/// Constructor
			/**
			 * \param annual_mortality_1st_year Fractional annual 
			 * mortality in first year of life.
			 * \param annual_mortality Fractional annual mortality in
			 * the rest of life.
			 * \throw std::invalid_argument If a mortality value is
			 * not in interval `[0.0,1.0)`.
			 */
			GetBackgroundMortality(
					const double annual_mortality_1st_year,
					const double annual_mortality);

			/// Get today’s background mortality.
			/** 
			 * \param age_days Current age in days (0=first day of life).
			 * \return Fractional daily mortality [0,1].
			 * \throw std::invalid_argument If `age_days<0`. 
			 */
			double operator()(const int age_days)const;
		private:
			double annual_mortality;
			double annual_mortality_1st_year;
	};

	/// Function object for herbivore death after given lifespan is reached.
	class GetSimpleLifespanMortality{
		public:
			/// Constructor
			/** \param lifespan_years Lifespan in years
			 * \throw std::invalid_argument If `lifespan_years<=0`.*/
			GetSimpleLifespanMortality(const int lifespan_years);

			/// Calculate mortality for today.
			/** \param age_days Current age of the herbivore in days 
			 * (0=first day of life).
			 * \throw std::invalid_argument If `age_days<0`.
			 * \return 0.0 if age<lifespan; 1.0 if age>=lifespan */
			double operator()(const int age_days)const{
				if (age_days<0)
					throw std::invalid_argument(
							"Fauna::GetSimpleLifespanMortality::operator()() "
							"age_days < 0");
				if (age_days >= lifespan_years*365)
					return 1.0; // all dead
				else
					return 0.0; // all survive
			}
		private:
			int lifespan_years;
	};

	/// Function object to calculate herbivore mortality after Illius & O’Connor (2000)
	/**
	 * After Illius & O'Connor (2000)\cite illius_resource_2000 :
   * <BLOCKQUOTE>
	 * “Mean body fat is assumed to be normally distributed with 
	 * standard deviation σ (sigma). 
	 * Mortality occurs in the proportion of animals in the tail 
	 * of this distribution that projects below zero.”
   * </BLOCKQUOTE>
   *
   * \image html herbiv_starvation_illius_oconnor_2000.png "Starvation mortality after Illius & O’Connor (2000). The red area indicates the dying part of the cohort. SD = standard deviation."
	 *
   * <H1> Shift Body Condition </H1>	 
   *
	 * \note The following extension to the mortality model is by Wolfgang 
	 * Pappa and *not* from Illius & O’Connor (2000).
	 *
	 * When the herbivores with a “negative body condition” die, the cohort
	 * mean would increase. To account for that, the switch 
	 * `shift_body_condition` can be turned on in the constructor.
	 * This will change the body condition `b` to the following new value
	 * `b_new`.
   *
	 * \f[
	 * b_{new} = b * \frac{2b + 2d}{2b + d}
	 * \f]
   *
	 * \f$d\f$ is the fraction that died.
	 *
	 * <H2> Idea </H2>
   *
	 * The mean body condition \f$b\f$ is thought to be given by the following
	 * weighted mean with two “tails” on either side of the normal
	 * distribution, one below zero and one symmetrically on the other side
	 * (at body condition \f$2b\f$).
   *
	 * \f[
	 * b = \frac{d * 0 + 2b * b + d * 2b}{d + 2b + d}
	 * \f]
   *
	 * When the fraction below zero is taken away, the weighted average
	 * changes accordingly:
   *
	 * \f[
	 * b_{new} = \frac{2b * b + d * 2b}{2b + d} = b * \frac{2b + 2d}{2b + d}
	 * \f]
   *
   * \image html herbiv_starvation_body_condition_shift.png "Shift in body condition after removing the dead fraction of the cohort. Standard deviation = 0.125"
	 *
	 * \note This class only makes sense for herbivore cohorts.
	 */
	class GetStarvationIlliusOConnor2000{
		public:
			/// Constructor
			/**
			 * \param fat_standard_deviation Standard deviation of fat 
			 * mass in a herbivore cohort as fraction of potential
			 * maximum fat mass.
			 * The default standard deviation is 12.5\%, taken from
			 * Illius & O’Connor (2000)\cite illius_resource_2000,
			 * who are citing Ledger (1968)\cite ledger_body_1968.
			 * \param shift_body_condition Whether to shift body condition up to
			 * compensate for dead herbivores (see class documentation).
			 * \throw std::invalid_argument If `fat_standard_deviation`
			 * not in interval [0,1].
			 */
			GetStarvationIlliusOConnor2000(
					const double fat_standard_deviation = .0125,
					const bool shift_body_condition = true);

			/// Get today’s mortality.
			/**
			 * \param[in] body_condition Current fat mass divided by
			 * potential maximum fat mass [kg/kg].
			 * \param[out] new_body_condition Updated body mean body condition
			 * in the cohort after starved individuals are ‘removed’. If
			 * `shift_body_condition` in the constructor is turned off, this
			 * equals `body_condition` without change.
			 * \return Fractional [0,1] daily mortality due to starvation.
			 * \throw std::invalid_argument If `body_condition` is not
			 * in interval [0,1].
			 */
			double operator()(const double body_condition, 
					double& new_body_condition)const;
		private:
			double fat_standard_deviation; // const
			bool shift_body_condition;

			/// Function Φ (phi)
			/** Implementation by John D. Cook:
			 * <https://www.johndcook.com/blog/cpp_phi/>*/
			static double cumulative_normal_distribution(double x);
	};

	/// Function object to calculate death at low body fat.
	/**
	 * Death simply occurs if proportional body fat drops below a 
	 * given threshold.
	 */
	class GetStarvationMortalityThreshold{
		public:
			/// Default minimum body fat threshold [kg/kg].
			static const double DEFAULT_MIN_BODYFAT;

			/// Constructor
			/**
			 * \param min_bodyfat Minimum body fat threshold [kg/kg].
			 * \throw std::invalid_argument If `min_bodyfat` not in [0,1).
			 */
			GetStarvationMortalityThreshold(const double min_bodyfat=DEFAULT_MIN_BODYFAT);

			/// Get daily mortality.
			/**
			 * \return 0.0 if `bodyfat<min_bodyfat`, else 1.0
			 * \throw std::invalid_argument If `bodyfat` not in [0,1).
			 */
			double operator()(const double bodyfat)const;
		private:
			double min_bodyfat;
	};
}

#endif // HERBIV_MORTALITY_H

// REFERENCES
// Illius, A. W., and T. G. O’Connor. 2000. “Resource Heterogeneity and Ungulate Population Dynamics.” Oikos 89 (2). Munksgaard:283–94. https://doi.org/10.1034/j.1600-0706.2000.890209.x.
// Ledger, H. P. 1968. “Body Composition as a Basis for a Comparative Study of Some East African Mammals.” In Symp. Zool. Soc. Lond, 21:289–310.
