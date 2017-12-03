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
	 * > “Mean body fat is assumed to be normally distributed with 
	 * > standard deviation σ (sigma). 
	 * > Mortality occurs in the proportion of animals in the tail 
	 * > of this distribution that projects below zero.”
	 *
	 * \note This only makes sense for herbivore cohorts.
	 * \warning The cumulative effects of this algorithm change with
	 * simulation interval. That means that, all together, more
	 * animals have died if the algorithm was applied 30 times *daily*
	 * than if it had been applied 1 time *monthly.*
	 * Since the model of Illius & O’Connor (2000) works on a monthly basis,
	 * this class should also be called monthly.
	 */
	class GetStarvationMortalityIllius2000{
		public:
			/// Constructor
			/**
			 * \param fat_standard_deviation Standard deviation of fat 
			 * mass in a herbivore cohort as fraction of potential
			 * maximum fat mass.
			 * The default standard deviation is 12.5\%, taken from
			 * Illius & O’Connor (2000)\cite illius_resource_2000,
			 * who are citing Ledger (1968)\cite ledger_body_1968.
			 * \throw std::invalid_argument If `fat_standard_deviation`
			 * not in interval [0,1].
			 */
			GetStarvationMortalityIllius2000(
					const double fat_standard_deviation = .0125);

			/// Get today’s mortality.
			/**
			 * \param body_condition Current fat mass divided by
			 * potential maximum fat mass [kg/kg].
			 * \return Fractional [0,1] daily mortality due to starvation.
			 * \throw std::invalid_argument If `body_condition` is not
			 * in interval [0,1].
			 */
			double operator()(const double body_condition)const;
		private:
			double fat_standard_deviation;
			/// Function Φ (phi)
			/** Implementation by John D. Cook:
			 * https://www.johndcook.com/blog/cpp_phi/*/
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
