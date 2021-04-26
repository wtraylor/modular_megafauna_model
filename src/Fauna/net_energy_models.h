// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Energy content of herbivore forage.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_NET_ENERGY_MODELS_H
#define FAUNA_NET_ENERGY_MODELS_H

#include "forage_values.h"

namespace Fauna {

/// Get net energy content of the forage [MJ/kgDM].
/**
 * See \ref sec_energy_content for a detailed description of this net energy
 * model.
 *
 * \[f
 * NE = ME * k_m = DE * ME/DE * k_m = GE * DMD * ME/DE * k_m
 * \]f
 *
 * \param ge_content Gross energy (GE), also called combustion energy, in dry
 * matter. [MJ/kgDM]
 * \param digestibility Proportional dry-matter digestibility (DMD).
 * \param me_coefficient Metabolizable energy coefficient, i.e. the ratio of
 * metabolizable energy to digestible energy, ME/DE. [fractional]
 * \param k_maintenance Net energy coefficient (k_m) for maintenance, i.e. the
 * proportion of metabolizable remaining energy after heat increment.
 * [fractional]
 *
 * \return Net energy content [MJ/kgDM].
 *
 * \throw std::invalid_argument If either `me_coefficient` or `k_maintenance`
 * not in interval (0,1).
 *
 * \see \ref sec_energy_content
 * \see \ref Parameters::forage_gross_energy
 * \see \ref Hft::digestion_me_coefficient
 * \see \ref Hft::digestion_k_maintenance
 * \see \ref Hft::digestion_net_energy_model
 * \see \ref NetEnergyModel
 */
inline ForageEnergyContent get_net_energy_from_gross_energy(
    const ForageEnergyContent& ge_content,
    const Digestibility& digestibility,
    const double me_coefficient,
    const double k_maintenance){
  if (me_coefficient <= 0.0 || me_coefficient >= 1.0)
    throw std::invalid_argument("Fauna::get_net_energy_from_gross_energy(): "
        "Parameter `me_coefficient` is not in interval (0,1).");
  if (k_maintenance <= 0.0 || k_maintenance >= 1.0)
    throw std::invalid_argument("Fauna::get_net_energy_from_gross_energy(): "
        "Parameter `k_maintenance` is not in interval (0,1).");
  return ge_content * digestibility * (me_coefficient * k_maintenance);
}
}  // namespace Fauna

#endif  // FAUNA_NET_ENERGY_MODELS_H
