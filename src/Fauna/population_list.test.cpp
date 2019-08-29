#include "catch.hpp"
#include "dummy_hft.h"
#include "dummy_population.h"
#include "population_list.h"

using namespace Fauna;

TEST_CASE("Fauna::PopulationList", "") {
  PopulationList list;
  static const int NPOP = 3;     // populations count
  static const int NHERBIS = 5;  // herbivores count

  // create some HFTs
  Hft hfts[NPOP];
  hfts[0].name = "hft1";
  hfts[1].name = "hft2";
  hfts[2].name = "hft3";

  // population below `minimum_density_threshold`
  const int DEAD_HFT_ID = 1;

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
    PopulationInterface* new_pop = new DummyPopulation(&hfts[i]);
    pops[i] = (DummyPopulation*)new_pop;

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

    // add them to the list -> transfer ownership
    list.add(new_pop);
  }

  // Check if all populations and herbivores have been added.
  REQUIRE(list.get_all_herbivores().size() == NPOP * NHERBIS);

  // Add NULL pointer.
  CHECK_THROWS(list.add(NULL));

  // HFT already exists.
  CHECK_THROWS(list.add(new DummyPopulation(&hfts[0])));

  // Check that all herbivores are there.
  HerbivoreVector all = list.get_all_herbivores();
  for (HerbivoreVector::iterator itr = all.begin(); itr != all.end(); itr++) {
    const HerbivoreInterface& herbiv = **itr;
    // check if herbivore access works (that no bad memory
    // access is thrown or so)
    herbiv.get_ind_per_km2();
  }

  // check random access
  for (int i = 0; i < NPOP; i++) CHECK(&list.get(hfts[i]) == pops[i]);
  CHECK_THROWS(list.get(Hft()));  // unnamed Hft is not in map

  SECTION("Kill population below threshold") {
    // Kill all herbivores of the one population below threshold.
    PopulationInterface& dead_pop = list.get(hfts[DEAD_HFT_ID]);

    // Reduce density of all herbivores in the one population
    HerbivoreVector herbivores_to_kill = dead_pop.get_list();
    for (HerbivoreVector::iterator h_itr = herbivores_to_kill.begin();
         h_itr != herbivores_to_kill.end(); h_itr++)
      ((DummyHerbivore*)(*h_itr))->ind_per_km2 = 0.0000001;

    // Mark those herbivores as killed.
    list.kill_nonviable();

    // the population object is still there
    CHECK_NOTHROW(list.get(hfts[DEAD_HFT_ID]));

    // Check the other (surviving) populations.
    for (auto& hft : hfts) {
      const PopulationInterface& pop = list.get(hft);
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
