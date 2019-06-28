#include <catch2/catch.hpp>
#include "dummy_herbivore_base.h"
#include "dummy_hft.h"
#include "environment.h"
#include "herbivore.h"
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
                                  SEX_MALE));
  CHECK_THROWS(HerbivoreBaseDummy(100, 0.5, NULL,  // hft== NULL
                                  SEX_MALE));
  CHECK_THROWS(HerbivoreBaseDummy(100, 1.1, &hft,  // body_conditon
                                  SEX_MALE));
  CHECK_THROWS(HerbivoreBaseDummy(100, -0.1, &hft,  // body_conditon
                                  SEX_MALE));

  SECTION("Body mass") {
    SECTION("Birth") {
      // call the birth constructor
      const HerbivoreBaseDummy birth(&hft, SEX_MALE);

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
                                            SEX_MALE);
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
                                              SEX_FEMALE);
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
                                            SEX_MALE);
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
                                              BODY_COND, &hft, SEX_FEMALE);
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
                                            BODY_COND, &hft, SEX_MALE);
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
                                              BODY_COND, &hft, SEX_FEMALE);
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

TEST_CASE("Fauna::HerbivoreCohort", "") {
  // PREPARE SETTINGS
  Parameters params;
  REQUIRE(params.is_valid());
  Hft hft = create_hfts(1, params)[0];
  REQUIRE(hft.is_valid(params));

  // exceptions (only specific to HerbivoreCohort)
  // initial density negative
  CHECK_THROWS(HerbivoreCohort(10, 0.5, &hft, SEX_MALE, -1.0));

  const double BC = 0.5;  // body condition
  const int AGE = 3 * 365;
  const double DENS = 10.0;  // [ind/km²]

  // constructor (only test what is specific to HerbivoreCohort)
  REQUIRE(HerbivoreCohort(AGE, BC, &hft, SEX_MALE, DENS).get_ind_per_km2() ==
          Approx(DENS));

  SECTION("is_same_age()") {
    REQUIRE(AGE % 365 == 0);
    const HerbivoreCohort cohort1(AGE, BC, &hft, SEX_MALE, DENS);
    // very similar cohort
    CHECK(cohort1.is_same_age(HerbivoreCohort(AGE, BC, &hft, SEX_MALE, DENS)));
    // in the same year
    CHECK(cohort1.is_same_age(
        HerbivoreCohort(AGE + 364, BC, &hft, SEX_MALE, DENS)));
    // the other is younger
    CHECK(!cohort1.is_same_age(
        HerbivoreCohort(AGE - 364, BC, &hft, SEX_MALE, DENS)));
    // the other is much older
    CHECK(!cohort1.is_same_age(
        HerbivoreCohort(AGE + 366, BC, &hft, SEX_MALE, DENS)));
  }

  SECTION("merge") {
    HerbivoreCohort cohort(AGE, BC, &hft, SEX_MALE, DENS);

    SECTION("exceptions") {
      SECTION("wrong age") {  // wrong age
        HerbivoreCohort other(AGE + 365, BC, &hft, SEX_MALE, DENS);
        CHECK_THROWS(cohort.merge(other));
      }
      SECTION("wrong sex") {  // wrong sex
        HerbivoreCohort other(AGE, BC, &hft, SEX_FEMALE, DENS);
        CHECK_THROWS(cohort.merge(other));
      }
      SECTION("wrong HFT") {  // wrong HFT
        Hft hft2 = create_hfts(2, params)[1];
        REQUIRE(hft2 != hft);
        HerbivoreCohort other(AGE, BC, &hft2, SEX_MALE, DENS);
        CHECK_THROWS(cohort.merge(other));
      }
    }

    SECTION("merge whole cohort") {
      const double old_bodymass = cohort.get_bodymass();
      const double BC2 = BC + 0.1;  // more fat in the other cohort
      const double DENS2 = DENS * 1.5;
      HerbivoreCohort other(AGE, BC2, &hft, SEX_MALE, DENS2);
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

TEST_CASE("Fauna::HerbivoreIndividual", "") {
  // PREPARE SETTINGS
  Parameters params;
  REQUIRE(params.is_valid());
  Hft hft = create_hfts(1, params)[0];
  REQUIRE(hft.is_valid(params));

  const double BC = 0.5;     // body condition
  const int AGE = 842;       // som arbitrary number [days]
  const double AREA = 10.0;  // [km²]

  // exceptions (only specific to HerbivoreIndividual)
  // invalid area
  CHECK_THROWS(HerbivoreIndividual(AGE, BC, &hft, SEX_MALE, -1.0));
  CHECK_THROWS(HerbivoreIndividual(AGE, BC, &hft, SEX_MALE, 0.0));
  CHECK_THROWS(HerbivoreIndividual(&hft, SEX_MALE, -1.0));
  CHECK_THROWS(HerbivoreIndividual(&hft, SEX_MALE, 0.0));

  // birth constructor
  REQUIRE(HerbivoreIndividual(&hft, SEX_MALE, AREA).get_area_km2() ==
          Approx(AREA));
  // establishment constructor
  REQUIRE(HerbivoreIndividual(AGE, BC, &hft, SEX_MALE, AREA).get_area_km2() ==
          Approx(AREA));

  SECTION("Mortality") {
    hft.mortality_factors.insert(MF_STARVATION_THRESHOLD);

    // create with zero fat reserves
    const double BC_DEAD = 0.0;  // body condition
    HerbivoreIndividual ind(AGE, BC_DEAD, &hft, SEX_MALE, AREA);

    // after one simulation day it should be dead
    double offspring_dump;   // ignored
    HabitatEnvironment env;  // ignored
    ind.simulate_day(0, env, offspring_dump);
    CHECK(ind.is_dead());
  }
  // NOTE: We cannot test mortality because it is a stochastic
  // event.
}

