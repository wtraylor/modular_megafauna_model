//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Energy content of herbivore forage.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
//////////////////////////////////////////////////////////////////////////
#ifndef FORAGEENERGY_H
#define FORAGEENERGY_H

#include "forageclasses.h"  // for ForageType
#include "hft.h"            // for DigestionType

namespace Fauna {

/// Interface for strategy to get net energy content of forage.
/** \see \ref sec_strategy */
class GetNetEnergyContentInterface {
 public:
  /// Get net energy content of the forage [MJ/kgDM]
  /**
   * \param digestibility Proportional digestibility.
   * \return Net energy content [MJ/kgDM].
   */
  virtual ForageEnergyContent operator()(
      const Digestibility& digestibility) const {
    ForageEnergyContent result;
    // loop through all forage types and call abstract method.
    for (Digestibility::const_iterator itr = digestibility.begin();
         itr != digestibility.end(); itr++) {
      const double NE = get_per_forage_type(itr->first, itr->second);
      result.set(itr->first, NE);
    }
    return result;
  }

  /// Virtual destructor.
  /** The virtual destructor is necessary so that the object is properly
   * released from memory.*/
  virtual ~GetNetEnergyContentInterface() {}

 protected:
  /// Get net energy content for one forage type [MJ/kgDM]
  /**
   * \param forage_type The type of forage, guaranteed to be
   * not \ref FT_INEDIBLE.
   * \param digestibility Proportional digestibility,
   * guaranteed to be in [0,1].
   * \return Net energy content [MJ/kgDM].
   */
  virtual double get_per_forage_type(const ForageType forage_type,
                                     const double digestibility) const = 0;
};

/// Default net energy content algorithm
/**
 * Multiply the respective dry matter biomass with the
 * corresponding NE content to obtain the amount of metabolizable
 * energy a herbivore can get out of the forage.
 *
 * Formula for ruminants given by Illius & Gordon (1992, p. 148)
 * \cite illius1992modelling
 * citing ARC (1980)
 * \cite agricultural_research_council1980nutrient
 * \f[Net Energy [MJ/kgDM] =  ME * (0.503\frac{MJ}{kgDM} + 0.019 * ME) * e\f]
 * - ME = metabolizable energy of dry matter [MJ/kgDM]
 * - e = digestion efficiency factor; for ruminants always e=1.0
 *
 * Metabolizable energy content ME is calculated by
 * multiplying digestibility with a forage-specific coefficient.
 */
class GetNetEnergyContentDefault : public GetNetEnergyContentInterface {
 public:
  /// Constructor
  GetNetEnergyContentDefault(const DigestionType digestion_type)
      : digestion_efficiency(digestion_type == DT_RUMINANT
                                 ? 1.0
                                 : DIGESTION_EFFICIENCY_HINDGUTS) {}

  /// Metabolizable energy coefficient of grass [MJ/kgDM]
  /**
   * Givens et al. (1989, p. 39)\cite givens1989digestibility :
   *
   * > “In the absence of energetic data, it has been common
   * > to calculate ME from DOMD content.
   * > MAFF et al. (1984) \cite maff1984energy
   * > stated that for a wide range of feedstuffs
   * > ME may be calculated as 0.015*DOMD.
   * > This is based on the assumption that the GE of digested
   * > OM is 19.0 MJ/kg together with a ME/DE ratio of 0.81.”
   *
   * - ME = Metabolizable Energy [MJ/kg]
   * - DE = Digestible Energy [MJ/kg]
   * - GE = Gross Energy [MJ/kg]
   * - OM = Organic Matter [kg]
   * - DOMD = Digestible Organic Matter Content [percent]
   *        = digestibility for dry matter forage
   * \note ME is in MJ/kg, but appears in the quotation as
   * divided by 100 to compensate for DOMD being in percent
   * [0--100].
   */
  static const double ME_COEFFICIENT_GRASS;

  /// Hindgut digestion factor
  /**
   * - Johnson et al. (1982) give a value of 0.89
   *   \cite johnson1982intake
   * - Foose (1982) gives a value of 0.84
   *   \cite foose1982trophic
   * - The model by Illius & Gordon (1992) gives a value of 0.93
   *   \cite illius1992modelling
   *
   * Here, the last figure is used.
   */
  static const double DIGESTION_EFFICIENCY_HINDGUTS;

 protected:
  // Interface implementation.
  double get_per_forage_type(const ForageType forage_type,
                             const double digestibility) const;

 private:
  const double digestion_efficiency;
};
}  // namespace Fauna

#endif  // FORAGEENERGY_H

//////////////////////////////////////////////////////////////////////
// REFERENCES
//////////////////////////////////////////////////////////////////////
// Foose, Thomas J. 1982. “Trophic Strategies of Ruminant Versus Nonruminant
// Ungulates.” PhD thesis, University of Chicago. Johnson, D. E., M. M. Borman,
// and L. R. Rittenhouse. 1982. “Intake, Apparent Utilization and Rate of
// Digestion in Mares and Cows.” In Proceedings of the Annual Meeting. American
// Society for Animal Science Western Section. Illius, A. W., and I. J. Gordon.
// 1992. “Modelling the Nutritional Ecology of Ungulate Herbivores: Evolution of
// Body Size and Competitive Interactions.” Oecologia 89 (3):428–34.
// http://link.springer.com/article/10.1007/BF00317422.
