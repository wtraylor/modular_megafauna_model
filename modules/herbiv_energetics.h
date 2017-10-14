///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Energy-related classes and functions for herbivores
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
/// \see \ref sec_herbiv_energetics
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_ENERGETICS_H
#define HERBIV_ENERGETICS_H

#include <cmath> // for pow()

namespace Fauna{


	/// A herbivore’s energy budget with fat reserves
	/**
	 * Terminology:
	 * - Anabolism  = build up fat mass
	 * - Catabolism = burn fat mass 
	 * - Metabolism = us burn food energy directly and use surplus for
	 *   anabolism
	 */
	class FatmassEnergyBudget {
		public:
			/// Constructor
			/** 
			 * \param initial_fatmass Initial fat mass [kg/ind] 
			 * \param maximum_fatmass Maximum fat mass [kg/ind]
			 * \throw std::invalid_argument If one parameter is <=0.0
			 * \throw std::logic_error `initial_fatmass > maximum_fatmass`
			 */
			FatmassEnergyBudget(
					const double initial_fatmass,
					const double maximum_fatmass);

			/// Increase energy needs
			/** \param energy Additional energy needs [MJ/ind] 
			 * \throw std::invalid_argument if `energy<0.0` */
			void add_energy_needs(const double energy);

			/// Burn fat reserves and reduce unmet energy needs.
			void catabolize_fat();

			/// Current energy needs [MJ/ind]
			double get_energy_needs()const{return energy_needs;}

			/// Current fat mass [kg/ind].
			double get_fatmass()const{return fatmass;}

			/// Get the maximum energy [MJ/ind/day] that could be anabolized in a day
			double get_max_anabolism_per_day()const;

			/// Maximum fat mass [kg/ind].
			double get_max_fatmass()const{return max_fatmass;}

			/// Merge this object with another one by building weihted averages.
			/**
			 * \param other The other object.
			 * \param this_weight Weight for this object.
			 * \param other_weight Weight for the other object.
			 */
			void merge(const FatmassEnergyBudget& other,
					const double this_weight, const double other_weight);

			/// Update budget and fat mass by receiving energy (through feeding)
			/** 
			 * \param energy Input energy [MJ/ind].
			 * \throw std::invalid_argument If `energy<0.0`.
			 * \throw std::logic_error if `energy` exceeds current
			 * energy needs and maximum anabolism.
			 */
			void metabolize_energy(double energy);

			/// Set the maximum allowed fat mass [kg/ind]
			/** \throw std::logic_error if `maximum` is smaller than 
			 * current fat mass 
			 * \throw std::invalid_argument if `maximum<=0.0`*/
			void set_max_fatmass(const double maximum);
		private:
			double energy_needs;   // MJ/ind
			double fatmass;        // kg/ind
			double max_fatmass;    // kg/ind

			/// Metabolic coefficient for anabolism [MJ/kg] (Blaxter 1989)\cite blaxter_energy_1989
			static const double FACTOR_ANABOLISM;
			/// Metabolism coefficient for catabolism [MJ/kg] (Blaxter 1989)\cite blaxter_energy_1989
			static const double FACTOR_CATABOLISM;
	};

	/// Energy expenditure based on cattle from Taylor et al. (1981)
	/**
	 * Taylor et al. (1981)\cite taylor_genetic_1981
	 * Daily energy expenditure is given by
	 * \f[
	 * E [\frac{MJ},{day*ind}] = 0.4 * M * M_{ad}^{-0.27}
	 * \f]
	 * where M is current body mass [kg/ind] and \f$M_{ad}\f$ is
	 * adult body mass.
	 *
	 * \return energy expenditure in MJ for one herbivore individual 
	 * per day [MJ/day/ind]
	 */
	inline double get_expenditure_taylor_1981(
			const double current_bodymass,
			const double adult_bodymass){
		return 0.4 * current_bodymass * pow(adult_bodymass,-0.27);
	}
}

#endif // HERBIV_ENERGETICS_H 
