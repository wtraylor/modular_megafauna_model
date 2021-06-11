// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for herbivore reproduction models.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "reproduction_models.h"
#include "catch.hpp"
using namespace Fauna;

TEST_CASE("Fauna::ReproductionLogistic", "") {
  // NOTE: We are more tolerant with the Approx() function of
  // the CATCH framework (by adjusting Approx().epsilon().

  const double INC = 1.0;
  const int START = 100;
  const int LENGTH = 90;
  const double OPT = 1.0;  // optimal body condition

  const BreedingSeason season(START, LENGTH);

  static const double B = 15.0;  // growth rate
  static const double C = 0.3;   // midpoint

  // exceptions
  SECTION("exceptions") {
    CHECK_THROWS(ReproductionLogistic(season, -1.0, B, C));
    CHECK_THROWS(ReproductionLogistic(season, INC, 0.0, C));
    CHECK_THROWS(ReproductionLogistic(season, INC, -1.0, C));
    CHECK_THROWS(ReproductionLogistic(season, INC, B, 0.0));
    CHECK_THROWS(ReproductionLogistic(season, INC, B, -1.0));
    CHECK_THROWS(ReproductionLogistic(season, INC, B, 1.0));
    CHECK_THROWS(ReproductionLogistic(season, INC, B, 1.1));
    ReproductionLogistic rep(season, INC, B, C);
    CHECK_THROWS(rep.get_offspring_density(-1, OPT));
    CHECK_THROWS(rep.get_offspring_density(365, OPT));
    CHECK_THROWS(rep.get_offspring_density(START, -0.1));
    CHECK_THROWS(rep.get_offspring_density(START, 1.1));
  }

  SECTION("higher annual increase makes more offspring") {
    const double INC2 = INC * 1.5;
    REQUIRE(INC2 > INC);
    ReproductionLogistic rep1(season, INC, B, C);
    ReproductionLogistic rep2(season, INC2, B, C);
    CHECK(rep1.get_offspring_density(START, OPT) <
          rep2.get_offspring_density(START, OPT));
    CHECK(rep1.get_offspring_density(START, OPT) < INC);
    CHECK(rep2.get_offspring_density(START, OPT) < INC2);
  }

  SECTION("better body condition makes more offspring") {
    const double BAD = OPT / 2.0;  // bad body condition
    ReproductionLogistic rep(season, INC, B, C);
    CHECK(rep.get_offspring_density(START, BAD) <
          rep.get_offspring_density(START, OPT));
  }

  SECTION("one-day season length -> all offspring at once") {
    const double BAD = OPT / 2.0;  // bad body condition
    BreedingSeason season_short(START, 1);
    ReproductionLogistic rep(season_short, INC, B, C);
    CHECK(rep.get_offspring_density(START, OPT) == Approx(INC).epsilon(0.05));
    CHECK(rep.get_offspring_density(START, BAD) < INC);

    SECTION("Check an absolute value for bad body condition") {
      CHECK(rep.get_offspring_density(START, BAD) ==
            Approx(INC / (1 + exp(-15.0 * (BAD - 0.3)))));
    }
  }

  SECTION("Sum of offspring over year must be max. annual increase") {
    ReproductionLogistic rep(season, INC, B, C);
    // sum over whole year
    double sum_year = 0.0;
    for (int d = 0; d < 365; d++)
      sum_year += rep.get_offspring_density((START + d) % 364, OPT);
    CHECK(sum_year == Approx(INC).epsilon(0.05));

    // sum over the breeding season only
    double sum_season = 0.0;
    for (int d = START; d < START + LENGTH; d++)
      sum_season += rep.get_offspring_density(d, OPT);
    CHECK(sum_season == Approx(INC).epsilon(0.05));
  }
}
