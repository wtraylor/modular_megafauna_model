#include "catch.hpp"
#include "dummy_hft.h"
#include "dummy_population.h"
#include "habitat.h"
#include "hft.h"
#include "parameters.h"
#include "population.h"
using namespace Fauna;

namespace {
/// \brief Check if the lengths of the modifiable and the
/// read-only population vectors match.
bool population_lists_match(PopulationInterface& pop) {
  // FIRST the read-only -> no chance for the population
  // object to change the list.
  ConstHerbivoreVector readonly = ((const PopulationInterface&)pop).get_list();
  HerbivoreVector modifiable = pop.get_list();
  return modifiable.size() == readonly.size();
}
}  // namespace

TEST_CASE("Fauna::CohortPopulation", "") {
  // prepare parameters
  Parameters params;
  REQUIRE(params.is_valid());

  // prepare HFT
  Hft hft = create_hfts(1, params)[0];
  hft.establishment_density = 10.0;  // [ind/km²]
  hft.mortality_factors.clear();     // immortal herbivores
  REQUIRE(hft.is_valid(params));

  // prepare creating object
  CreateHerbivoreCohort create_cohort(&hft, &params);

  // create cohort population
  CohortPopulation pop(create_cohort);
  REQUIRE(pop.get_list().empty());
  REQUIRE(population_lists_match(pop));
  REQUIRE(pop.get_hft() == hft);

  CHECK_THROWS(pop.create_offspring(-1.0));

  SECTION("Establishment") {
    REQUIRE(pop.get_list().empty());  // empty before

    SECTION("Establish one age class") {
      hft.establishment_age_range.first = hft.establishment_age_range.second =
          4;
      pop.establish();
      REQUIRE(!pop.get_list().empty());  // filled afterwards
      REQUIRE(population_lists_match(pop));

      // There should be only one age class with male and female
      REQUIRE(pop.get_list().size() == 2);

      // Does the total density match?
      REQUIRE(pop.get_ind_per_km2() == Approx(hft.establishment_density));
    }

    SECTION("Establish several age classes") {
      hft.establishment_age_range.first = 3;
      hft.establishment_age_range.second = 6;
      pop.establish();
      REQUIRE(!pop.get_list().empty());  // filled afterwards
      REQUIRE(population_lists_match(pop));

      // There should be 2 cohorts per year in the age range.
      REQUIRE(pop.get_list().size() == 4 * 2);

      // Does the total density match?
      REQUIRE(pop.get_ind_per_km2() == Approx(hft.establishment_density));
    }

    SECTION("Removal of dead cohorts with mortality") {
      // we will kill all herbivores in the list with a copy
      // assignment trick

      // Let them die ...
      HerbivoreVector vec = pop.get_list();
      const int old_count = vec.size();
      // call birth constructor with zero density
      HerbivoreCohort dead(&hft, SEX_FEMALE, 0.0);
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

TEST_CASE("Fauna::HftPopulationsMap", "") {
  HftPopulationsMap map;
  const int NPOP = 3;     // populations count
  const int NHERBIS = 5;  // herbivores count

  // create some HFTs
  Hft hfts[NPOP];
  hfts[0].name = "hft1";
  hfts[1].name = "hft2";
  hfts[2].name = "hft3";
  const int DEAD_HFT_ID = 1;  // population below `minimum_density_threshold`
  for (int i = 0; i < NPOP; i++) {
    hfts[i].establishment_density = (double)i + 1.0;
    // have only one age class established:
    hfts[i].establishment_age_range.first =
        hfts[i].establishment_age_range.second;
    // First, have every populatien be above the minimum threshold.
    hfts[i].minimum_density_threshold = 2 * i;
  }
  // Now let *one* population have a very low threshold.
  // It will be deleted later.
  hfts[DEAD_HFT_ID].minimum_density_threshold = 0.01;

  // create some populations with establishment_density
  DummyPopulation* pops[NPOP];
  for (int i = 0; i < NPOP; i++) {
    // Create population object
    std::auto_ptr<PopulationInterface> new_pop(new DummyPopulation(&hfts[i]));
    pops[i] = (DummyPopulation*)new_pop.get();

    // Create herbivores
    for (int j = 0; j < NHERBIS; j++) pops[i]->establish();

    // Check that all HFT populations have been created.
    REQUIRE(pops[i]->get_list().size() == NHERBIS);

    // Check that each HFT population has the expected density.
    double pop_dens = 0.0;
    const HerbivoreVector herbi_list = pops[i]->get_list();
    for (HerbivoreVector::const_iterator itr = herbi_list.begin();
         itr != herbi_list.end(); itr++)
      pop_dens += (**itr).get_ind_per_km2();
    REQUIRE(pop_dens == Approx(NHERBIS * hfts[i].establishment_density));

    // add them to the map -> transfer ownership
    map.add(new_pop);
  }

  // Check if all populations and herbivores have been added.
  REQUIRE(map.size() == NPOP);
  REQUIRE(map.get_all_herbivores().size() == NPOP * NHERBIS);

  // throw some exceptions
  CHECK_THROWS(map.add(std::auto_ptr<PopulationInterface>()));
  CHECK_THROWS(map.add(std::auto_ptr<PopulationInterface>(
      new DummyPopulation(&hfts[0]))));  // HFT already exists

  // check if iterator access works
  // ... for populations
  HftPopulationsMap::const_iterator itr = map.begin();
  while (itr != map.end()) {
    const PopulationInterface& pop = **itr;
    bool found = false;
    // check against HFTs (order in the map is not defined)
    for (int i = 0; i < NPOP; i++)
      if (pop.get_hft() == hfts[i]) {
        found = true;
        const HerbivoreInterface& herbiv = **pop.get_list().begin();
        // check if herbivore access works (that no bad memory
        // access is thrown or so)
        herbiv.get_ind_per_km2();
      }
    CHECK(found);
    itr++;
  }
  // ... for herbivore list
  HerbivoreVector all = map.get_all_herbivores();
  for (HerbivoreVector::iterator itr = all.begin(); itr != all.end(); itr++) {
    const HerbivoreInterface& herbiv = **itr;
    // check if herbivore access works (that no bad memory
    // access is thrown or so)
    herbiv.get_ind_per_km2();
  }

  // check random access
  for (int i = 0; i < NPOP; i++) CHECK(&map[hfts[i]] == pops[i]);
  CHECK_THROWS(map[Hft()]);  // unnamed Hft is not in map

  SECTION("Kill population below threshold") {
    // Kill all herbivores of the one population below threshold.
    PopulationInterface& dead_pop = map[hfts[DEAD_HFT_ID]];

    // Reduce density of all herbivores in the one population
    HerbivoreVector herbivores_to_kill = dead_pop.get_list();
    for (HerbivoreVector::iterator h_itr = herbivores_to_kill.begin();
         h_itr != herbivores_to_kill.end(); h_itr++)
      ((DummyHerbivore*)(*h_itr))->ind_per_km2 = 0.0000001;

    // Mark those herbivores as killed.
    map.kill_nonviable();

    CHECK(map.size() == NPOP);  // the population object is still there

    // Check the other (surviving) populations.
    itr = map.begin();
    for (itr = map.begin(); itr != map.end(); itr++) {
      const PopulationInterface& pop = **itr;
      if (pop.get_hft() != hfts[DEAD_HFT_ID]) {
        // The populations above threshold are not affected.
        CHECK(pop.get_list().size() == NHERBIS);
        CHECK(pop.get_ind_per_km2() > 0.0);
        CHECK(pop.get_kg_per_km2() > 0.0);
        ConstHerbivoreVector herbiv_vec = pop.get_list();
        for (ConstHerbivoreVector::iterator h_itr = herbiv_vec.begin();
             h_itr != herbiv_vec.end(); h_itr++)
          CHECK(!(**h_itr).is_dead());
      }
    }

    // The population below the threshold should have only killed herbivores.
    CHECK(dead_pop.get_list().size() == NHERBIS);
    CHECK(dead_pop.get_ind_per_km2() == 0.0);
    CHECK(dead_pop.get_kg_per_km2() == 0.0);
    HerbivoreVector herbiv_vec = dead_pop.get_list();
    for (HerbivoreVector::const_iterator h_itr = herbiv_vec.begin();
         h_itr != herbiv_vec.end(); h_itr++)
      CHECK((**h_itr).is_dead());
  }
}

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
      hft.mortality_factors.insert(MF_STARVATION_THRESHOLD);
      // create a dead individual
      const int AGE = 10;
      const double BC = 0.0;  // starved to death!
      const double AREA = 10.0;
      HerbivoreIndividual dead(AGE, BC, &hft, SEX_FEMALE, AREA);
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
