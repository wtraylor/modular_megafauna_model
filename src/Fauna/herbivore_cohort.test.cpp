// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::HerbivoreCohort.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "herbivore_cohort.h"
#include "catch.hpp"
#include "dummy_hft.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::HerbivoreCohort", "") {
  // PREPARE SETTINGS
  Parameters params;
  REQUIRE(params.is_valid());
  std::shared_ptr<Hft> hft(new Hft);
  REQUIRE(hft->is_valid(params));

  static const auto GE = Parameters().forage_gross_energy;

  // exceptions (only specific to HerbivoreCohort)
  // initial density negative
  CHECK_THROWS(HerbivoreCohort(10, 0.5, hft, Sex::Male, -1.0, GE));

  const double BC = 0.5;  // body condition
  const int AGE = 3 * 365;
  const double DENS = 10.0;  // [ind/km²]

  // constructor (only test what is specific to HerbivoreCohort)
  REQUIRE(
      HerbivoreCohort(AGE, BC, hft, Sex::Male, DENS, GE).get_ind_per_km2() ==
      Approx(DENS));

  SECTION("is_same_age()") {
    REQUIRE(AGE % 365 == 0);
    const HerbivoreCohort cohort1(AGE, BC, hft, Sex::Male, DENS, GE);
    // very similar cohort
    CHECK(cohort1.is_same_age(
        HerbivoreCohort(AGE, BC, hft, Sex::Male, DENS, GE)));
    // in the same year
    CHECK(cohort1.is_same_age(
        HerbivoreCohort(AGE + 364, BC, hft, Sex::Male, DENS, GE)));
    // the other is younger
    CHECK(!cohort1.is_same_age(
        HerbivoreCohort(AGE - 364, BC, hft, Sex::Male, DENS, GE)));
    // the other is much older
    CHECK(!cohort1.is_same_age(
        HerbivoreCohort(AGE + 366, BC, hft, Sex::Male, DENS, GE)));
  }

  SECTION("merge") {
    HerbivoreCohort cohort(AGE, BC, hft, Sex::Male, DENS, GE);

    SECTION("exceptions") {
      SECTION("wrong age") {
        HerbivoreCohort other(AGE + 365, BC, hft, Sex::Male, DENS, GE);
        CHECK_THROWS(cohort.merge(other));
      }
      SECTION("wrong sex") {
        HerbivoreCohort other(AGE, BC, hft, Sex::Female, DENS, GE);
        CHECK_THROWS(cohort.merge(other));
      }
      SECTION("wrong HFT") {
        std::shared_ptr<Hft> hft2(new Hft);  // generate default Hft object
        hft2->name = "other_hft";
        REQUIRE(hft2.get() != hft.get());
        REQUIRE(*hft2 != *hft);
        HerbivoreCohort other(AGE, BC, hft2, Sex::Male, DENS, GE);
        CHECK_THROWS(cohort.merge(other));
      }
    }

    SECTION("merge whole cohort") {
      const double old_bodymass = cohort.get_bodymass();
      const double BC2 = BC + 0.1;  // more fat in the other cohort
      const double DENS2 = DENS * 1.5;
      HerbivoreCohort other(AGE, BC2, hft, Sex::Male, DENS2, GE);
      cohort.merge(other);
      // The other cohort is gone
      CHECK(other.get_kg_per_km2() == 0.0);
      // More fat => more bodymass
      CHECK(cohort.get_bodymass() > old_bodymass);
      // That’s all we can test from the public methods...
    }
  }

  SECTION("mortality") {}
}
