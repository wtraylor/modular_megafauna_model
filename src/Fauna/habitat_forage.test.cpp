// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::HabitatForage.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "catch.hpp"
#include "habitat_forage.h"

using namespace Fauna;

TEST_CASE("Fauna::HabitatForage", "") {
  HabitatForage hf1 = HabitatForage();

  // Initialization
  REQUIRE(hf1.get_total().get_mass() == Approx(0.0));
  REQUIRE(hf1.get_total().get_digestibility() == Approx(0.0));

  SECTION("adding forage") {
    const double GRASSMASS = 10.0;        // dry matter [kgDM/km²]
    const double NMASS = GRASSMASS * .1;  // nitrogen [kgN/km²]
    hf1.grass.set_mass(GRASSMASS);
    hf1.grass.set_digestibility(0.5);
    hf1.grass.set_fpc(0.3);

    // Nitrogen
    CHECK_THROWS(hf1.grass.set_nitrogen_mass(GRASSMASS * 1.1));
    hf1.grass.set_nitrogen_mass(NMASS);
    CHECK(hf1.grass.get_nitrogen_mass() == NMASS);
    CHECK(hf1.get_nitrogen_content()[ForageType::Grass] ==
          Approx(NMASS / GRASSMASS));
    CHECK_THROWS(hf1.grass.set_mass(NMASS * .9));

    // Check value access
    REQUIRE(hf1.grass.get_mass() == GRASSMASS);
    CHECK(hf1.grass.get_mass() == hf1.get_mass()[ForageType::Grass]);
    REQUIRE(hf1.get_total().get_mass() == GRASSMASS);
    CHECK(hf1.get_total().get_mass() == Approx(hf1.get_mass().sum()));
    REQUIRE(hf1.get_total().get_digestibility() == 0.5);
  }

  // The member function `merge()` is not tested here
  // because it is a nothing more than simple wrapper around
  // the merge functions of ForageBase and its child classes.
}
