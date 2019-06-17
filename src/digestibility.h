//////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Models for digestibility of herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////
#ifndef HERBIV_DIGESTIBILITY_H
#define HERBIV_DIGESTIBILITY_H

#include <deque>
#include <map>

// Forward declarations of LPJ-GUESS classes
class Individual;

namespace Fauna {

	/// Abstract base class (interface) for calculating forage digestibility
	/**
	 * This is a function object and implementing the
	 * \ref sec_strategy design pattern.
	 */
	struct GetDigestibility{
		/// Get current forage digestibility for one plant individual.
		virtual double operator()(const Individual&)const=0;

		/// Virtual destructor.
		/** The virtual destructor is necessary so that the object is properly
		 * released from memory.*/
		virtual ~GetDigestibility(){}
	};

	/// Digestibility model using \ref PftParams::digestibility.
	/**
	 * Digestibility is a fixed value for each \ref Pft.
	 */
	class PftDigestibility : public GetDigestibility{
		public:
			/// Returns \ref PftParams::digestibility.
			virtual double operator()(const Individual&) const;
	};

	/// Grass digestibility as biomass-dependent live/dead mix of last month.
	/**
	 * \warning This model works only with grass.
	 *
	 * This digestibility model mimics Pachzelt et al. (2013)
	 * \cite pachzelt2013coupling.
	 * The average phenology over the last month
	 * (\ref Individual::get_average_phenology()) is interpreted as the
	 * proportion of live (green) grass available; the rest is dead
	 * (non-functional) grass.
	 * Dead grass has a fixed digestibility value of 0.4 while live grass
	 * digestibility \f$d_{Living}\f$ depends on the live grass density
	 * \f$V_{Living}\f$ (kg/m²) in the habitat.
	 *
	 * Crude protein fraction of live grass is calculated with a formula
	 * from van Wijngaarden (1985) \cite vanwijngaarden1985elephants.
	 * \f[
	 * CPC =  0.208790 * V_{Living}^{-0.1697}
	 * \f]
	 *
	 * Fractional digestibility \f$d_{Living}\f$ can be derived from crude
	 * protein content (fraction) as follows:
	 * \f[
	 * d_{Living} = 0.4605 + 1.4152 * CPC
	 * \f]
	 * This formula is cited by
	 * Prins (1996) \cite prins1996ecology and
	 * Smallegange & Brunsting (2002) \cite smallegange2002food,
	 * but originates from
	 * Bredon & Wilson (1963) \cite bredon1963chemical, who derived it
	 * from zebu cattle.
	 *
	 * The total digestibility is then calculated as live and dead grass
	 * mixed.
	 * \note This model differs from Pachzelt et al. (2013) in that
	 * the proportion of live to dead grass does not depend on the animals
	 * weight.
	 * Moreover, phenology is averaged over the last 30 days, but for grass
	 * density, the daily value of \ref Individual::cmass_leaf is used
	 * (multiplied by 2 to obtain dry matter).
	 */
	class DigestibilityPachzelt2013 : public GetDigestibility{
		public:
			/// Digestibility value of dead (non-functional) grass.
			static const double DIG_DEAD_GRASS;

			/// Maximum live grass digestibility.
			static const double DIG_LIVE_GRASS;

			/// \copydoc GetDigestibility::operator()()
			/**
			 * \throw std::invalid_argument If the plant individual is not
			 * a grass.
			 * \return Digestibility as described in class documentation.
			 * If \ref Individual::cmass_leaf is `<=0.0`, \ref DIG_DEAD_GRASS
			 * is returned.
			 */
			virtual double operator()(const Individual&) const;
	};

	/// Digestibility model using daily NPP to weigh in fresh & old forage.
	/**
	 * This model assumes that there is a linear decrease of forage quality
	 * over time from the day of production, starting with a digestibility
	 * of “fresh” biomass (\ref PftParams::digestibility) down to a
	 * digestibility of “dead” biomass (\ref PftParams::digestibility_dead).
	 * This process of senescence takes a certain amount of days
	 * (“attrition period”).
	 *
	 * Each plant individual is thought to be composed of fresher and
	 * older forage. The proportions are defined by a record of daily NPP
	 * (\ref Individual::dnpp_record). Even though not all net primary
	 * production is allocated into edible plant compartments, it is valid
	 * to use daily NPP for weighing the proportions when one broadly
	 * assumes a constant fraction of NPP allocated to edible plant material.
	 *
	 * In order to accurately reflect how much older forage is present and
	 * remove any old biomass from the weighted average that is not there
	 * anymore, the values in \ref Individual::dnpp_record need to be
	 * reduced proportionally (see \ref Individual::update_dnpp_record()).
	 * This way, fresh forage after fire, feeding, or disturbance weighs in
	 * more, increasing average digestbility.
	 * \see \ref sec_herbiv_digestibility
	 */
	class DigestibilityFromNPP : public GetDigestibility{
		public:
			/// Number of days for forage to go from “fresh” to “dead” state.
			static const int ATTRITION_PERIOD = 365;

			/// \copydoc GetDigestibility::operator()()
			/**
			 * \return Digestibility as described in class documentation.
			 * \see \ref get_digestibility_from_dnpp()
			 */
			virtual double operator()(const Individual&) const;

			// TODO
			/// Calculate average digestibility from daily NPP record.
			/**
			 * \param weights The proportional weight of the fraction of biomass
			 * for each age (days). Each deque entry represents the biomass that
			 * has grown on the specific day in the past, counting back.
			 * `weights[0]` is forage grown today, `weights[1]` is forage from
			 * yesterday and so on.
			 * The digestibility of each portion is given by a linear decrease
			 * from “fresh” to “dead” over the time of \ref ATTRITION_PERIOD.
			 * \param dig_fresh Fractional digestbility of biomass from today.
			 * \param dig_dead Fractional digestbility of biomass from
			 * \ref ATTRITION_PERIOD days ago.
			 * \returns The weighted average digestibility.
			 * Zero if `weights.size() == 0`.
			 * \throw std::logic_error If `dig_dead` > `dig_fresh`.
			 * \throw std::invalid_argument If `dead_fresh` or `dig_dead` is not
			 * in interval [0,1].
			 * \throw std::invalid_argument If any entry in `weights` is negative.
			 */
			static double get_digestibility_from_dnpp(
					const std::deque<double>& weights,
					const double dig_fresh, const double dig_dead);
	};

}
#endif // HERBIV_DIGESTIBILITY_H

// REFERENCES
// Adrian Pachzelt, Anja Rammig, Steven Higgins & Thomas Hickler (2013). Coupling a physiological grazer population model with a generalized model for vegetation dynamics. Ecological Modelling, 263, 92 - 102.
// Bredon, R.M., Wilson, J., 1963. The chemical composition and nutritive value of grasses from semi-arid areas of Karamoja as related to ecology and types of soil. E. Afr. Agric. For. J. 29, 134–142.
// Prins, Herbert (1996). Ecology and behaviour of the African buffalo: social inequality and decision making. Springer Science \& Business Media
// Smallegange, I. M., & Brunsting, A. M. (2002). Food supply and demand, a simulation model of the functional response of grazing ruminants. Ecological modelling, 149(1), 179-192.
// van Wijngaarden, Willem (1985). Elephants-trees-grass-grazers. Relationships between climate, soils, vegetation and large herbivores in a semi-arid savanna ecosystem (Tsavo, Kenya). ITC
