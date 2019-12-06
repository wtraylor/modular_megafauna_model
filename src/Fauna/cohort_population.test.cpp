/**
 * \file
 * \brief Unit test for Fauna::CohortPopulation.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "cohort_population.h"
#include "dummy_hft.h"
#include "environment.h"
#include "herbivore_cohort.h"
#include "hft.h"
#include "parameters.h"
#include "population_lists_match.h"

using namespace Fauna;

TEST_CASE("Fauna::CohortPopulation", "") {
  // prepare parameters
  std::shared_ptr<Parameters> params(new Parameters());
  REQUIRE(params->is_valid());

  // Prepare HFT by copying the dummy and modifying it.
  auto hft = std::shared_ptr<Hft>(new Hft(*create_hfts(1, *params)[0]));
  hft->establishment_density = 10.0;  // [ind/km²]
  hft->mortality_factors.clear();     // immortal herbivores
  REQUIRE(hft->is_valid(*params));

  // prepare creating object
  CreateHerbivoreCohort create_cohort(hft, params);

  // create cohort population
  CohortPopulation pop(create_cohort);
  REQUIRE(pop.get_list().empty());
  REQUIRE(population_lists_match(pop));
  REQUIRE(pop.get_hft() == *hft);

  CHECK_THROWS(pop.create_offspring(-1.0));

  SECTION("Establishment") {
    REQUIRE(pop.get_list().empty());  // empty before

    SECTION("Establish one age class") {
      hft->establishment_age_range.first = hft->establishment_age_range.second =
          4;
      pop.establish();
      REQUIRE(!pop.get_list().empty());  // filled afterwards
      REQUIRE(population_lists_match(pop));

      // There should be only one age class with male and female
      REQUIRE(pop.get_list().size() == 2);

      // Does the total density match?
      REQUIRE(pop.get_ind_per_km2() == Approx(hft->establishment_density));
    }

    SECTION("Establish several age classes") {
      hft->establishment_age_range.first = 3;
      hft->establishment_age_range.second = 6;
      pop.establish();
      REQUIRE(!pop.get_list().empty());  // filled afterwards
      REQUIRE(population_lists_match(pop));

      // There should be 2 cohorts per year in the age range.
      REQUIRE(pop.get_list().size() == 4 * 2);

      // Does the total density match?
      REQUIRE(pop.get_ind_per_km2() == Approx(hft->establishment_density));
    }

    SECTION("Removal of dead cohorts with mortality") {
      // we will kill all herbivores in the list with a copy
      // assignment trick

      // Let them die ...
      HerbivoreVector vec = pop.get_list();
      const int old_count = vec.size();
      // call birth constructor with zero density
      static const ForageEnergyContent ME = Parameters().metabolizable_energy;
      HerbivoreCohort dead(hft, Sex::Female, 0.0, ME);
      for (HerbivoreVector::iterator itr = vec.begin(); itr != vec.end();
           itr++) {
        HerbivoreInterface* pint = *itr;
        HerbivoreCohort* pcohort = (HerbivoreCohort*)pint;
        pcohort->operator=(dead);
        REQUIRE(pcohort->get_ind_per_km2() == 0.0);
      }
      // now they should be all dead

      // So far, the list shouldn’t have changed. It still includes the
      // dead cohorts.
      CHECK(population_lists_match(pop));
      CHECK(old_count == pop.get_list().size());

      // Check that each cohort is really dead.
      HerbivoreVector dead_vec = pop.get_list();
      for (HerbivoreVector::const_iterator itr = vec.begin(); itr != vec.end();
           itr++) {
        CHECK((*itr)->is_dead());
      }

      // Now delete all dead cohorts
      pop.purge_of_dead();
      CHECK(pop.get_list().size() == 0);
    }
  }

  SECTION("Offspring with enough density") {
    const double DENS = 10.0;  // offspring density [ind/km²]
    INFO("DENS = " << DENS);
    pop.create_offspring(DENS);

    // There should be only one age class with male and female
    REQUIRE(pop.get_list().size() == 2);
    CHECK(population_lists_match(pop));
    // Does the total density match?
    REQUIRE(pop.get_ind_per_km2() == Approx(DENS));

    // simulate one day
    HerbivoreVector list = pop.get_list();
    double offspring_dump;   // ignored
    HabitatEnvironment env;  // ignored
    for (HerbivoreVector::iterator itr = list.begin(); itr != list.end(); itr++)
      (*itr)->simulate_day(0, env, offspring_dump);

    // add more offspring
    pop.create_offspring(DENS);
    // This must be in the same age class even though we advanced one day.
    REQUIRE(pop.get_list().size() == 2);
    CHECK(population_lists_match(pop));
    REQUIRE(pop.get_ind_per_km2() == Approx(2.0 * DENS));

    // let the herbivores age (they are immortal)
    for (int i = 1; i < 365; i++) {
      HerbivoreVector::iterator itr;
      HerbivoreVector list = pop.get_list();
      double offspring_dump;   // ignored
      HabitatEnvironment env;  // ignored
      for (itr = list.begin(); itr != list.end(); itr++)
        (*itr)->simulate_day(i, env, offspring_dump);
    }
    // now they should have grown older, and if we add more
    // offspring, there should be new age classes
    pop.create_offspring(DENS);
    CHECK(pop.get_list().size() == 4);
    CHECK(population_lists_match(pop));
    REQUIRE(pop.get_ind_per_km2() == Approx(3.0 * DENS));
  }
}
