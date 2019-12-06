/**
 * \file
 * \brief Unit test for Fauna::IndividualPopulation.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "dummy_hft.h"
#include "environment.h"
#include "herbivore_individual.h"
#include "hft.h"
#include "individual_population.h"
#include "parameters.h"
#include "population_lists_match.h"

using namespace Fauna;

TEST_CASE("Fauna::IndividualPopulation", "") {
  const double AREA = 10.0;  // habitat area [km²]
  // prepare parameters
  Parameters params;
  params.habitat_area_km2 = AREA;
  REQUIRE(params.is_valid());

  // prepare HFT
  const int ESTABLISH_COUNT = 100;  // [ind]
  Hft hft = create_hfts(1, params)[0];
  hft.establishment_density = ESTABLISH_COUNT / AREA;  // [ind/km²]
  hft.mortality_factors.clear();                       // immortal herbivores
  REQUIRE(hft.is_valid(params));

  // prepare creating object
  CreateHerbivoreIndividual create_ind(&hft, &params);

  IndividualPopulation pop(create_ind);

  SECTION("Exceptions") { CHECK_THROWS(pop.create_offspring(-1.0)); }

  SECTION("Create empty population") {
    REQUIRE(pop.get_list().empty());
    REQUIRE(population_lists_match(pop));
    REQUIRE(pop.get_hft() == hft);
  }

  SECTION("Establishment") {
    pop.establish();
    REQUIRE(!pop.get_list().empty());
    CHECK(population_lists_match(pop));
    // Do we have the exact number of individuals?
    CHECK(pop.get_list().size() == ESTABLISH_COUNT);
    // Does the total density match?
    CHECK(pop.get_ind_per_km2() == Approx(hft.establishment_density));

    SECTION("Removal of dead individuals") {
      // kill all herbivores in the list with a copy assignment
      // trick
      hft.mortality_factors.insert(MortalityFactor::StarvationThreshold);
      // create a dead individual
      const int AGE = 10;
      const double BC = 0.0;  // starved to death!
      const double AREA = 10.0;
      HerbivoreIndividual dead(AGE, BC, &hft, Sex::Female, AREA,
                               Parameters().metabolizable_energy);
      double offspring_dump;
      HabitatEnvironment env;
      dead.simulate_day(0, env, offspring_dump);
      REQUIRE(dead.is_dead());

      SECTION("Kill only one individual") {
        HerbivoreIndividual* pind =
            (HerbivoreIndividual*)*pop.get_list().begin();
        pind->operator=(dead);
        REQUIRE(pind->is_dead());

        // No change yet.
        CHECK(pop.get_list().size() == ESTABLISH_COUNT);

        // We purge and should have one object less.
        pop.purge_of_dead();
        CHECK(pop.get_list().size() == ESTABLISH_COUNT - 1);
      }

      SECTION("Kill ALL individuals") {
        // copy assign it to every ind. in the list
        {
          HerbivoreVector list = pop.get_list();
          for (HerbivoreVector::iterator itr = list.begin(); itr != list.end();
               itr++) {
            HerbivoreInterface* pint = *itr;
            HerbivoreIndividual* pind = (HerbivoreIndividual*)pint;
            pind->operator=(dead);
            REQUIRE(pind->is_dead());
          }
        }

        // Now the list should contain only dead herbivores. Nothing was
        // deleted yet.
        CHECK(pop.get_list().size() == ESTABLISH_COUNT);

        // If we now delete the dead ones, we should have an empty list.
        pop.purge_of_dead();
        CHECK(pop.get_list().empty());
      }
    }
  }

  SECTION("Complete offspring") {
    // Here, we create a discrete number of individuals.

    // Integer, even number of individuals
    const int IND_COUNT = 10;  // [ind]

    const double IND_DENS = IND_COUNT / AREA;

    pop.create_offspring(IND_DENS);
    REQUIRE(pop.get_list().size() == IND_COUNT);
    CHECK(population_lists_match(pop));
    REQUIRE(pop.get_ind_per_km2() == Approx(IND_DENS));

    // add more offspring
    pop.create_offspring(IND_DENS);
    REQUIRE(pop.get_list().size() == 2 * IND_COUNT);
    CHECK(population_lists_match(pop));
    REQUIRE(pop.get_ind_per_km2() == Approx(2.0 * IND_DENS));
  }

  SECTION("Incomplete offspring") {
    // Here, we create offspring with non-integer individual counts.

    // Try to create offspring. It shouldn’t create anything since there
    // is no complete individual.
    pop.create_offspring(.4 / AREA);  // .4 individuals
    REQUIRE(pop.get_list().size() == 0);
    CHECK(population_lists_match(pop));
    REQUIRE(pop.get_ind_per_km2() == Approx(0.0));

    // Try it again, but it should still not work.
    pop.create_offspring(.4 / AREA);  // .8 individuals
    REQUIRE(pop.get_list().size() == 0);
    CHECK(population_lists_match(pop));
    REQUIRE(pop.get_ind_per_km2() == Approx(0.0));

    // Now we should get above a sum of 1.0, BUT males and females are
    // created in parallel, so they shouldn’t be created until total
    // offspring reaches a number of at least TWO individuals.
    pop.create_offspring(.4 / AREA);  // 1.2 individuals
    REQUIRE(pop.get_list().size() == 0);
    CHECK(population_lists_match(pop));
    REQUIRE(pop.get_ind_per_km2() == Approx(0.0));

    // Finally, we have 2 individuals complete.
    pop.create_offspring(.9 / AREA);  // 2.1 individuals
    REQUIRE(pop.get_list().size() == 2);
    CHECK(population_lists_match(pop));
    REQUIRE(pop.get_ind_per_km2() == Approx(2.0 / AREA));
  }
}
