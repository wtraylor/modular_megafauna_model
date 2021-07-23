// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::WorldConstructor.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "world_constructor.h"
#include "catch.hpp"
#include "cohort_population.h"
#include "dummy_habitat.h"
#include "dummy_hft.h"
#include "parameters.h"
#include "population_interface.h"
#include "population_list.h"
#include "simulation_unit.h"
using namespace Fauna;

TEST_CASE("Fauna::WorldConstructor", "") {
  std::shared_ptr<Parameters> params(new Parameters);
  REQUIRE(params->is_valid());

  // prepare HFT list
  static const HftList HFTLIST = *create_hfts(3, *params);

  SECTION("create_populations() for several HFTs in one habitat") {
    REQUIRE(params->herbivore_type == HerbivoreType::Cohort);
    REQUIRE(params->one_hft_per_habitat == false);
    WorldConstructor world_cons(params, HFTLIST);
    PopulationList* pops = world_cons.create_populations(0);
    REQUIRE(pops != NULL);
    // Check that there is one population per HFT.
    for (auto& hft : HFTLIST) {
      bool hft_found = false;
      for (auto& pop : *pops) {
        REQUIRE(pop);
        if (&((CohortPopulation*)pop.get())->get_hft() == hft.get()) {
          REQUIRE(!hft_found);  // An HFT shouldn’t be found twice.
          hft_found = true;
        }
      }
      CHECK(hft_found);
    }
  }

  SECTION("create_populations() for one HFT per habitat") {
    REQUIRE(params->herbivore_type == HerbivoreType::Cohort);
    params->one_hft_per_habitat = true;
    WorldConstructor world_cons(params, HFTLIST);
    for (int i = 0; i < 3 * HFTLIST.size(); i++) {
      const PopulationList* pops = world_cons.create_populations(i);
      REQUIRE(pops != NULL);
      REQUIRE(pops->size() == 1);  // Only 1 HFT per habitat!
      // Check that the *right* HFT is in the populations list.
      const CohortPopulation* pop = (CohortPopulation*)(pops->front().get());
      const Hft& found_hft = pop->get_hft();
      const int hft_index = i % HFTLIST.size();
      REQUIRE(hft_index >= 0);
      REQUIRE(hft_index < HFTLIST.size());
      const Hft& right_hft = *HFTLIST[hft_index];
      REQUIRE(found_hft == right_hft);
    }
  }

  SECTION("create_populations() for one HFT") {
    WorldConstructor world_cons(params, HFTLIST);
    REQUIRE(params->herbivore_type == HerbivoreType::Cohort);
    const auto hft = HFTLIST[0];
    const PopulationList* pops = world_cons.create_populations(0);
    REQUIRE(pops != NULL);
    // Check that the CohortPopulation is of the given HFT.
    CHECK(&((CohortPopulation*)pops->begin()->get())->get_hft() == hft.get());
  }
}
