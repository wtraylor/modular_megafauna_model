/**
 * \file
 * \brief Energy content of herbivore forage.
 * \copyright ...
 * \date 2019
 */
#include "net_energy_models.h"

using namespace Fauna;
//
/// Metabolizable energy coefficient of grass [MJ/kgDM]
/**
 * Givens et al. (1989, p. 39)\cite givens1989digestibility :
 *
 * > “In the absence of energetic data, it has been common to calculate ME
 * > from DOMD content. MAFF et al. (1984) \cite maff1984energy stated that
 * > for a wide range of feedstuffs ME may be calculated as 0.015*DOMD. This
 * > is based on the assumption that the GE of digested OM is 19.0 MJ/kg
 * > together with a ME/DE ratio of 0.81.”
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

static const double ME_COEFFICIENT_GRASS = 15.0;
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
static const double DIGESTION_EFFICIENCY_HINDGUTS = 0.93;

ForageEnergyContent Fauna::get_net_energy_content_default(
    const Digestibility& digestibility, const DigestionType digestion_type) {
  const double digestion_efficiency = digestion_type == DigestionType::Ruminant
                                          ? 1.0
                                          : DIGESTION_EFFICIENCY_HINDGUTS;

  if (!(digestibility >= 0.0 && digestibility <= 1.0))
    throw std::invalid_argument(
        "Fauna::GetNetEnergyContentDefault() "
        "Digestibility out of range");
  ForageEnergyContent result;
  // loop through all forage types and call abstract method.
  for (const auto forage_type : FORAGE_TYPES) {
    // PREPARE VARIABLES

    double ME = 0.0;  // metabolizable energy
    if (forage_type == ForageType::Inedible) {
      result[forage_type] = 0.0;
      continue;
    } else if (forage_type == ForageType::Grass) {
      ME = digestibility[forage_type] * ME_COEFFICIENT_GRASS;
      // ADD NEW FORAGE TYPES HERE
    } else
      throw std::logic_error(
          "Fauna::GetNetEnergyContentDefault() "
          "Forage type is not implemented");

    const double e = digestion_efficiency;

    // COMPOSE THE FORMULA

    result[forage_type] = ME * (0.019 * ME + 0.503) * e;  // [MJ/kgDM]

    assert(result[forage_type] >= 0.0);
  }
  return result;
}
