#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "energetics.h"
using namespace Fauna;

TEST_CASE("Fauna::FatmassEnergyBudget", "") {
  CHECK_THROWS(FatmassEnergyBudget(-1.0, 1.0));
  CHECK_THROWS(FatmassEnergyBudget(0.0, 0.0));
  CHECK_THROWS(FatmassEnergyBudget(0.0, -1.0));
  CHECK_THROWS(FatmassEnergyBudget(1.1, 1.0));

  const double INIT_FATMASS = 1.0;  // initial fat mass
  const double MAX_FATMASS = 2.0;   // maximim fat mass
  FatmassEnergyBudget budget(INIT_FATMASS, MAX_FATMASS);

  // Initialization
  REQUIRE(budget.get_fatmass() == INIT_FATMASS);
  REQUIRE(budget.get_energy_needs() == 0.0);
  REQUIRE(budget.get_max_anabolism_per_day() ==
          Approx(54.6 * (MAX_FATMASS - INIT_FATMASS)));

  // exceptions
  CHECK_THROWS(budget.metabolize_energy(-1.0));
  CHECK_THROWS(budget.metabolize_energy(1000000.0));
  CHECK_THROWS(budget.add_energy_needs(-1.0));
  CHECK_THROWS(budget.set_max_fatmass(INIT_FATMASS / 2.0, .1));
  CHECK_THROWS(budget.set_max_fatmass(-1.0, .1));
  CHECK_THROWS(budget.set_max_fatmass(INIT_FATMASS, -.1));

  const double ENERGY = 10.0;  // MJ

  SECTION("force_body_condition()") {
    CHECK_THROWS(budget.force_body_condition(-.1));
    CHECK_THROWS(budget.force_body_condition(1.1));
    budget.force_body_condition(0.3);
    CHECK(budget.get_fatmass() / budget.get_max_fatmass() == Approx(0.3));
  }

  SECTION("Set energy needs") {
    budget.add_energy_needs(ENERGY);
    REQUIRE(budget.get_energy_needs() == Approx(ENERGY));
    budget.add_energy_needs(ENERGY);
    REQUIRE(budget.get_energy_needs() == Approx(2.0 * ENERGY));
  }

  SECTION("Anabolism") {
    budget.metabolize_energy(ENERGY);
    CHECK(budget.get_fatmass() > INIT_FATMASS);

    // Check the number with coefficient of Blaxter (1989)
    CHECK(budget.get_fatmass() == Approx(INIT_FATMASS + ENERGY / 54.6));
  }

  SECTION("Anabolism Limit") {
    const double anabolism_unlimited = budget.get_max_anabolism_per_day();

    // Setting maximum gain to zero means no limits.
    budget.set_max_fatmass(MAX_FATMASS, 0.0);
    CHECK(budget.get_max_anabolism_per_day() == Approx(anabolism_unlimited));

    // Set maximum gain to half of the gap towards maximum fat mass.
    const double MAX_GAIN = (MAX_FATMASS - INIT_FATMASS) / 2.0;
    budget.set_max_fatmass(MAX_FATMASS, MAX_GAIN);

    CHECK(budget.get_max_anabolism_per_day() ==
          Approx(anabolism_unlimited / 2.0));
  }

  SECTION("Catabolism") {
    budget.add_energy_needs(ENERGY);
    budget.catabolize_fat();
    CHECK(budget.get_fatmass() < INIT_FATMASS);

    // Check the number with coefficient of Blaxter (1989)
    CHECK(budget.get_fatmass() == Approx(INIT_FATMASS - ENERGY / 39.3));
  }

  SECTION("Metabolism") {
    budget.add_energy_needs(ENERGY);
    REQUIRE(budget.get_energy_needs() == ENERGY);
    budget.metabolize_energy(ENERGY);
    CHECK(budget.get_energy_needs() == Approx(0.0));
  }

  SECTION("Metabolism and Anabolism") {
    budget.add_energy_needs(ENERGY / 2.0);
    REQUIRE(budget.get_energy_needs() == Approx(ENERGY / 2.0));
    budget.metabolize_energy(ENERGY);
    CHECK(budget.get_energy_needs() == 0.0);
    CHECK(budget.get_fatmass() > INIT_FATMASS);
    CHECK(budget.get_fatmass() < MAX_FATMASS);
  }

  SECTION("Merge") {
    budget.add_energy_needs(ENERGY);
    const double OTHER_FATMASS = 3.0;
    const double OTHER_MAX_FATMASS = 4.0;
    const double OTHER_ENERGY = 13.0;
    FatmassEnergyBudget other(OTHER_FATMASS, OTHER_MAX_FATMASS);
    other.add_energy_needs(OTHER_ENERGY);

    SECTION("Merge with equal weight") {
      budget.merge(other, 1.0, 1.0);
      CHECK(budget.get_energy_needs() == Approx((ENERGY + OTHER_ENERGY) / 2.0));
      CHECK(budget.get_max_fatmass() ==
            Approx((MAX_FATMASS + OTHER_MAX_FATMASS) / 2.0));
      CHECK(budget.get_fatmass() ==
            Approx((INIT_FATMASS + OTHER_FATMASS) / 2.0));
    }

    SECTION("Merge with different weight") {
      const double W1 = 0.4;
      const double W2 = 1.2;
      budget.merge(other, W1, W2);
      CHECK(budget.get_energy_needs() ==
            Approx((ENERGY * W1 + OTHER_ENERGY * W2) / (W1 + W2)));
      CHECK(budget.get_max_fatmass() ==
            Approx((MAX_FATMASS * W1 + OTHER_MAX_FATMASS * W2) / (W1 + W2)));
      CHECK(budget.get_fatmass() ==
            Approx((INIT_FATMASS * W1 + OTHER_FATMASS * W2) / (W1 + W2)));
    }
  }
}

TEST_CASE("Fauna::get_expenditure_taylor_1981()") {
  const double CURRENT = 90;  // [kg]
  const double ADULT = 100;   // [kg]
  CHECK(get_expenditure_taylor_1981(CURRENT, ADULT) ==
        Approx(0.4 * CURRENT * pow(ADULT, -0.27)));
}

TEST_CASE("Fauna::get_expenditure_zhu_et_al_2018()") {
  const double M = 100;  // [kg]
  const double T = 20;   // [°C]
  CHECK_THROWS(get_expenditure_zhu_et_al_2018(-4, T));
  CHECK_THROWS(get_expenditure_zhu_et_al_2018(0, T));
  // Check formula:
  CHECK(get_expenditure_zhu_et_al_2018(M, T) ==
        0.36 / exp(0.0079 * T) * pow(M, 0.75));
  // More energy costs in lower temperature:
  CHECK(get_expenditure_zhu_et_al_2018(M, T) <
        get_expenditure_zhu_et_al_2018(M, T - 20));
}



