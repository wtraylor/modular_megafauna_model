/**
 * \file
 * \brief Energy content of herbivore forage.
 * \copyright ...
 * \date 2019
 */
#include "net_energy_models.h"

using namespace Fauna;

ForageEnergyContent Fauna::get_net_energy_content_default(
    const Digestibility& digestibility,
    const ForageEnergyContent& metabolizable_energy) {
  if (!(digestibility >= 0.0 && digestibility <= 1.0))
    throw std::invalid_argument(
        "Fauna::GetNetEnergyContentDefault() "
        "Digestibility out of range");
  const auto ME = metabolizable_energy * digestibility;
  return ME * (ME * 0.019 + 0.503);
}
