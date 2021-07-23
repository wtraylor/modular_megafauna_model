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
  HftList hftlist = *create_hfts(3, *params);

  SECTION("create_populations() for several HFTs in one habitat") {
    REQUIRE(params->herbivore_type == HerbivoreType::Cohort);
    REQUIRE(params->one_hft_per_habitat == false);
    WorldConstructor world_cons(params, hftlist);
    PopulationList* pops = world_cons.create_populations(0);
    REQUIRE(pops != NULL);
    // Check that there is one population per HFT.
    for (auto& hft : hftlist) {
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
    WorldConstructor world_cons(params, hftlist);
    for (int i = 0; i < 3 * hftlist.size(); i++) {
      PopulationList* pops = world_cons.create_populations(i);
      REQUIRE(pops != NULL);
      REQUIRE(pops->size() == 1);  // Only 1 HFT per habitat!
      // Check that the *right* HFT is in the populations list.
      const CohortPopulation& pop = (CohortPopulation&)pops->front();
      const Hft& found_hft = pop.get_hft();
      const Hft& right_hft = *hftlist[i % hftlist.size()];
      REQUIRE(found_hft == right_hft);
    }
  }

  SECTION("create_populations() for one HFT") {
    WorldConstructor world_cons(params, hftlist);
    REQUIRE(params->herbivore_type == HerbivoreType::Cohort);
    const auto hft = hftlist[0];
    PopulationList* pops = world_cons.create_populations(0);
    REQUIRE(pops != NULL);
    // Check that the CohortPopulation is of the given HFT.
    CHECK(&((CohortPopulation*)pops->begin()->get())->get_hft() == hft.get());
  }
}
