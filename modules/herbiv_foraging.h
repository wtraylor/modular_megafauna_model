///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Foraging models of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FORAGING_H
#define HERBIV_FORAGING_H

#include <vector>
#include <string>                 // for forage type names
#include "herbiv_hft.h"           // for DigestionType
#include "herbiv_forageclasses.h" // for ForageEnergy

namespace Fauna{

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
	 */
	class GetDigestiveLimitIllius1992{
		public:
			/// Constructor
			/**
			 * \param bodymass_adult Body mass [kg] at physical maturity.
			 * \param digestion_type The herbivore’s digestion type.
			 * \throw std::invalid_argument If `bodymass_adult<=0.0`.
			 */
			GetDigestiveLimitIllius1992( 
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
}

#endif //HERBIV_FORAGING_H

// Adrian Pachzelt, Anja Rammig, Steven Higgins & Thomas Hickler (2013). Coupling a physiological grazer population model with a generalized model for vegetation dynamics. Ecological Modelling, 263, 92 - 102.
// Holling, C. S. (1959). The Components of Predation as Revealed by a Study of Small-Mammal Predation of the European Pine Sawfly. The Canadian Entomologist, 91, 293-320.
// Illius, A. W. & Gordon, I. J. (1992). Modelling the nutritional ecology of ungulate herbivores: evolution of body size and competitive interactions. Oecologia, 89, 428-434.
// Illius, A. W. & O'Connor, T. G. (2000). Resource Heterogeneity and Ungulate Population Dynamics. Oikos, 89, 283-294.
