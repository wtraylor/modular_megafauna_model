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

	// TODO
	class GetHalfMaxForagingLimit{
		public:
			/// Constructor
			/**
			 * \param half_max_density The forage density [g/m²] at which
			 * the intake rate of a herbivore is half of its maximum.
			 */
			GetHalfMaxForagingLimit(
					const double half_max_density):
				half_max_density(half_max_density){}


		private:
			const double half_max_density;
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
