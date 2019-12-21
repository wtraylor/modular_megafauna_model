/**
 * \file
 * \brief Energy content of herbivore forage.
 * \copyright ...
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
 * NE = ME * k = DE * ME/DE * k = GE * DMD * ME/DE * k
 * \]f
 *
 * \param ge_content Gross energy (GE), also called combustion energy, in dry
 * matter. [MJ/kgDM]
 * \param digestibility Proportional dry-matter digestibility (DMD).
 * \param me_coefficient Metabolizable energy coefficient, i.e. the ratio of
 * metabolizable energy to digestible energy, ME/DE. [fractional]
 * \param ne_coefficient Net energy coefficient (k) for maintenance, i.e. the
 * proportion of metabolizable remaining energy after heat increment.
 * [fractional]
 *
 * \return Net energy content [MJ/kgDM].
 *
 * \throw std::invalid_argument If either `me_coefficient` or `ne_coefficient`
 * not in interval (0,1).
 *
 * \see \ref sec_energy_content
 * \see \ref Parameters::forage_gross_energy
 * \see \ref Hft::digestion_me_coefficient
 * \see \ref Hft::digestion_ne_coefficient
 * \see \ref Hft::net_energy_model
 * \see \ref NetEnergyModel
 */
inline ForageEnergyContent get_net_energy_from_gross_energy(
    const ForageEnergyContent& ge_content,
    const Digestibility& digestibility,
    const double me_coefficient,
    const double ne_coefficient){
  if (me_coefficient <= 0.0 || me_coefficient >= 1.0)
    throw std::invalid_argument("Fauna::get_net_energy_from_gross_energy(): "
        "Parameter `me_coefficient` is not in interval (0,1).");
  if (ne_coefficient <= 0.0 || ne_coefficient >= 1.0)
    throw std::invalid_argument("Fauna::get_net_energy_from_gross_energy(): "
        "Parameter `ne_coefficient` is not in interval (0,1).");
  return ge_content * digestibility * (me_coefficient * ne_coefficient);
}
}  // namespace Fauna

#endif  // FAUNA_NET_ENERGY_MODELS_H
