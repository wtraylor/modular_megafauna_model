/**
 * \file
 * \brief Energy content of herbivore forage.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_NET_ENERGY_MODELS_H
#define FAUNA_NET_ENERGY_MODELS_H

#include "forage_types.h"
#include "forage_values.h"
#include "hft.h"

namespace Fauna {

/// Get net energy content of the forage [MJ/kgDM]
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
 * \param digestibility Proportional digestibility.
 * \param digestion_type Is it a ruminant or hindgut herbivore?
 * \return Net energy content [MJ/kgDM].
 */
ForageEnergyContent get_net_energy_content_default(
    const Digestibility& digestibility, const DigestionType digestion_type);
}  // namespace Fauna

#endif  // FAUNA_NET_ENERGY_MODELS_H
