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

#include <assert.h>
#include <cmath> // for pow()
#include <stdexcept>

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

			/// Set the proportional fat reserves.
			/**
			 * \param body_condition Current fat mass divided by potential
			 * maximum fat mass.
			 * \throw std::invalid_argument If `body_condition` not in interval
			 * [0,1]
			 */
			void force_body_condition(const double body_condition);

			/// Current energy needs [MJ/ind]
			double get_energy_needs()const{return energy_needs;}

			/// Current fat mass [kg/ind].
			double get_fatmass()const{return fatmass;}

			/// Get the maximum energy [MJ/ind/day] that could be anabolized in a day
			double get_max_anabolism_per_day()const;

			/// Maximum fat mass [kg/ind].
			double get_max_fatmass()const{return max_fatmass;}

			/// Merge this object with another one by building weighted averages.
			/**
			 * \param other The other object.
			 * \param this_weight Weight for this object.
			 * \param other_weight Weight for the other object.
			 */
			void merge(const FatmassEnergyBudget& other,
					const double this_weight, const double other_weight);

			/// Update budget and fat mass by receiving energy (through feeding)
			/** 
			 * If `energy` exceeds current energy needs, the surplus is stored
			 * as fat (anabolism).
			 * \param energy Input energy [MJ/ind].
			 * \throw std::invalid_argument If `energy<0.0`.
			 * \throw std::logic_error if `energy` exceeds current
			 * energy needs and maximum anabolism.
			 */
			void metabolize_energy(double energy);

			/// Set the maximum allowed fat mass [kg/ind] and fat gain [kg/ind/day].
			/** 
			 * \param max_fatmass Current maximum fat mass [kg/ind].
			 * \param max_gain Maximum fat mass gain [kg/ind/day]. A value of 
			 * zero indicates no limit.
			 * \throw std::logic_error if `max_fatmass` is smaller than 
			 * current fat mass 
			 * \throw std::invalid_argument If `max_fatmass<=0.0` or 
			 * if `max_gain < 0`.*/
			void set_max_fatmass(const double max_fatmass, const double max_gain);
		private:
			double energy_needs;     // MJ/ind
			double fatmass;          // kg/ind
			double max_fatmass;      // kg/ind
			double max_fatmass_gain; // kg/ind/day

			/// Metabolic coefficient for anabolism [MJ/kg] (Blaxter 1989)\cite blaxter1989energy
			static const double FACTOR_ANABOLISM;
			/// Metabolism coefficient for catabolism [MJ/kg] (Blaxter 1989)\cite blaxter1989energy
			static const double FACTOR_CATABOLISM;
	};

	/// Energy expenditure [MJ/ind/day] based on cattle from Taylor et al. (1981)
	/**
	 * Taylor et al. (1981)\cite taylor1981genetic
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

	/// Convert Watts (=J/s) to MJ/day.
	inline double watts_to_MJ_per_day(const double W){
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
	inline double get_conductance_bradley_deavers_1980(const double bodymass){
		if (bodymass <= 0.0)
			throw std::invalid_argument("Fauna::get_conductance_bradley_deavers_1980() "
					"Parameter `bodymass` is <=0.");
		return 0.224 * pow(bodymass, 0.574);
	}

	/// Selector for winter or summer pelt.
	enum FurSeason{
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
			const FurSeason season){
		if (bodymass <= 0.0)
			throw std::invalid_argument("Fauna::get_conductance_cuyler_oeritsland_2004() "
					"Parameter `bodymass` is <=0.");
		assert( (season == FS_SUMMER) || (season == FS_WINTER) );
		if (season == FS_SUMMER)
			return 0.29 * pow(bodymass, 0.57);
		else // Winter:
			return 0.08 * pow(bodymass, 0.57);
	}

	/// Calculate additional energy requirements to keep body temperature.
	/**
	 * Please see \ref sec_herbiv_thermoregulation for the formulas and 
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
	double get_thermoregulatory_expenditure(
			const double thermoneutral_rate,
			const double conductance,
			const double core_temperature,
			const double ambient_temperature);
}

#endif // HERBIV_ENERGETICS_H 

// References

// S.Robert Bradley and Daniel R Deavers. A re-examination of the relationship between thermal conductance and body weight in mammals. Comparative Biochemistry and Physiology Part A: Physiology, 65(4):465–476, 1980.

// Hudson, Robert J. and Robert G. White (1985). Bioenergetics of wild herbivores. CRC press.

// Peters, Robert Henry (1983). The ecological implications of body size. 1st publ. Cambridge studies in ecology. - Cambridge : Univ. Press, 1982 2. Cambridge [u.a.]: Cambridge Univ.Pr. xii+329.

// Soppela, Päivi, Mauri Nieminen, and Jouni Timisjärvi (June 1, 1986). “吀ermoregulation in reindeer”. In: Rangifer 6.2, pp. 273–278. doi: 10.7557/2.6.2.659.
