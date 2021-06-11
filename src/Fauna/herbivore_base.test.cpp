// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::HerbivoreBase.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "herbivore_base.h"
#include "catch.hpp"
#include "dummy_herbivore_base.h"
#include "dummy_hft.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::HerbivoreBase", "") {
  // Since HerbivoreBase cannot be instantiated directly, we
  // test the relevant functionality in HerbivoreBaseDummy.

  // PREPARE SETTINGS
  Parameters params;
  REQUIRE(params.is_valid());
  auto hft = create_hfts(1, params)[0];
  REQUIRE(hft->is_valid(params));

  // Let’s throw some exceptions
  CHECK_THROWS(HerbivoreBaseDummy(-1, 0.5, hft,  // age_days
                                  Sex::Male));
  CHECK_THROWS(HerbivoreBaseDummy(100, 0.5, NULL,  // hft== NULL
                                  Sex::Male));
  CHECK_THROWS(HerbivoreBaseDummy(100, 1.1, hft,  // body_conditon
                                  Sex::Male));
  CHECK_THROWS(HerbivoreBaseDummy(100, -0.1, hft,  // body_conditon
                                  Sex::Male));

  SECTION("Body mass") {
    SECTION("Birth") {
      // call the birth constructor
      const HerbivoreBaseDummy birth(hft, Sex::Male);

      REQUIRE(&birth.get_hft() == hft.get());
      REQUIRE(birth.get_age_days() == 0);
      REQUIRE(birth.get_age_years() == 0);

      INFO("hft->body_mass_birth = " << hft->body_mass_birth);
      INFO("hft->body_fat_birth = " << hft->body_fat_birth);
      INFO("hft->body_mass_empty = " << hft->body_mass_empty);
      INFO("hft->body_fat_maximum = " << hft->body_fat_maximum);

      const double fatmass_birth =
          hft->body_mass_birth * hft->body_mass_empty * hft->body_fat_birth;
      const double lean_bodymass_birth = hft->body_mass_birth - fatmass_birth;
      INFO("fatmass_birth = " << fatmass_birth);
      INFO("lean_bodymass_birth = " << lean_bodymass_birth);

      REQUIRE(hft->body_mass_birth ==
              Approx(lean_bodymass_birth + fatmass_birth));

      // body mass
      CHECK(birth.get_bodymass() == Approx(hft->body_mass_birth));

      // fat mass
      CHECK(birth.get_fatmass() == Approx(fatmass_birth));
      CHECK(birth.get_bodyfat() == Approx(hft->body_fat_birth));
    }

    SECTION("Pre-adult") {
      const double BODY_COND = 1.0;
      SECTION("pre-adult male") {
        const int AGE_YEARS = hft->life_history_physical_maturity_male - 1;
        const int AGE_DAYS = AGE_YEARS * 365;
        REQUIRE(AGE_DAYS > 0);
        const HerbivoreBaseDummy male_young(AGE_DAYS, BODY_COND, hft,
                                            Sex::Male);
        REQUIRE(male_young.get_age_days() == AGE_DAYS);
        REQUIRE(male_young.get_age_years() == AGE_YEARS);
        CHECK(male_young.get_bodymass() < hft->body_mass_male);
        CHECK(male_young.get_bodymass() > hft->body_mass_birth);
        CHECK(male_young.get_fatmass() / male_young.get_max_fatmass() ==
              Approx(BODY_COND));
      }

      SECTION("pre-adult female") {
        const int AGE_YEARS = hft->life_history_physical_maturity_female - 1;
        const int AGE_DAYS = AGE_YEARS * 365;
        REQUIRE(AGE_DAYS > 0);
        const HerbivoreBaseDummy female_young(AGE_DAYS, BODY_COND, hft,
                                              Sex::Female);
        REQUIRE(female_young.get_age_days() == AGE_DAYS);
        REQUIRE(female_young.get_age_years() == AGE_YEARS);
        CHECK(female_young.get_bodymass() < hft->body_mass_female);
        CHECK(female_young.get_bodymass() > hft->body_mass_birth);
        CHECK(female_young.get_fatmass() / female_young.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }

    SECTION("Adult with full fat") {
      const double BODY_COND = 1.0;  // optimal body condition
      SECTION("Adult male with full fat") {
        const int AGE_YEARS = hft->life_history_physical_maturity_male;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy male_adult(AGE_DAYS, BODY_COND, hft,
                                            Sex::Male);
        // AGE
        REQUIRE(male_adult.get_age_days() == AGE_DAYS);
        REQUIRE(male_adult.get_age_years() == AGE_YEARS);
        // BODY MASS
        // Since the HFT parameter for adult body mass is with 1/2 the max. fat
        // reserves, the body mass with full fat reserves must be higher.
        CHECK(male_adult.get_bodymass() >= Approx(hft->body_mass_male));
        // FAT MASS
        CHECK(male_adult.get_bodyfat() == Approx(hft->body_fat_maximum));
        CHECK(male_adult.get_fatmass() / male_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
      SECTION("Adult female with full fat") {
        const int AGE_YEARS = hft->life_history_physical_maturity_female;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy female_adult(AGE_DAYS, BODY_COND, hft,
                                              Sex::Female);
        // AGE
        REQUIRE(female_adult.get_age_days() == AGE_DAYS);
        REQUIRE(female_adult.get_age_years() == AGE_YEARS);
        // BODY MASS
        CHECK(female_adult.get_bodymass() >= Approx(hft->body_mass_female));
        // FAT MASS
        CHECK(female_adult.get_bodyfat() == Approx(hft->body_fat_maximum));
        CHECK(female_adult.get_fatmass() / female_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }

    SECTION("Adult with low fat") {
      const double BODY_COND = 0.3;  // poor body condition

      SECTION("Male") {
        const HerbivoreBaseDummy male_adult(
            hft->life_history_physical_maturity_male * 365, BODY_COND, hft,
            Sex::Male);
        // FAT MASS
        CHECK(male_adult.get_fatmass() / male_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }

      SECTION("Female") {
        const HerbivoreBaseDummy female_adult(
            hft->life_history_physical_maturity_male * 365, BODY_COND, hft,
            Sex::Female);
        // FAT MASS
        CHECK(female_adult.get_fatmass() / female_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }
  }
}
