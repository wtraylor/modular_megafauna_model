// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for forage distribution models.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "forage_distribution_algorithms.h"
#include "catch.hpp"
#include "dummy_herbivore.h"
#include "dummy_hft.h"
#include "dummy_population.h"
#include "habitat_forage.h"
#include "hft.h"
#include "parameters.h"
#include "population_list.h"

using namespace Fauna;

TEST_CASE("Fauna::DistributeForageEqually", "") {
  // PREPARE POPULATIONS
  const int HFT_COUNT = 5;
  const int IND_PER_HFT = 10;
  const int IND_TOTAL = HFT_COUNT * IND_PER_HFT;  // dummy herbivores total
  const HftList hftlist = create_hfts(HFT_COUNT, Parameters());
  PopulationList pops;
  for (const auto& hft : hftlist) {
    // create new population
    PopulationInterface* new_pop = new DummyPopulation(hft.get());
    // fill with herbivores
    for (int i = 1; i <= IND_PER_HFT; i++) new_pop->create_offspring(1.0);
    // add newly created dummy population
    pops.emplace_back(new_pop);
  }

  // CREATE DEMAND MAP
  ForageDistribution demands;
  // loop through all herbivores and fill the distribution
  // object with pointer to herbivore and zero demands (to be
  // filled later)
  for (auto& p : pops)
    for (auto& h : p->get_list()) {
      // create with zero demands
      static const ForageMass ZERO_DEMAND;
      demands.emplace_back(h, ZERO_DEMAND);
    }

  // PREPARE AVAILABLE FORAGE
  HabitatForage available;
  const double AVAIL = 1.0;  // [kg/km²]
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++)
    available[*ft].set_mass(AVAIL);

  // our distrubition functor
  DistributeForageEqually distribute;

  SECTION("less demanded than available") {
    // SET DEMANDS
    const ForageMass IND_DEMAND(AVAIL / (IND_TOTAL + 1));
    // add new forage types here
    for (ForageDistribution::iterator itr = demands.begin();
         itr != demands.end(); itr++) {
      DummyHerbivore* pherbivore = (DummyHerbivore*)itr->first;
      pherbivore->set_demand(IND_DEMAND);
      itr->second = IND_DEMAND;
    }

    // DISTRIBUTE
    distribute(available, demands);

    // CHECK
    // there must not be any change
    ForageMass sum;
    for (ForageDistribution::iterator itr = demands.begin();
         itr != demands.end(); itr++) {
      DummyHerbivore* pherbivore = (DummyHerbivore*)itr->first;
      CHECK(itr->second == pherbivore->get_original_demand());
      sum += pherbivore->get_original_demand();
    }
    CHECK(sum <= available.get_mass());
  }

  SECTION("More demanded than available") {
    // SET DEMANDS
    ForageMass total_demand;
    int i = 0;  // a counter to vary the demands a little
    for (ForageDistribution::iterator itr = demands.begin();
         itr != demands.end(); itr++) {
      DummyHerbivore* pherbivore = (DummyHerbivore*)itr->first;
      // define a demand that is in total somewhat higher than
      // what’s available and varies among the herbivores
      ForageMass ind_demand(AVAIL / IND_TOTAL *
                            (1.0 + (i % 5) / 5));  // just arbitrary
      pherbivore->set_demand(ind_demand);
      itr->second = ind_demand;
      total_demand += ind_demand;
      i++;
    }

    // DISTRIBUTE
    distribute(available, demands);

    // CHECK
    // each herbivore must have approximatly its equal share
    ForageMass sum;
    for (ForageDistribution::iterator itr = demands.begin();
         itr != demands.end(); itr++) {
      DummyHerbivore* pherbivore = (DummyHerbivore*)itr->first;
      CHECK(itr->second != pherbivore->get_original_demand());
      sum += itr->second;
      // check each forage type individually because Approx()
      // is not defined for ForageMass
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++) {
        const double ind_portion = itr->second[*ft];
        const double ind_demand = pherbivore->get_original_demand()[*ft];
        const double tot_portion = available.get_mass()[*ft];
        const double tot_demand = total_demand[*ft];
        REQUIRE(tot_portion != 0.0);
        REQUIRE(tot_demand != 0.0);
        CHECK(ind_portion / tot_portion ==
              Approx(ind_demand / tot_demand).epsilon(0.05));
      }
    }
    // The sum may never exceed available forage
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++) {
      CHECK(sum[*ft] <= available.get_mass()[*ft]);
    }
    CHECK(sum <= available.get_mass());
  }
}
