#include <catch2/catch.hpp>
#include "feed.h"
using namespace Fauna;

TEST_CASE("Fauna::DistributeForageEqually", "") {
  // PREPARE POPULATIONS
  const int HFT_COUNT = 5;
  const int IND_PER_HFT = 10;
  const int IND_TOTAL = HFT_COUNT * IND_PER_HFT;  // dummy herbivores total
  const HftList hftlist = create_hfts(HFT_COUNT, Parameters());
  HftPopulationsMap popmap;
  for (HftList::const_iterator itr = hftlist.begin(); itr != hftlist.end();
       itr++) {
    // create new population
    std::auto_ptr<PopulationInterface> new_pop(new DummyPopulation(&*itr));
    // fill with herbivores
    for (int i = 1; i <= IND_PER_HFT; i++) new_pop->create_offspring(1.0);
    // add newly created dummy population
    popmap.add(new_pop);
    //
  }

  // CREATE DEMAND MAP
  ForageDistribution demands;
  // loop through all herbivores and fill the distribution
  // object with pointer to herbivore and zero demands (to be
  // filled later)
  for (HftPopulationsMap::iterator itr_pop = popmap.begin();
       itr_pop != popmap.end(); itr_pop++) {
    PopulationInterface& pop = **(itr_pop);
    HerbivoreVector vec = pop.get_list();
    // loop through herbivores in the population
    for (HerbivoreVector::iterator itr_her = vec.begin(); itr_her != vec.end();
         itr_her++) {
      HerbivoreInterface* pherbivore = *itr_her;
      // create with zero demands
      static const ForageMass ZERO_DEMAND;
      demands[pherbivore] = ZERO_DEMAND;
    }
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


TEST_CASE("Fauna::FeedHerbivores") {
  CHECK_THROWS(FeedHerbivores(std::auto_ptr<DistributeForage>(NULL)));

  // create objects
  const HftList HFTS = create_hfts(3, Parameters());
  const double DENS = 1.0;  // irrelevant in this test
  FeedHerbivores feed(
      std::auto_ptr<DistributeForage>(new DistributeForageEqually()));

  // these variables are set in each test section.
  HabitatForage AVAILABLE;
  HerbivoreVector herbivores;

  SECTION("no herbivores") {
    // set some arbitrary forage
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      AVAILABLE[*ft].set_mass(123.4);

    const ForageMass OLD_AVAIL = AVAILABLE.get_mass();
    feed(AVAILABLE, herbivores);

    // no changes
    CHECK(AVAILABLE.get_mass() == OLD_AVAIL);
  }

  SECTION("single herbivore") {
    DummyHerbivore herbi(&HFTS[0], DENS);

    // create some arbitrary demand (different for each forage type)
    ForageMass DEMAND;
    double i = 1.0;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      DEMAND.set(*ft, i++);
    herbi.set_demand(DEMAND);

    herbivores.push_back(&herbi);

    SECTION("more available than demanded") {
      // set available forage slightly higher than demand
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        AVAILABLE[*ft].set_mass(DEMAND[*ft] * 1.1);
      const ForageMass OLD_AVAIL = AVAILABLE.get_mass();

      // perform feeding operations
      feed(AVAILABLE, herbivores);
      const ForageMass eaten = OLD_AVAIL - AVAILABLE.get_mass();

      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++) {
        CHECK(eaten[*ft] == Approx(DEMAND[*ft]).epsilon(.05));
        CHECK(herbi.get_eaten()[*ft] == Approx(DEMAND[*ft]).epsilon(.05));
      }
    }

    SECTION("less available than demanded") {
      // set available forage slightly lower than demand
      const double FRACTION = 0.9;
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        AVAILABLE[*ft].set_mass(DEMAND[*ft] * FRACTION);
      const ForageMass OLD_AVAIL = AVAILABLE.get_mass();

      // perform feeding operations
      feed(AVAILABLE, herbivores);
      const ForageMass eaten = OLD_AVAIL - AVAILABLE.get_mass();

      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++) {
        CHECK(eaten[*ft] == Approx(DEMAND[*ft] * FRACTION).epsilon(.05));
        CHECK(herbi.get_eaten()[*ft] ==
              Approx(DEMAND[*ft] * FRACTION).epsilon(.05));
      }
    }

    SECTION("nothing available") {
      REQUIRE(AVAILABLE.get_mass() == 0.0);
      feed(AVAILABLE, herbivores);
      CHECK(AVAILABLE.get_mass() == 0.0);  // nothing changed
      CHECK(herbi.get_eaten() == 0.0);
    }

    SECTION("diet switch") {
      // TODO
    }
  }

  SECTION("many herbivores") {
    ForageMass TOTAL_DEMAND;
    // number of herbivores per HFT
    const int HERBI_COUNT = 10;
    std::list<DummyHerbivore> dummylist;
    // create herbivores
    for (HftList::const_iterator hft = HFTS.begin(); hft != HFTS.end(); hft++) {
      for (int i = 0; i < HERBI_COUNT; i++) {
        dummylist.push_back(DummyHerbivore(&*hft, DENS));
        herbivores.push_back(&dummylist.back());
        // set some arbitrary demand
        const ForageMass IND_DEMAND((double)i);
        dummylist.back().set_demand(IND_DEMAND);
        TOTAL_DEMAND += IND_DEMAND;
      }
    }

    SECTION("more available than demanded") {
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        AVAILABLE[*ft].set_mass(TOTAL_DEMAND[*ft] * 1.1);
      const ForageMass OLD_AVAIL = AVAILABLE.get_mass();

      feed(AVAILABLE, herbivores);
      const ForageMass eaten = OLD_AVAIL - AVAILABLE.get_mass();

      for (std::vector<HerbivoreInterface*>::const_iterator itr =
               herbivores.begin();
           itr != herbivores.end(); itr++) {
        const DummyHerbivore& herbi = *((DummyHerbivore*)*itr);
        // check for each forage type
        for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
             ft != FORAGE_TYPES.end(); ft++) {
          CHECK(herbi.get_eaten()[*ft] ==
                Approx(herbi.get_original_demand()[*ft]).epsilon(.05));
          CHECK(eaten[*ft] == Approx(TOTAL_DEMAND[*ft]).epsilon(.05));
        }
      }
    }

    SECTION("less available than demanded") {
      const double FRACTION = .4;
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        AVAILABLE[*ft].set_mass(TOTAL_DEMAND[*ft] * FRACTION);
      const ForageMass OLD_AVAIL = AVAILABLE.get_mass();

      feed(AVAILABLE, herbivores);
      const ForageMass eaten = OLD_AVAIL - AVAILABLE.get_mass();

      for (std::vector<HerbivoreInterface*>::const_iterator itr =
               herbivores.begin();
           itr != herbivores.end(); itr++) {
        const DummyHerbivore& herbi = *((DummyHerbivore*)*itr);
        // check for each forage type
        for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
             ft != FORAGE_TYPES.end(); ft++) {
          CHECK(
              herbi.get_eaten()[*ft] ==
              Approx(herbi.get_original_demand()[*ft] * FRACTION).epsilon(.05));
          CHECK(eaten[*ft] ==
                Approx(TOTAL_DEMAND[*ft] * FRACTION).epsilon(.05));
        }
      }
    }
  }
}
