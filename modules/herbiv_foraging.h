///////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Foraging models and digestive contraints of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FORAGING_H
#define HERBIV_FORAGING_H

#include <vector>
#include <string>                 // for forage type names
#include "herbiv_hft.h"           // for DigestionType
#include "herbiv_forageclasses.h" // for ForageEnergy
#include "herbiv_utils.h"         // for Sex

namespace Fauna{
	/// Function object to calculate forage demands for a herbivore.
	/**
	 * \see HerbivoreInterface::get_forage_demands()
	 */
	class GetForageDemands{
		public:
			/// Constructor.
			/**
			 * \param hft Herbivore functional type.
			 * \param sex The sex of the herbivore individual/cohort.
			 * \throw std::invalid_argument If `hft==NULL`.
			 */
			GetForageDemands(const Hft* hft, const Sex sex);

			/// Initialize foraging for another day.
			/**
			 * - Calculate the diet composition with different forage types.
			 * - Set \ref max_intake to the minimum of digestive and foraging
			 *   constraints.
			 * \param available_forage The forage in the habitat.
			 * \param day Current day of year, 0=Jan. 1st.
			 * \param bodymass Current live weight body mass [kg/ind].
			 * \param energy_content Net energy content of the available forage
			 * [MJ/kgDM].
			 * \throw std::invalid_argument If `day` not in [0,364] or if
			 * `bodymass<=0` or if `bodymass` greater than adult body mass.
			 */
			void init_today(
					const int day,
					const HabitatForage& available_forage,
					const ForageEnergyContent& energy_content,
					const double bodymass);

			/// Whether the given day has been initialized with \ref init_today().
			bool is_day_initialized(const int day)const{
				return day == this->today;
			}

			/// Calculate current forage demands.
			/**
			 * Call this only after \ref init_today().
			 * \throw std::logic_error If \ref init_today() hasn’t been called
			 * yet.
			 * \throw std::invalid_argument If `energy_needs < 0.0`.
			 */
			ForageMass operator()(const double energy_needs);

		private:
	
			/// Adult herbivore body mass [kg/ind].
			double get_bodymass_adult() const{
				if (sex == SEX_MALE)
					return get_hft().bodymass_male; 
				else
					return get_hft().bodymass_female;
			}

			/// The herbivore functional type.
			const Hft& get_hft()const{ 
				assert( hft != NULL); 
				return *hft; 
			}

			/// Get energy-wise preferences for forage types.
			/** 
			 * To what fractions the different forage
			 * types are eaten (in sum the fractions must be 1.0).
			 *
			 * This is the ad-libidum diet according to the preferences of the 
			 * HFT.
			 * The fractions refer to energy, not mass.
			 * The composition is *set*, i.e. that the demanded forage will 
			 * be put together accordingly.
			 * In case of forage shortage in the habitat,
			 * there is the chance to switch to other forage types when the 
			 * demands are queried again in the same day.
			 * (⇒ see Fauna::DistributeForage).
			 * \return Energy fractions of forage types composing current diet;
			 * the sum is 1.0.
			 * \throw std::logic_error If the \ref Hft::diet_composer is not
			 * implemented.
			 */ 
			ForageFraction get_diet_composition()const;

			/// Maximum forage [kgDM/ind/day] that could be potentially digested.
			/**
			 * The algorithm selected by \ref Hft::digestive_limit is employed.
			 *
			 * \return Maximum digestible dry matter today with given forage
			 * composition [kgDM/ind/day].
			 * \throw std::logic_error If the \ref Hft::digestive_limit is not
			 * implemented.
			 */
			ForageMass get_max_digestion()const;

