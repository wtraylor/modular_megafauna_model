#include "catch.hpp"
#include "mortality_factors.h"
using namespace Fauna;

TEST_CASE("Fauna::GetBackgroundMortality", "") {
  CHECK_THROWS(GetBackgroundMortality(0.0, -1.0));
  CHECK_THROWS(GetBackgroundMortality(1.0, 0.0));
  CHECK_THROWS(GetBackgroundMortality(-1.0, -1.0));
  CHECK_THROWS(GetBackgroundMortality(-1.0, 0.0));
  CHECK_THROWS(GetBackgroundMortality(0.0, 1.1));

  const GetBackgroundMortality get_zero(0.0, 0.0);
  CHECK(get_zero(0) == 0.0);
  CHECK(get_zero(1 * 365) == 0.0);
  CHECK(get_zero(4 * 365) == 0.0);

  const double JUV = 0.3;
  const double ADULT = 0.1;
  const GetBackgroundMortality get_mort(JUV, ADULT);
  CHECK_THROWS(get_mort(-1));
  REQUIRE(get_mort(1) > 0.0);

  // Check that the daily mortality matches the annual one.
  double surviving_juveniles = 1.0;
  for (int d = 0; d < 365; d++) surviving_juveniles *= (1.0 - get_mort(d));
  CHECK(surviving_juveniles == Approx(1.0 - JUV));

  // Check that the daily mortality matches the annual one.
  double surviving_adults = 1.0;
  for (int d = 365; d < 2 * 365; d++) surviving_adults *= (1.0 - get_mort(d));
  CHECK(surviving_adults == Approx(1.0 - ADULT));
}

TEST_CASE("Fauna::GetSimpleLifespanMortality", "") {
  CHECK_THROWS(GetSimpleLifespanMortality(-1));
  CHECK_THROWS(GetSimpleLifespanMortality(0));
  const int LIFESPAN = 20;
  const GetSimpleLifespanMortality get_mort(LIFESPAN);
  CHECK_THROWS(get_mort(-1));
  // some arbitrary numbers
  CHECK(get_mort(0) == 0.0);
  CHECK(get_mort(40) == 0.0);
  CHECK(get_mort(3 * 365) == 0.0);
  CHECK(get_mort(LIFESPAN * 365 - 1) == 0.0);
  CHECK(get_mort(LIFESPAN * 365) == 1.0);
  CHECK(get_mort(LIFESPAN * 365 + 10) == 1.0);
  CHECK(get_mort((LIFESPAN + 1) * 365) == 1.0);
}

TEST_CASE("Fauna::GetStarvationIlliusOConnor2000", "") {
  CHECK_THROWS(GetStarvationIlliusOConnor2000(-0.1));
  CHECK_THROWS(GetStarvationIlliusOConnor2000(1.1));

  double new_bc, new_bc1, new_bc2,
      new_bc3;  // variables to store new body condition

  SECTION("default standard deviation") {
    const GetStarvationIlliusOConnor2000 get_mort(
        0.125,
        true);  // yes, shift body condition
    CHECK_THROWS(get_mort(-1.0, new_bc));
    CHECK_THROWS(get_mort(1.1, new_bc));

    // With full fat reserves there shouldn’t be any considerable
    // mortality
    CHECK(get_mort(1.0, new_bc) == Approx(0.0).margin(0.001));
    CHECK(new_bc == Approx(1.0));

    // Mortality increases with lower body condition.
    const double mort1 = get_mort(.01, new_bc1);
    const double mort2 = get_mort(.1, new_bc2);
    CHECK(mort1 > mort2);
    CHECK(new_bc1 > 0.01);
    CHECK(new_bc2 > 0.1);

    // The change in body condition peaks around a body condition of 0.1
    // if standard deviation is 0.125
    INFO("new_bc1 = " << new_bc1);
    INFO("mort1 = " << mort1);
    INFO("new_bc2 = " << new_bc2);
    INFO("mort2 = " << mort2);
    CHECK(new_bc1 - .01 < new_bc2 - .1);
    get_mort(.2, new_bc3);
    CHECK(new_bc3 > .2);
    CHECK(new_bc2 - .1 > new_bc3 - .2);

    // Because of the symmetry of the normal distribution,
    // only half of the population actually falls below zero
    // fat reserves if the average is zero.
    CHECK(get_mort(0.0, new_bc) == Approx(0.5));
    // ... but the average body condition does not increase
    CHECK(new_bc == Approx(0.0));
  }

  SECTION("compare standard deviations") {
    const GetStarvationIlliusOConnor2000 get_mort1(0.1);
    const GetStarvationIlliusOConnor2000 get_mort2(0.3);
    const double mort1 = get_mort1(.1, new_bc1);
    const double mort2 = get_mort2(.2, new_bc2);
    CHECK(mort1 < mort2);
    CHECK(new_bc1 < new_bc2);
  }
}

TEST_CASE("Fauna::GetStarvationMortalityThreshold", "") {
  CHECK_THROWS(GetStarvationMortalityThreshold(-0.1));
  CHECK_THROWS(GetStarvationMortalityThreshold(1.1));
  SECTION("default threshold") {
    GetStarvationMortalityThreshold get_mort;
    CHECK_THROWS(get_mort(-0.1));
    CHECK_THROWS(get_mort(1.1));
    CHECK(get_mort(0.0) == 1.0);
    CHECK(get_mort(0.1) == 0.0);
  }
  SECTION("custom threshold") {
    GetStarvationMortalityThreshold get_mort(0.05);
    CHECK(get_mort(0.0) == 1.0);
    CHECK(get_mort(0.04) == 1.0);
    CHECK(get_mort(0.05) == 0.0);
    CHECK(get_mort(0.06) == 0.0);
  }
}

