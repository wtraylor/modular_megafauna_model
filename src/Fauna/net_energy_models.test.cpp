/**
 * \file
 * \brief Unit test for forage energy content models.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "net_energy_models.h"
using namespace Fauna;

TEST_CASE("Fauna::GetNetEnergyContentDefault", "") {
  GetNetEnergyContentDefault ne_ruminant(DigestionType::Ruminant);
  GetNetEnergyContentDefault ne_hindgut(DigestionType::Hindgut);

  const Digestibility DIG1(0.5);
  const Digestibility DIG2(0.3);

  // higher digestibility ==> more energy
  CHECK(ne_ruminant(DIG1) > ne_ruminant(DIG2));
  CHECK(ne_hindgut(DIG1) > ne_hindgut(DIG2));

  // hindguts have lower efficiency
  CHECK(ne_ruminant(DIG1) > ne_hindgut(DIG1));

  // Check some absolute numbers
  {  // grass for ruminants
    const double ME = 15.0 * DIG1[ForageType::Grass];
    CHECK(ne_ruminant(DIG1[ForageType::Grass])[ForageType::Grass] ==
          Approx(ME * (0.019 * ME + 0.503)));
  }
  {  // grass for hindguts
    const double ME = 15.0 * DIG1[ForageType::Grass];
    CHECK(ne_hindgut(DIG1[ForageType::Grass])[ForageType::Grass] ==
          Approx(ME * (0.019 * ME + 0.503) * 0.93));
  }
}
