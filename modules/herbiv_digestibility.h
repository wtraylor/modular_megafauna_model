//////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Models for digestibility of herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////
#ifndef HERBIV_DIGESTIBILITY_H
#define HERBIV_DIGESTIBILITY_H

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
   * \cite pachzelt_coupling_2013.
   * The average phenology over the last month 
   * (\ref Individual::get_average_phenology()) is interpreted as the
   * proportion of live (green) grass available; the rest is dead 
   * (non-functional) grass.
   * Dead grass has a fixed digestibility value of 0.4 while live grass
   * digestibility \f$d_{Living}\f$ depends on the live grass density 
   * \f$V_{Living}\f$ (kg/m²) in the habitat.
   *
   * Crude protein fraction of live grass is calculated with a formula
   * from van Wijngaarden (1985) \cite vanwijngaarden_elephants_1985.
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
   * Prins (1996) \cite prins_ecology_1996 and
   * Smallegange & Brunsting (2002) \cite smallegange_food_2002,
   * but originates from
   * Bredon & Wilson (1963) \cite bredon_chemical_1963, who derived it
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

}
#endif // HERBIV_DIGESTIBILITY_H

// REFERENCES
// Adrian Pachzelt, Anja Rammig, Steven Higgins & Thomas Hickler (2013). Coupling a physiological grazer population model with a generalized model for vegetation dynamics. Ecological Modelling, 263, 92 - 102.
// Bredon, R.M., Wilson, J., 1963. The chemical composition and nutritive value of grasses from semi-arid areas of Karamoja as related to ecology and types of soil. E. Afr. Agric. For. J. 29, 134–142.
// Prins, Herbert (1996). Ecology and behaviour of the African buffalo: social inequality and decision making. Springer Science \& Business Media
// Smallegange, I. M., & Brunsting, A. M. (2002). Food supply and demand, a simulation model of the functional response of grazing ruminants. Ecological modelling, 149(1), 179-192.
// van Wijngaarden, Willem (1985). Elephants-trees-grass-grazers. Relationships between climate, soils, vegetation and large herbivores in a semi-arid savanna ecosystem (Tsavo, Kenya). ITC