			/// Get the amount of forage the herbivore would be able to 
      /// harvest [kgDM/day/ind].
			/**
			 * The relative amount of each forage type is prescribed, and 
			 * the absolute mass that the herbivore could potentially ingest
			 * is returned. This does not consider digestive limits or actual
			 * metabolic needs (“hunger”), but only considers the harvesting
			 * efficiency of the herbivore this day.
			 *
			 * \return Maximum potentially harvested dry matter mass of 
			 * each forage type [kgDM/day/ind].
			 * \throw std::logic_error If one of \ref Hft::foraging_limits is 
       * not implemented.
			 */
			ForageMass get_max_foraging()const;

			/// Current day of the year, as set in \ref init_today().
			/** \throw std::logic_error If current day not yet set by an
			 * initial call to \ref init_today(). */
			int get_today()const;
	private:
			/// @{ \name Constants
			Hft const * hft;
			Sex sex;
			/** @} */ // constants

	private: 
			/// @{ \name State Variables
			HabitatForage available_forage;
			double bodymass;                    // [kg/ind]
			ForageFraction diet_composition;    // [frac.] sum = 1.0
			Digestibility digestibility;        // [frac.]
			ForageEnergyContent energy_content; // [MJ/kgDM]
			double energy_needs;                // [MJ/ind]
			ForageMass max_intake;              // [kgDM/ind/day]
			int today;                          // 0 = January 1st
			/** @} */ // State Variables
	};

	/// Ingestion rate as Holling’s Type II functional response.
	/**
	 * The functional response of grazers (feeding rate depending on
	 * grass biomass) is usually described as a “Type II” of the types
	 * formulated by Crawford Stanley Holling (1959)
	 * \cite holling_components_1959.
	 *
	 * Intake rate \f$I\f$ is expressed as a hyperbolically saturating
	 * (‘Michalig-Menten’) function:
	 * \f[
	 * I = I_{max} * \frac{V}{V_{1/2} + V}
	 * \f]
	 * \f$I_{max}\f$ is the maximum intake rate: the asymptote of the
	 * function curve. \f$V\f$ (gDM/m²) is the dry-matter forage (grass)
	 * density and \f$V_{1/2}\f$ (gDM/m²) is a species-specific 
	 * half-saturation constant at which the herbivore reaches half of its
	 * maximum ingestion rate.
	 * This model is primarily empirical and does not represent any 
	 * underlying mechanisms. The parameter \f$V_{1/2}\f$ does not generally
	 * scale with body mass and usually needs to be derived from field
	 * observations of the particular species.
	 * \note Illius & O’Connor (2000) and Pachzelt et al. (2013) also call 
	 * \f$V_{1/2}\f$ “beta” (β).
	 *
	 * \see \ref FL_ILLIUS_OCONNOR_2000
	 */
	class HalfMaxIntake{
		public:
			/// Constructor
			/**
			 * \param half_max_density The forage density at which
			 * the intake rate of a herbivore is half of its maximum.
			 * The unit can be freely chosen, but must correspond to the 
			 * parameter `density` in \ref get_intake_rate().
			 * \param max_intake Maximum intake rate; the asymptote of the
			 * functional response curve. The unit can be freely chosen as
			 * mass or energy per day or per minute etc.
			 * \throw std::invalid_argument If either `half_max_density` or
			 * `max_intake` is not a positive number.
			 */
			HalfMaxIntake(
					const double half_max_density,
					const double max_intake);

			/// Get maximum daily net energy intake [unit as `max_intake`].
			/**
			 * \param density Grass sward density [unit as `half_max_density`].
			 * \return Realized feeding rate [unit as `max_intake`].
			 * \throw std::invalid_argument If `density` is negative.
			 */
			double get_intake_rate(const double density)const;

		private:
			const double half_max_density;
			const double max_intake;
	};

