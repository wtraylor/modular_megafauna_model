/**
 * \file
 * \brief Unit test for Fauna::HerbivoreCohort.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "dummy_hft.h"
#include "herbivore_cohort.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::HerbivoreCohort", "") {
  // PREPARE SETTINGS
  Parameters params;
  REQUIRE(params.is_valid());
  Hft hft = create_hfts(1, params)[0];
  REQUIRE(hft.is_valid(params));

  static const auto ME = Parameters().metabolizable_energy;

  // exceptions (only specific to HerbivoreCohort)
  // initial density negative
  CHECK_THROWS(HerbivoreCohort(10, 0.5, &hft, Sex::Male, -1.0, ME));

  const double BC = 0.5;  // body condition
  const int AGE = 3 * 365;
  const double DENS = 10.0;  // [ind/km²]

  // constructor (only test what is specific to HerbivoreCohort)
  REQUIRE(
      HerbivoreCohort(AGE, BC, &hft, Sex::Male, DENS, ME).get_ind_per_km2() ==
      Approx(DENS));

  SECTION("is_same_age()") {
    REQUIRE(AGE % 365 == 0);
    const HerbivoreCohort cohort1(AGE, BC, &hft, Sex::Male, DENS, ME);
    // very similar cohort
    CHECK(cohort1.is_same_age(
        HerbivoreCohort(AGE, BC, &hft, Sex::Male, DENS, ME)));
    // in the same year
    CHECK(cohort1.is_same_age(
        HerbivoreCohort(AGE + 364, BC, &hft, Sex::Male, DENS, ME)));
    // the other is younger
    CHECK(!cohort1.is_same_age(
        HerbivoreCohort(AGE - 364, BC, &hft, Sex::Male, DENS, ME)));
    // the other is much older
    CHECK(!cohort1.is_same_age(
        HerbivoreCohort(AGE + 366, BC, &hft, Sex::Male, DENS, ME)));
  }

  SECTION("merge") {
    HerbivoreCohort cohort(AGE, BC, &hft, Sex::Male, DENS, ME);

    SECTION("exceptions") {
      SECTION("wrong age") {  // wrong age
        HerbivoreCohort other(AGE + 365, BC, &hft, Sex::Male, DENS, ME);
        CHECK_THROWS(cohort.merge(other));
      }
      SECTION("wrong sex") {  // wrong sex
        HerbivoreCohort other(AGE, BC, &hft, Sex::Female, DENS, ME);
        CHECK_THROWS(cohort.merge(other));
      }
      SECTION("wrong HFT") {  // wrong HFT
        Hft hft2 = create_hfts(2, params)[1];
        REQUIRE(hft2 != hft);
        HerbivoreCohort other(AGE, BC, &hft2, Sex::Male, DENS, ME);
        CHECK_THROWS(cohort.merge(other));
      }
    }

    SECTION("merge whole cohort") {
      const double old_bodymass = cohort.get_bodymass();
      const double BC2 = BC + 0.1;  // more fat in the other cohort
      const double DENS2 = DENS * 1.5;
      HerbivoreCohort other(AGE, BC2, &hft, Sex::Male, DENS2, ME);
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
