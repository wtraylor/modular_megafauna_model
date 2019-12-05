/**
 * \file
 * \brief Unit test for forage energy content models.
 * \copyright ...
 * \date 2019
 */
#include <sstream>
#include "catch.hpp"
#include "net_energy_models.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::get_net_energy_content_default()", "") {
  static const Digestibility DIG1(0.5);
  static const Digestibility DIG2(0.3);
  static const ForageEnergyContent ME = Parameters().metabolizable_energy;

  // higher digestibility ==> more energy
  auto energy1 = get_net_energy_content_default(DIG1, ME);
  auto energy2 = get_net_energy_content_default(DIG2, ME);
  for (const auto& ft : FORAGE_TYPES) {
    INFO("ft = " + get_forage_type_name(ft));
    INFO("ME[" + get_forage_type_name(ft) + "] = " + std::to_string(ME[ft]));
    CHECK(energy1[ft] > energy2[ft]);
  }

  const double ME_GRASS = ME[ForageType::Grass] * DIG1[ForageType::Grass];
  CHECK(get_net_energy_content_default(DIG1, ME)[ForageType::Grass] ==
        Approx(ME_GRASS * (0.019 * ME_GRASS + 0.503)));
}