	/// Digestion-limited intake function after Illius & Gordon (1992)
	/**
	 * The model of digestive passage rates by Illius & Gordon 
	 * (1992)\cite illius_modelling_1992 constrains maximum daily
	 * energy intake \f$I_{dig[MJ/day]}\f$ by gut size and retention 
	 * time.
	 *
	 * \f[
	 * I_{dig[MJ/day]} = i * e^{jd} * M_{ad_{[kg]}}^{(ke^d+0.73)} * u_g
	 * \f]
	 * - e: Euler number
	 * - d: proportional digestibility
	 * - \f$M_{ad}\f$: Adult body mass in kg
	 * - \f$u_g = (M/M_{ad})^{0.75}\f$ is a scaling factor for
	 *   gut capacity, introduced by Illius & Gordon (1999)
	 *   \cite illius_scaling_1999
	 * - %Parameters i, j, and k are derived from regression analysis 
	 *   with 12 mammalian herbivores (0.05--547 kg) and are specific
	 *   to hindguts and ruminants (Shipley et al. 1999)
	 *   \cite shipley_predicting_1999.
	 *
	 * Grass forage:
	 * |     | Hindgut | Ruminant |
	 * |-----|---------|----------|
	 * | i   | 0.108   | 0.034    |
	 * | j   | 3.284   | 3.565    |
	 * | k   | 0.080   | 0.077    |
	 *
	 * \note This currently only works for pure grass diet.
	 * \see \ref DL_ILLIUS_GORDON_1992
	 */
	class GetDigestiveLimitIlliusGordon1992{
		public:
			/// Constructor
			/**
			 * \param bodymass_adult Body mass [kg] at physical maturity.
			 * \param digestion_type The herbivore’s digestion type.
			 * \throw std::invalid_argument If `bodymass_adult<=0.0`.
			 */
			GetDigestiveLimitIlliusGordon1992( 
					const double bodymass_adult,
					const DigestionType digestion_type);

			/// Get digestion-limited daily net energy intake [MJ/day/ind]
			/**
			 * \param bodymass current body mass [kg/ind]
			 * \param digestibility proportional digestibility of the
			 * forage
			 * \return maximum energy intake [MJ/day/ind]
			 * \throw std::invalid_argument If `bodymass<=0.0`
			 * \throw std::logic_error If `bodymass > bodymass_adult`.
			 * \throw std::logic_error If `digestion_type` not implemented.
			 * \throw std::logic_error If not all forage types are
			 * implemented.
			 */
			const ForageEnergy operator()(
					const double bodymass,
					const Digestibility& digestibility)const;
		private:
			double bodymass_adult;        // constant
			DigestionType digestion_type; // constant
	};

	/// Get maximum intake for each forage type limited by total mass.
	/**
	 * \param mj_proportions Diet composition in energy proportions [MJ/MJ].
	 * \param mj_per_kg Energy content of forage [MJ/kgDM].
	 * \param kg_total The maximum total intake [kgDM/day].
	 * \return The maximum intake for each forage type [kgDM/day] while
	 * retaining the given relative energy proportions.
	 * \throw std::invalid_argument If `kg_total < 0.0`.
	 * \see \ref DL_BODYMASS_FRACTION
	 */
	ForageMass get_max_intake_as_total_mass(
			const ForageFraction& mj_proportions,
			const ForageEnergyContent& mj_per_kg,
			const double kg_total);
}

#endif //HERBIV_FORAGING_H

// Adrian Pachzelt, Anja Rammig, Steven Higgins & Thomas Hickler (2013). Coupling a physiological grazer population model with a generalized model for vegetation dynamics. Ecological Modelling, 263, 92 - 102.
// Holling, C. S. (1959). The Components of Predation as Revealed by a Study of Small-Mammal Predation of the European Pine Sawfly. The Canadian Entomologist, 91, 293-320.
// Illius, A. W. & Gordon, I. J. (1992). Modelling the nutritional ecology of ungulate herbivores: evolution of body size and competitive interactions. Oecologia, 89, 428-434.
// Illius, A. W. & O'Connor, T. G. (2000). Resource Heterogeneity and Ungulate Population Dynamics. Oikos, 89, 283-294.
