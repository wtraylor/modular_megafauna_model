/**
 * \file
 * \brief Unit test for forage energy content models.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "net_energy_models.h"
using namespace Fauna;

TEST_CASE("Fauna::get_net_energy_content_default", "") {
  static const Digestibility DIG1(0.5);
  static const Digestibility DIG2(0.3);

  // higher digestibility ==> more energy
  CHECK(get_net_energy_content_default(DIG1, DigestionType::Ruminant) >
        get_net_energy_content_default(DIG2, DigestionType::Ruminant));
  CHECK(get_net_energy_content_default(DIG1, DigestionType::Hindgut) >
        get_net_energy_content_default(DIG2, DigestionType::Hindgut));

  // hindguts have lower efficiency
  CHECK(get_net_energy_content_default(DIG1, DigestionType::Ruminant) >
        get_net_energy_content_default(DIG1, DigestionType::Hindgut));

  // Check some absolute numbers
  {  // grass for ruminants
    static const double ME = 15.0 * DIG1[ForageType::Grass];
    CHECK(get_net_energy_content_default(
              DIG1[ForageType::Grass],
              DigestionType::Ruminant)[ForageType::Grass] ==
          Approx(ME * (0.019 * ME + 0.503)));
  }
  {  // grass for hindguts
    static const double ME = 15.0 * DIG1[ForageType::Grass];
    CHECK(get_net_energy_content_default(
              DIG1[ForageType::Grass],
              DigestionType::Hindgut)[ForageType::Grass] ==
          Approx(ME * (0.019 * ME + 0.503) * 0.93));
  }
}
