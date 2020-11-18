// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::GrassForage.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "average.h"
#include "catch.hpp"
#include "grass_forage.h"

using namespace Fauna;

TEST_CASE("Fauna::GrassForage", "") {
  SECTION("Initialization") {
    CHECK(GrassForage().get_mass() == 0.0);
    CHECK(GrassForage().get_digestibility() == 0.0);
    CHECK(GrassForage().get_fpc() == 0.0);
  }

  SECTION("Exceptions") {
    CHECK_THROWS(GrassForage().set_fpc(1.2));
    CHECK_THROWS(GrassForage().set_fpc(-0.2));
    CHECK_THROWS(GrassForage().set_mass(-0.2));
    CHECK_THROWS(GrassForage().set_digestibility(-0.2));
    CHECK_THROWS(GrassForage().set_digestibility(1.2));

    GrassForage g;
    CHECK_THROWS(g.set_fpc(0.5));  // mass must be >0.0
    g.set_mass(1.0);
    CHECK_THROWS(g.get_fpc());     // illogical state
    CHECK_THROWS(g.set_fpc(0.0));  // fpc must be >0.0
  }

  SECTION("sward density") {
    CHECK(GrassForage().get_sward_density() == 0.0);

    GrassForage g;
    const double FPC = .234;
    const double MASS = 1256;
    g.set_mass(MASS);
    g.set_fpc(FPC);
    CHECK(g.get_sward_density() == Approx(MASS / FPC));
  }

  SECTION("merge") {
    // merge some arbitrary numbers
    GrassForage g1, g2;
    const double W1 = 956;
    const double W2 = 123;
    const double M1 = 23;
    const double M2 = 54;
    const double D1 = 0.342;
    const double D2 = 0.56;
    const double F1 = 0.76;
    const double F2 = 0.123;
    g1.set_mass(M1);
    g2.set_mass(M2);
    g1.set_digestibility(D1);
    g2.set_digestibility(D2);
    g1.set_fpc(F1);
    g2.set_fpc(F2);

    g1.merge(g2, W1, W2);
    CHECK(g1.get_mass() == Approx(average(M1, M2, W1, W2)));
    CHECK(g1.get_digestibility() == Approx(average(D1, D2, W1, W2)));
    CHECK(g1.get_fpc() == Approx(average(F1, F2, W1, W2)));
  }
}
