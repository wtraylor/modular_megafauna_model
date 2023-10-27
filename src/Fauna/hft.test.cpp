// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::Hft.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "hft.h"

#include "catch.hpp"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::GivenPointAllometry()") {
  // An arbitrary value: the y value for adult male body mass.
  static const double Y = 1.3;

  // Typical case without error.
  REQUIRE_NOTHROW(GivenPointAllometry({0.75, Y}).extrapolate(10, 8));
  // Body mass <= 0
  CHECK_THROWS(GivenPointAllometry({0.75, Y}).extrapolate(0, 10));
  CHECK_THROWS(GivenPointAllometry({0.75, Y}).extrapolate(10, 0));
  CHECK_THROWS(GivenPointAllometry({0.75, Y}).extrapolate(10, -5));
  CHECK_THROWS(GivenPointAllometry({0.75, Y}).extrapolate(-10, -5));

  // Whatever the adult male body mass and the exponent are, the given y value
  // will be the result if current body mass equals adult male body mass.
  CHECK(GivenPointAllometry({0.73, Y}).extrapolate(10, 10) == Approx(Y));
  CHECK(GivenPointAllometry({0.25, Y}).extrapolate(8, 8) == Approx(Y));
  CHECK(GivenPointAllometry({0.89, Y}).extrapolate(800, 800) == Approx(Y));

  // Check some arbitrary numbers.

  static const double M = 100;   // [kg] An arbitrary adult male body mass.
  static const double E = 0.75;  // exponent
  static const GivenPointAllometry ALLOMETRY = {E, Y};
  // Calculate the coefficient $c$ in $y = c * M^e$.
  static const double C = Y / pow(M, E);
  for (const double m : {0.1, 15.0, 100.0, 132.0, 200.0})
    CHECK(ALLOMETRY.extrapolate(M, m) == Approx(C * pow(m, E)));
}

TEST_CASE("Fauna::Hft", "") {
  Hft hft = Hft();
  std::string msg;

  SECTION("not valid without name") {
    hft.name = "";
    CHECK_FALSE(hft.is_valid(Fauna::Parameters(), msg));
  }
}
