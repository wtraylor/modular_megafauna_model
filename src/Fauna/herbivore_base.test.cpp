/**
 * \file
 * \brief Unit test for Fauna::HerbivoreBase.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "dummy_herbivore_base.h"
#include "dummy_hft.h"
#include "herbivore_base.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::HerbivoreBase", "") {
  // Since HerbivoreBase cannot be instantiated directly, we
  // test the relevant functionality in HerbivoreBaseDummy.

  // PREPARE SETTINGS
  Parameters params;
  REQUIRE(params.is_valid());
  Hft hft = create_hfts(1, params)[0];
  REQUIRE(hft.is_valid(params));

  // Let’s throw some exceptions
  CHECK_THROWS(HerbivoreBaseDummy(-1, 0.5, &hft,  // age_days
                                  Sex::Male));
  CHECK_THROWS(HerbivoreBaseDummy(100, 0.5, NULL,  // hft== NULL
                                  Sex::Male));
  CHECK_THROWS(HerbivoreBaseDummy(100, 1.1, &hft,  // body_conditon
                                  Sex::Male));
  CHECK_THROWS(HerbivoreBaseDummy(100, -0.1, &hft,  // body_conditon
                                  Sex::Male));

  SECTION("Body mass") {
    SECTION("Birth") {
      // call the birth constructor
      const HerbivoreBaseDummy birth(&hft, Sex::Male);

      REQUIRE(&birth.get_hft() == &hft);
      REQUIRE(birth.get_age_days() == 0);
      REQUIRE(birth.get_age_years() == 0);

      const double lean_bodymass_birth =
          hft.bodymass_birth * (1.0 - hft.bodyfat_birth);
      const double pot_bodymass_birth =
          lean_bodymass_birth / (1.0 - hft.bodyfat_max);
      // body mass
      CHECK(birth.get_bodymass() == Approx(hft.bodymass_birth));
      CHECK(birth.get_potential_bodymass() == Approx(pot_bodymass_birth));
      CHECK(birth.get_lean_bodymass() == Approx(lean_bodymass_birth));
      // fat mass
      CHECK(birth.get_bodyfat() == Approx(hft.bodyfat_birth));
      CHECK(birth.get_max_fatmass() ==
            Approx(pot_bodymass_birth * hft.bodyfat_max));
    }

    SECTION("Pre-adult") {
      const double BODY_COND = 1.0;
      SECTION("pre-adult male") {
        const int AGE_YEARS = hft.maturity_age_phys_male / 2;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy male_young(AGE_DAYS, BODY_COND, &hft,
                                            Sex::Male);
        REQUIRE(male_young.get_age_days() == AGE_DAYS);
        REQUIRE(male_young.get_age_years() == AGE_YEARS);
        CHECK(male_young.get_bodymass() < hft.bodymass_male);
        CHECK(male_young.get_bodymass() > hft.bodymass_birth);
        CHECK(male_young.get_fatmass() / male_young.get_max_fatmass() ==
              Approx(BODY_COND));
      }

      SECTION("pre-adult female") {
        const int AGE_YEARS = hft.maturity_age_phys_female / 2;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy female_young(AGE_DAYS, BODY_COND, &hft,
                                              Sex::Female);
        REQUIRE(female_young.get_age_days() == AGE_DAYS);
        REQUIRE(female_young.get_age_years() == AGE_YEARS);
        CHECK(female_young.get_bodymass() < hft.bodymass_female);
        CHECK(female_young.get_bodymass() > hft.bodymass_birth);
        CHECK(female_young.get_fatmass() / female_young.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }

    SECTION("Adult with full fat") {
      const double BODY_COND = 1.0;  // optimal body condition
      SECTION("Adult male with full fat") {
        const int AGE_YEARS = hft.maturity_age_phys_male;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy male_adult(AGE_DAYS, BODY_COND, &hft,
                                            Sex::Male);
        // AGE
        REQUIRE(male_adult.get_age_days() == AGE_DAYS);
        REQUIRE(male_adult.get_age_years() == AGE_YEARS);
        // BODY MASS
        CHECK(male_adult.get_bodymass() == Approx(hft.bodymass_male));
        CHECK(male_adult.get_potential_bodymass() == male_adult.get_bodymass());
        CHECK(male_adult.get_lean_bodymass() ==
              Approx(hft.bodymass_male * (1.0 - hft.bodyfat_max)));
        // FAT MASS
        CHECK(male_adult.get_max_fatmass() ==
              Approx(hft.bodyfat_max * hft.bodymass_male));
        CHECK(male_adult.get_bodyfat() == Approx(hft.bodyfat_max));
        CHECK(male_adult.get_fatmass() / male_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
      SECTION("Adult female with full fat") {
        const int AGE_YEARS = hft.maturity_age_phys_female;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy female_adult(hft.maturity_age_phys_male * 365,
                                              BODY_COND, &hft, Sex::Female);
        // AGE
        REQUIRE(female_adult.get_age_days() == AGE_DAYS);
        REQUIRE(female_adult.get_age_years() == AGE_YEARS);
        // BODY MASS
        CHECK(female_adult.get_bodymass() == Approx(hft.bodymass_female));
        CHECK(female_adult.get_potential_bodymass() ==
              female_adult.get_bodymass());
        CHECK(female_adult.get_lean_bodymass() ==
              Approx(hft.bodymass_female * (1.0 - hft.bodyfat_max)));
        // FAT MASS
        CHECK(female_adult.get_max_fatmass() ==
              Approx(hft.bodyfat_max * hft.bodymass_female));
        CHECK(female_adult.get_bodyfat() == Approx(hft.bodyfat_max));
        CHECK(female_adult.get_fatmass() / female_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }

    SECTION("Adult with low fat") {
      const double BODY_COND = 0.3;  // poor body condition

      SECTION("Male") {
        const HerbivoreBaseDummy male_adult(hft.maturity_age_phys_male * 365,
                                            BODY_COND, &hft, Sex::Male);
        // BODY MASS
        CHECK(male_adult.get_potential_bodymass() == Approx(hft.bodymass_male));
        CHECK(male_adult.get_lean_bodymass() + male_adult.get_max_fatmass() ==
              Approx(male_adult.get_potential_bodymass()));
        // FAT MASS
        CHECK(male_adult.get_max_fatmass() ==
              Approx(hft.bodyfat_max * hft.bodymass_male));
        CHECK(male_adult.get_fatmass() / male_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }

      SECTION("Female") {
        const HerbivoreBaseDummy female_adult(hft.maturity_age_phys_male * 365,
                                              BODY_COND, &hft, Sex::Female);
        // BODY MASS
        CHECK(female_adult.get_potential_bodymass() ==
              Approx(hft.bodymass_female));
        CHECK(female_adult.get_lean_bodymass() +
                  female_adult.get_max_fatmass() ==
              Approx(female_adult.get_potential_bodymass()));
        // FAT MASS
        CHECK(female_adult.get_max_fatmass() ==
              Approx(hft.bodyfat_max * hft.bodymass_female));
        CHECK(female_adult.get_fatmass() / female_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }
  }
}
