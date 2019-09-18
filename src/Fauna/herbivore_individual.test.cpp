/**
 * \file
 * \brief Unit test for Fauna::HerbivoreIndividual.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "dummy_hft.h"
#include "environment.h"
#include "herbivore_individual.h"
#include "parameters.h"
using namespace Fauna;

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
  CHECK_THROWS(HerbivoreIndividual(AGE, BC, &hft, Sex::Male, -1.0));
  CHECK_THROWS(HerbivoreIndividual(AGE, BC, &hft, Sex::Male, 0.0));
  CHECK_THROWS(HerbivoreIndividual(&hft, Sex::Male, -1.0));
  CHECK_THROWS(HerbivoreIndividual(&hft, Sex::Male, 0.0));

  // birth constructor
  REQUIRE(HerbivoreIndividual(&hft, Sex::Male, AREA).get_area_km2() ==
          Approx(AREA));
  // establishment constructor
  REQUIRE(HerbivoreIndividual(AGE, BC, &hft, Sex::Male, AREA).get_area_km2() ==
          Approx(AREA));

  SECTION("Mortality") {
    hft.mortality.factors.insert(MortalityFactor::StarvationThreshold);

    // create with zero fat reserves
    const double BC_DEAD = 0.0;  // body condition
    HerbivoreIndividual ind(AGE, BC_DEAD, &hft, Sex::Male, AREA);

    // after one simulation day it should be dead
    double offspring_dump;   // ignored
    HabitatEnvironment env;  // ignored
    ind.simulate_day(0, env, offspring_dump);
    CHECK(ind.is_dead());
  }
  // NOTE: We cannot test mortality because it is a stochastic
  // event.
}

