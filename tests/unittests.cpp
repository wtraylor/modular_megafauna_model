//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Unit tests for megafauna herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 05-21-2017
//////////////////////////////////////////////////////////////////////////

#include <algorithm> // for std::max()
#include <cmath>   // for exp() and pow()
#include <memory>  // for std::auto_ptr
#include "catch.hpp"
#include "digestibility.h"
#include "energetics.h"
#include "environment.h"
#include "forageclasses.h"
#include "forageenergy.h"
#include "foraging.h"
#include "framework.h"
#include "herbivore.h"
#include "hft.h"
#include "mortality.h"
#include "parameters.h"
#include "population.h"
#include "reproduction.h"
#include "simulation_unit.h"
#include "testhabitat.h"
#include "utils.h"

using namespace Fauna;
using namespace FaunaOut;
using namespace FaunaSim;

namespace {
struct DistributeForageDummy : public DistributeForage {
  virtual void operator()(const HabitatForage& available,
                          ForageDistribution& forage_distribution) const {
    // TODO
  }
};

/// A dummy habitat that does nothing
class DummyHabitat : public Habitat {
 public:
  virtual void add_excreted_nitrogen(const double) {}  // deactivated
  virtual HabitatForage get_available_forage() const { return HabitatForage(); }
  virtual HabitatEnvironment get_environment() const {
    return HabitatEnvironment();
  }
  int get_day_public() const { return get_day(); }
};

/// A dummy herbivore that does nothing
class DummyHerbivore : public HerbivoreInterface {
 public:
  DummyHerbivore(const Hft* hft, const double ind_per_km2,
                 const double bodymass = 30.0)
      : hft(hft), ind_per_km2(ind_per_km2), bodymass(bodymass), killed(false) {}

  virtual void eat(const ForageMass& kg_per_km2,
                   const Digestibility& digestibility,
                   const ForageMass& N_kg_per_km2 = ForageMass(0)) {
    eaten += kg_per_km2;
    // Make sure we don’t drop the demand below zero.
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      actual_demand.set(*ft, std::max(0.0, actual_demand[*ft] - eaten[*ft]));
  }

  virtual double get_bodymass() const { return bodymass; }

  virtual ForageMass get_forage_demands(const HabitatForage& available_forage) {
    return actual_demand;
  }

  virtual const Hft& get_hft() const { return *hft; }

  virtual double get_ind_per_km2() const { return ind_per_km2; }
  virtual double get_kg_per_km2() const { return bodymass * ind_per_km2; }

  virtual const FaunaOut::HerbivoreData& get_todays_output() const {
    static FaunaOut::HerbivoreData dummy_output;
    return dummy_output;
  }

  virtual bool is_dead() const { return killed; }
  virtual void kill() {
    killed = true;
    ind_per_km2 = 0.0;
  }

  virtual void simulate_day(const int day, const HabitatEnvironment&,
                            double& offspring) {
    offspring = 0.0;
  }

  virtual double take_nitrogen_excreta() { return 0; }

 public:
  const ForageMass& get_original_demand() const { return original_demand; }
  void set_demand(const ForageMass& d) { original_demand = actual_demand = d; }
  const ForageMass& get_eaten() const { return eaten; }
  double ind_per_km2;

 private:
  const Hft* hft;
  const double bodymass;
  ForageMass original_demand, actual_demand, eaten;
  int killed;
};

/// A population of dummy herbivores
class DummyPopulation : public PopulationInterface {
 private:
  const Hft* hft;
  std::vector<DummyHerbivore> vec;

 public:
  DummyPopulation(const Hft* hft) : hft(hft), has_been_purged(false) {}

  /// creates one new herbivore object
  virtual void create_offspring(const double ind_per_km2) {
    vec.push_back(DummyHerbivore(hft, ind_per_km2));
  }

  /// creates one new herbivore object
  virtual void establish() {
    vec.push_back(DummyHerbivore(hft, hft->establishment_density));
  }

  virtual const Hft& get_hft() const { return *hft; }

  virtual std::vector<const HerbivoreInterface*> get_list() const {
    std::vector<const HerbivoreInterface*> res;
    for (int i = 0; i < vec.size(); i++) res.push_back(&vec[i]);
    return res;
  }
  virtual std::vector<HerbivoreInterface*> get_list() {
    std::vector<HerbivoreInterface*> res;
    for (int i = 0; i < vec.size(); i++) res.push_back(&vec[i]);
    return res;
  }

  void purge_of_dead() { has_been_purged = true; }
  bool has_been_purged;
};

/// Dummy class to test \ref HerbivoreBase
class HerbivoreBaseDummy : public HerbivoreBase {
 public:
  virtual double get_ind_per_km2() const { return ind_per_km2; }

  virtual bool is_dead() const { return false; }
  virtual void kill() {}
  /// Establishment Constructor
  HerbivoreBaseDummy(const int age_days, const double body_condition,
                     const Hft* hft, const Sex sex)
      : HerbivoreBase(age_days, body_condition, hft, sex), ind_per_km2(1.0) {}

  /// Birth Constructor
  HerbivoreBaseDummy(const Hft* hft, const Sex sex)
      : HerbivoreBase(hft, sex), ind_per_km2(1.0) {}

  HerbivoreBaseDummy(const HerbivoreBaseDummy& other)
      : HerbivoreBase(other), ind_per_km2(other.ind_per_km2) {}
  HerbivoreBaseDummy& operator=(const HerbivoreBaseDummy& rhs) {
    HerbivoreBase::operator=(rhs);
    ind_per_km2 = rhs.ind_per_km2;
    return *this;
  }

 protected:
  virtual void apply_mortality(const double mortality) {
    ind_per_km2 *= mortality;
  }

 private:
  double ind_per_km2;
};

/// Create a simple, valid HftList
HftList create_hfts(const int count, const Parameters& params) {
  HftList hftlist;
  for (int i = 0; i < count; i++) {
    Hft hft;
    hft.is_included = true;
    // construct name for HFT
    std::ostringstream stream;
    stream << "hft" << i;
    hft.name = stream.str();

    std::string msg;
    if (!hft.is_valid(params, msg))
      FAIL("create_hfts(): HFT is not valid:\n" << msg);

    hftlist.insert(hft);
  }
  REQUIRE(hftlist.size() == count);
  return hftlist;
}

/// \brief Check if the lengths of the modifiable and the
/// read-only population vectors match.
bool population_lists_match(PopulationInterface& pop) {
  // FIRST the read-only -> no chance for the population
  // object to change the list.
  ConstHerbivoreVector readonly = ((const PopulationInterface&)pop).get_list();
  HerbivoreVector modifiable = pop.get_list();
  return modifiable.size() == readonly.size();
}

}  // anonymous namespace

// TEST CASES IN ALPHABETICAL ORDER, PLEASE

TEST_CASE("Dummies", "") {
  Hft hft1;
  hft1.name = "hft1";
  SECTION("DummyHerbivore") {
    // INITIALIZATION
    CHECK(DummyHerbivore(&hft1, 1.0).get_ind_per_km2() == 1.0);
    CHECK(DummyHerbivore(&hft1, 0.0).get_ind_per_km2() == 0.0);
    CHECK(DummyHerbivore(&hft1, 1.0, 25.0).get_bodymass() == 25.0);
    CHECK(DummyHerbivore(&hft1, 1.0).get_original_demand() == 0.0);
    DummyHerbivore d(&hft1, 1.0);
    CHECK(&d.get_hft() == &hft1);
    CHECK(d.get_eaten() == 0.0);
    CHECK(d.get_original_demand() == 0.0);

    const ForageMass DEMAND(23.9);
    d.set_demand(DEMAND);
    CHECK(d.get_original_demand() == DEMAND);

    const ForageMass EATEN(12.4);
    d.eat(EATEN, Digestibility(.5));
    CHECK(d.get_eaten() == EATEN);
  }

  SECTION("DummyPopulation") {
    DummyHerbivore dummy1 = DummyHerbivore(&hft1, 1.0);
    DummyHerbivore dummy2 = DummyHerbivore(&hft1, 0.0);
    DummyPopulation pop = DummyPopulation(&hft1);
    pop.create_offspring(1.0);
    REQUIRE(pop.get_list().size() == 1);
  }
}

TEST_CASE("Fauna::average()", "") {
  CHECK_THROWS(average(1.0, 2.0, -1.0, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, -1.0));
  CHECK_THROWS(average(1.0, 2.0, 0.0, 0.0));
  CHECK_THROWS(average(1.0, 2.0, NAN, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, NAN));
  CHECK_THROWS(average(1.0, 2.0, INFINITY, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, INFINITY));
  CHECK(average(1.0, 3.0) == Approx(2.0));
  CHECK(average(1.0, 1.0) == Approx(1.0));
  CHECK(average(-1.0, 1.0) == Approx(0.0));
}

TEST_CASE("Fauna::BreedingSeason", "") {
  CHECK_THROWS(BreedingSeason(-1, 1));
  CHECK_THROWS(BreedingSeason(365, 1));
  CHECK_THROWS(BreedingSeason(0, 0));
  CHECK_THROWS(BreedingSeason(0, -1));
  CHECK_THROWS(BreedingSeason(0, 366));

  const int START = 100;
  SECTION("check breeding season") {
    const int LENGTH = 90;
    const BreedingSeason b(START, LENGTH);
    // within season
    CHECK(b.is_in_season(START) > 0.0);
    CHECK(b.is_in_season(START + LENGTH) > 0.0);

    // before breeding season
    CHECK(b.is_in_season(START - 1) == 0.0);

    // after season
    CHECK(b.is_in_season(START + LENGTH + 1) == 0.0);
  }

  SECTION("check breeding season extending over year boundary") {
    const int LENGTH2 = 360;
    const BreedingSeason b(START, LENGTH2);
    const int END = (START + LENGTH2) % 365;

    // within season
    CHECK(b.is_in_season(START) > 0.0);
    CHECK(b.is_in_season(END) > 0.0);

    // before breeding season
    CHECK(b.is_in_season(START - 1) == 0.0);

    // after season
    CHECK(b.is_in_season(END + 1) == 0.0);
  }
}

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

TEST_CASE("Fauna::DigestibilityFromNPP") {
  static const double FRESH = .7;  // fresh digestibility
  static const double DEAD = .4;   // dead digestibility
  std::deque<double> dnpp;         // daily NPP [kgC/m²/day]

  // Exception: dead > fresh digestibility
  CHECK_THROWS(
      DigestibilityFromNPP::get_digestibility_from_dnpp(dnpp, DEAD, FRESH));

  // Empty DNPP record ⇒ zero digestibility
  CHECK(DigestibilityFromNPP::get_digestibility_from_dnpp(dnpp, FRESH, DEAD) ==
        0.0);

  // Create a few NPP values and calculate digestibility “manually”.
  dnpp.push_front(.4);
  dnpp.push_front(.5);
  dnpp.push_front(.7);
  dnpp.push_front(.4);

  // Calculate ‘manually’
  const double sum = .4 + .5 + .7 + .4;
  const double dig =
      (.4 * FRESH + .5 * FRESH -
       (FRESH - DEAD) * 1.0 / DigestibilityFromNPP::ATTRITION_PERIOD +
       .7 * FRESH -
       (FRESH - DEAD) * 2.0 / DigestibilityFromNPP::ATTRITION_PERIOD +
       .4 * FRESH -
       (FRESH - DEAD) * 3.0 / DigestibilityFromNPP::ATTRITION_PERIOD) /
      sum;

  // Calculate with function
  const double dig_from_function =
      DigestibilityFromNPP::get_digestibility_from_dnpp(dnpp, FRESH, DEAD);

  REQUIRE(dig_from_function == Approx(dig).epsilon(.05));

  // Adding new zero values *in the back* shouldn’t change the result
  // because new entries with zero weight won’t change the average. All
  // other values are unchanged since we added to the *back* of the deque.
  dnpp.push_back(0.0);
  dnpp.push_back(0.0);
  REQUIRE(DigestibilityFromNPP::get_digestibility_from_dnpp(
              dnpp, FRESH, DEAD) == Approx(dig_from_function));

  // Adding values beyond the time frame in question shouldn’t change the
  // result either.
  // First, fill the deque with zeros up until the ATTRITION_PERIOD is all
  // covered.
  for (int i = dnpp.size(); i < DigestibilityFromNPP::ATTRITION_PERIOD; i++)
    dnpp.push_back(0.0);
  // Second, add some values that would change the result
  dnpp.push_back(1.0);
  dnpp.push_back(2.0);
  // Now, the result should still be the same.
  REQUIRE(DigestibilityFromNPP::get_digestibility_from_dnpp(
              dnpp, FRESH, DEAD) == Approx(dig_from_function));
}

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

TEST_CASE("Fauna::get_expenditure_taylor_1981()") {
  const double CURRENT = 90;  // [kg]
  const double ADULT = 100;   // [kg]
  CHECK(get_expenditure_taylor_1981(CURRENT, ADULT) ==
        Approx(0.4 * CURRENT * pow(ADULT, -0.27)));
}

TEST_CASE("Fauna::get_expenditure_zhu_et_al_2018()") {
  const double M = 100;  // [kg]
  const double T = 20;   // [°C]
  CHECK_THROWS(get_expenditure_zhu_et_al_2018(-4, T));
  CHECK_THROWS(get_expenditure_zhu_et_al_2018(0, T));
  // Check formula:
  CHECK(get_expenditure_zhu_et_al_2018(M, T) ==
        0.36 / exp(0.0079 * T) * pow(M, 0.75));
  // More energy costs in lower temperature:
  CHECK(get_expenditure_zhu_et_al_2018(M, T) <
        get_expenditure_zhu_et_al_2018(M, T - 20));
}

TEST_CASE("Fauna::FatmassEnergyBudget", "") {
  CHECK_THROWS(FatmassEnergyBudget(-1.0, 1.0));
  CHECK_THROWS(FatmassEnergyBudget(0.0, 0.0));
  CHECK_THROWS(FatmassEnergyBudget(0.0, -1.0));
  CHECK_THROWS(FatmassEnergyBudget(1.1, 1.0));

  const double INIT_FATMASS = 1.0;  // initial fat mass
  const double MAX_FATMASS = 2.0;   // maximim fat mass
  FatmassEnergyBudget budget(INIT_FATMASS, MAX_FATMASS);

  // Initialization
  REQUIRE(budget.get_fatmass() == INIT_FATMASS);
  REQUIRE(budget.get_energy_needs() == 0.0);
  REQUIRE(budget.get_max_anabolism_per_day() ==
          Approx(54.6 * (MAX_FATMASS - INIT_FATMASS)));

  // exceptions
  CHECK_THROWS(budget.metabolize_energy(-1.0));
  CHECK_THROWS(budget.metabolize_energy(1000000.0));
  CHECK_THROWS(budget.add_energy_needs(-1.0));
  CHECK_THROWS(budget.set_max_fatmass(INIT_FATMASS / 2.0, .1));
  CHECK_THROWS(budget.set_max_fatmass(-1.0, .1));
  CHECK_THROWS(budget.set_max_fatmass(INIT_FATMASS, -.1));

  const double ENERGY = 10.0;  // MJ

  SECTION("force_body_condition()") {
    CHECK_THROWS(budget.force_body_condition(-.1));
    CHECK_THROWS(budget.force_body_condition(1.1));
    budget.force_body_condition(0.3);
    CHECK(budget.get_fatmass() / budget.get_max_fatmass() == Approx(0.3));
  }

  SECTION("Set energy needs") {
    budget.add_energy_needs(ENERGY);
    REQUIRE(budget.get_energy_needs() == Approx(ENERGY));
    budget.add_energy_needs(ENERGY);
    REQUIRE(budget.get_energy_needs() == Approx(2.0 * ENERGY));
  }

  SECTION("Anabolism") {
    budget.metabolize_energy(ENERGY);
    CHECK(budget.get_fatmass() > INIT_FATMASS);

    // Check the number with coefficient of Blaxter (1989)
    CHECK(budget.get_fatmass() == Approx(INIT_FATMASS + ENERGY / 54.6));
  }

  SECTION("Anabolism Limit") {
    const double anabolism_unlimited = budget.get_max_anabolism_per_day();

    // Setting maximum gain to zero means no limits.
    budget.set_max_fatmass(MAX_FATMASS, 0.0);
    CHECK(budget.get_max_anabolism_per_day() == Approx(anabolism_unlimited));

    // Set maximum gain to half of the gap towards maximum fat mass.
    const double MAX_GAIN = (MAX_FATMASS - INIT_FATMASS) / 2.0;
    budget.set_max_fatmass(MAX_FATMASS, MAX_GAIN);

    CHECK(budget.get_max_anabolism_per_day() ==
          Approx(anabolism_unlimited / 2.0));
  }

  SECTION("Catabolism") {
    budget.add_energy_needs(ENERGY);
    budget.catabolize_fat();
    CHECK(budget.get_fatmass() < INIT_FATMASS);

    // Check the number with coefficient of Blaxter (1989)
    CHECK(budget.get_fatmass() == Approx(INIT_FATMASS - ENERGY / 39.3));
  }

  SECTION("Metabolism") {
    budget.add_energy_needs(ENERGY);
    REQUIRE(budget.get_energy_needs() == ENERGY);
    budget.metabolize_energy(ENERGY);
    CHECK(budget.get_energy_needs() == Approx(0.0));
  }

  SECTION("Metabolism and Anabolism") {
    budget.add_energy_needs(ENERGY / 2.0);
    REQUIRE(budget.get_energy_needs() == Approx(ENERGY / 2.0));
    budget.metabolize_energy(ENERGY);
    CHECK(budget.get_energy_needs() == 0.0);
    CHECK(budget.get_fatmass() > INIT_FATMASS);
    CHECK(budget.get_fatmass() < MAX_FATMASS);
  }

  SECTION("Merge") {
    budget.add_energy_needs(ENERGY);
    const double OTHER_FATMASS = 3.0;
    const double OTHER_MAX_FATMASS = 4.0;
    const double OTHER_ENERGY = 13.0;
    FatmassEnergyBudget other(OTHER_FATMASS, OTHER_MAX_FATMASS);
    other.add_energy_needs(OTHER_ENERGY);

    SECTION("Merge with equal weight") {
      budget.merge(other, 1.0, 1.0);
      CHECK(budget.get_energy_needs() == Approx((ENERGY + OTHER_ENERGY) / 2.0));
      CHECK(budget.get_max_fatmass() ==
            Approx((MAX_FATMASS + OTHER_MAX_FATMASS) / 2.0));
      CHECK(budget.get_fatmass() ==
            Approx((INIT_FATMASS + OTHER_FATMASS) / 2.0));
    }

    SECTION("Merge with different weight") {
      const double W1 = 0.4;
      const double W2 = 1.2;
      budget.merge(other, W1, W2);
      CHECK(budget.get_energy_needs() ==
            Approx((ENERGY * W1 + OTHER_ENERGY * W2) / (W1 + W2)));
      CHECK(budget.get_max_fatmass() ==
            Approx((MAX_FATMASS * W1 + OTHER_MAX_FATMASS * W2) / (W1 + W2)));
      CHECK(budget.get_fatmass() ==
            Approx((INIT_FATMASS * W1 + OTHER_FATMASS * W2) / (W1 + W2)));
    }
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

TEST_CASE("Fauna::ForageValues", "") {
  // Not all functions are tested here, only the
  // exceptions for invalid values, the constructors, and
  // get, set, and sum.
  SECTION("positive and zero") {
    CHECK_THROWS(ForageValues<POSITIVE_AND_ZERO>(-1.0));
    CHECK_THROWS(ForageValues<POSITIVE_AND_ZERO>(NAN));
    CHECK_THROWS(ForageValues<POSITIVE_AND_ZERO>(INFINITY));

    // zero initialization
    ForageValues<POSITIVE_AND_ZERO> fv;
    CHECK(fv.sum() == Approx(0.0));
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(fv[*ft] == 0.0);

    // exceptions
    CHECK_THROWS(fv.get(FT_INEDIBLE));
    CHECK_THROWS(fv[FT_INEDIBLE]);
    CHECK_THROWS(fv.set(FT_GRASS, -1.0));
    CHECK_THROWS(fv.set(FT_GRASS, NAN));
    CHECK_THROWS(fv.set(FT_GRASS, INFINITY));
    CHECK_THROWS(fv / 0.0);
    CHECK_THROWS(fv /= 0.0);

    const double G = 2.0;
    fv.set(FT_GRASS, G);
    CHECK(fv.get(FT_GRASS) == G);
    CHECK(fv[FT_GRASS] == G);
    CHECK(fv.sum() == G);  // because only grass changed

    // assignment
    ForageValues<POSITIVE_AND_ZERO> fv2 = fv;
    CHECK(fv2 == fv);
    CHECK(fv2[FT_GRASS] == fv[FT_GRASS]);
    CHECK(fv2.sum() == fv.sum());

    // value initialization
    const double V = 3.0;
    ForageValues<POSITIVE_AND_ZERO> fv3(V);
    CHECK(fv3[FT_GRASS] == V);
    CHECK(fv3.sum() == FORAGE_TYPES.size() * V);

    // Sums
    CHECK((fv + fv).sum() == Approx(fv.sum() + fv.sum()));
    CHECK((fv2 + fv).sum() == Approx(fv2.sum() + fv.sum()));
    CHECK((fv3 + fv).sum() == Approx(fv3.sum() + fv.sum()));
  }

  SECTION("zero to one") {
    CHECK_THROWS(ForageValues<ZERO_TO_ONE>(-1.0));
    CHECK_THROWS(ForageValues<ZERO_TO_ONE>(1.1));
    CHECK_THROWS(ForageValues<ZERO_TO_ONE>(NAN));
    CHECK_THROWS(ForageValues<ZERO_TO_ONE>(INFINITY));
  }

  SECTION("Comparison") {
    ForageValues<POSITIVE_AND_ZERO> fv1(0.0);
    ForageValues<POSITIVE_AND_ZERO> fv2(1.0);
    ForageValues<POSITIVE_AND_ZERO> fv3(fv2);

    CHECK(fv1 < fv2);
    CHECK(fv1 <= fv2);
    CHECK(fv2 >= fv1);
    CHECK(fv2 > fv1);

    CHECK(fv2 == fv3);
    CHECK(fv2 <= fv3);
    CHECK(fv2 >= fv3);
  }

  SECTION("Merging: positive and zero") {
    const double V1 = 3.0;
    const double V2 = 19.0;
    ForageValues<POSITIVE_AND_ZERO> a(V1);
    const ForageValues<POSITIVE_AND_ZERO> b(V2);
    const double W1 = 12.0;
    const double W2 = 23.0;
    a.merge(b, W1, W2);
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(a[*ft] == Approx((V1 * W1 + V2 * W2) / (W2 + W1)));
  }

  SECTION("Merging: zero to one") {
    const double V1 = 0.1;
    const double V2 = 0.8;
    ForageValues<POSITIVE_AND_ZERO> a(V1);
    const ForageValues<POSITIVE_AND_ZERO> b(V2);
    const double W1 = 12.0;
    const double W2 = 23.0;
    a.merge(b, W1, W2);
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(a[*ft] == Approx((V1 * W1 + V2 * W2) / (W2 + W1)));
  }

  SECTION("Minimums") {
    ForageValues<POSITIVE_AND_ZERO> a(1.0);
    ForageValues<POSITIVE_AND_ZERO> b(2.0);
    CHECK(a.min(a) == a);
    CHECK(a.min(b) == b.min(a));
    CHECK(a.min(b) == a);
  }

  //------------------------------------------------------------------
  // FREE FUNCTIONS

  SECTION("operator*(ForageFraction, double)") {
    ForageFraction ff;
    double i = 1.0;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      ff.set(*ft, 1.0 / ++i);

    double d = 123.4;
    const ForageValues<POSITIVE_AND_ZERO> result = d * ff;

    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(result[*ft] == ff[*ft] * d);
  }

  SECTION("operator*(ForageFraction, ForageValues<POSITIVE_AND_ZERO>") {
    ForageFraction ff;
    double i = 1.0;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      ff.set(*ft, 1.0 / ++i);

    ForageValues<POSITIVE_AND_ZERO> fv;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      fv.set(*ft, ++i);

    const ForageValues<POSITIVE_AND_ZERO> result = ff * fv;

    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(result[*ft] == ff[*ft] * fv[*ft]);
  }

  SECTION("foragevalues_to_foragefractions()") {
    // `tolerance` mustn’t be negative
    CHECK_THROWS(foragevalues_to_foragefractions(
        ForageValues<POSITIVE_AND_ZERO>(), -.1));

    ForageValues<POSITIVE_AND_ZERO> fv;
    SECTION("All numbers below 1.0") {
      double i = 1;
      // create some numbers between 0 and 1
      for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
           ft != FORAGE_TYPES.end(); ft++)
        fv.set(*ft, 1.0 / (++i));
      const ForageFraction ff = foragevalues_to_foragefractions(fv, 0.0);

      for (ForageFraction::const_iterator i = ff.begin(); i != ff.end(); i++)
        CHECK(i->second == fv[i->first]);
    }

    SECTION("Numbers with tolerance") {
      const double TOLERANCE = .1;
      fv.set(FT_GRASS, 1.0 + TOLERANCE);
      const ForageFraction ff = foragevalues_to_foragefractions(fv, TOLERANCE);

      CHECK(ff[FT_GRASS] == 1.0);
    }

    SECTION("Exception exceeding tolerance") {
      const double TOLERANCE = .1;
      fv.set(FT_GRASS, 1.0 + TOLERANCE + .001);
      CHECK_THROWS(foragevalues_to_foragefractions(fv, TOLERANCE));
    }
  }

  SECTION("foragefractions_to_foragevalues()") {
    ForageFraction ff;
    double i = 1;
    // create some numbers between 0 and 1
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      ff.set(*ft, 1.0 / (++i));

    const ForageValues<POSITIVE_AND_ZERO> fv =
        foragefractions_to_foragevalues(ff);

    for (ForageValues<POSITIVE_AND_ZERO>::const_iterator i = fv.begin();
         i != fv.end(); i++)
      CHECK(i->second == ff[i->first]);
  }

  SECTION("convert_mj_to_kg_proportionally()") {
    // set some arbitrary energy content
    double i = 31.0;
    ForageEnergyContent energy_content;
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      energy_content.set(*ft, i++);

    // set some arbitrary proportions
    ForageFraction prop_mj;  // energy proportions
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      prop_mj.set(*ft, 1.0 / (++i));

    // calculate mass proportions
    const ForageFraction prop_kg =
        convert_mj_to_kg_proportionally(energy_content, prop_mj);

    // CHECK RESULTS

    CHECK(prop_kg.sum() == Approx(prop_mj.sum()));

    // convert mass back to energy
    ForageMass mj = prop_kg * energy_content;
    const double mj_total = mj.sum();

    // The relation between each energy component towards the total
    // energy must stay the same.
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      CHECK(mj[*ft] / mj.sum() == Approx(prop_mj[*ft] / prop_mj.sum()));
  }
}

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

TEST_CASE("Fauna::get_day_of_month()") {
  CHECK_THROWS(get_day_of_month(-1));
  CHECK_THROWS(get_day_of_month(365));
  // Just check some arbitrary dates
  CHECK(get_day_of_month(0) == 0);        // Jan 1st
  CHECK(get_day_of_month(364) == 30);     // Dec. 1st
  CHECK(get_day_of_month(32) == 1);       // Feb. 2nd
  CHECK(get_day_of_month(31 + 28) == 0);  // Mar. 1st
}

TEST_CASE("Fauna::get_max_intake_as_total_mass()") {
  double i = 1.0;
  ForageEnergyContent energy_content;
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++)
    energy_content.set(*ft, i++);

  // set some arbitrary proportions
  ForageFraction prop_mj;  // energy proportions
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++)
    prop_mj.set(*ft, 1.0 / (++i));

  // exception
  CHECK_THROWS(get_max_intake_as_total_mass(prop_mj, energy_content, -1));

  const double KG_TOTAL = 10.0;

  const ForageMass result =
      get_max_intake_as_total_mass(prop_mj, energy_content, KG_TOTAL);

  CHECK(result.sum() == Approx(KG_TOTAL));

  // convert mass back to energy
  ForageMass mj = result * energy_content;
  const double mj_total = mj.sum();

  // The relation between each energy component towards the total
  // energy must stay the same.
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++)
    CHECK(mj[*ft] / mj.sum() == Approx(prop_mj[*ft] / prop_mj.sum()));
}

TEST_CASE("Fauna::GetDigestiveLimitIlliusGordon1992", "") {
  CHECK_THROWS(GetDigestiveLimitIlliusGordon1992(-1.0, DT_RUMINANT));
  CHECK_THROWS(GetDigestiveLimitIlliusGordon1992(0.0, DT_RUMINANT));

  const Digestibility digestibility(0.5);

  SECTION("exceptions") {
    const Digestibility digestibility(0.5);
    const double AD = 100.0;
    GetDigestiveLimitIlliusGordon1992 rum(AD, DT_RUMINANT);
    CHECK_THROWS(rum(AD + 1, digestibility));
    CHECK_THROWS(rum(0.0, digestibility));
    CHECK_THROWS(rum(-1.0, digestibility));
  }

  SECTION("check some example numbers") {
    const double ADULT = 40.0;    // adult weight, [kg]
    const double CURRENT = 20.0;  // current weight [kg]
    GetDigestiveLimitIlliusGordon1992 rum(ADULT, DT_RUMINANT);
    GetDigestiveLimitIlliusGordon1992 hind(ADULT, DT_HINDGUT);

    SECTION("...for grass") {
      const double d = digestibility[FT_GRASS];
      CHECK(rum(CURRENT, d)[FT_GRASS] ==
            Approx(0.034 * exp(3.565 * d) * pow(ADULT, 0.077 * exp(d) + 0.73) *
                   pow(CURRENT / ADULT, 0.75)));

      CHECK(hind(CURRENT, d)[FT_GRASS] ==
            Approx(0.108 * exp(3.284 * d) * pow(ADULT, 0.080 * exp(d) + 0.73) *
                   pow(CURRENT / ADULT, 0.75)));
    }
  }

  SECTION("pre-adult has less capacity") {
    const double ADULT = 100.0;
    GetDigestiveLimitIlliusGordon1992 rum(ADULT, DT_RUMINANT);
    CHECK(rum(ADULT / 2, digestibility) < rum(ADULT, digestibility));
    GetDigestiveLimitIlliusGordon1992 hind(ADULT, DT_HINDGUT);
    CHECK(hind(ADULT / 2, digestibility) < hind(ADULT, digestibility));
  }

  SECTION("bigger animals have more capacity") {
    const double AD1 = 100.0;
    const double AD2 = AD1 * 1.4;
    const Digestibility DIG(.5);
    CHECK(GetDigestiveLimitIlliusGordon1992(AD1, DT_HINDGUT)(AD1, DIG) <
          GetDigestiveLimitIlliusGordon1992(AD2, DT_HINDGUT)(AD2, DIG));
    CHECK(GetDigestiveLimitIlliusGordon1992(AD1, DT_RUMINANT)(AD1, DIG) <
          GetDigestiveLimitIlliusGordon1992(AD2, DT_RUMINANT)(AD2, DIG));
  }

  SECTION("higher digestibility brings higher capacity") {
    const double ADULT = 100.0;
    const Digestibility DIG1(.8);
    const Digestibility DIG2(.9);
    {  // RUMINANT
      const GetDigestiveLimitIlliusGordon1992 rumi(ADULT, DT_RUMINANT);

      INFO("Ruminant, digestibility=" << DIG1[FT_GRASS]);
      INFO("grass: " << rumi(ADULT, DIG1)[FT_GRASS]);

      INFO("Ruminant, digestibility=" << DIG2[FT_GRASS]);
      INFO("grass: " << rumi(ADULT, DIG2)[FT_GRASS]);

      CHECK(rumi(ADULT, DIG1) < rumi(ADULT, DIG2));
    }
    {  // HINDGUT
      const GetDigestiveLimitIlliusGordon1992 hind(ADULT, DT_HINDGUT);

      INFO("Hindgut, digestibility=" << DIG1[FT_GRASS]);
      INFO("grass: " << hind(ADULT, DIG1)[FT_GRASS]);

      INFO("Hindgut, digestibility=" << DIG2[FT_GRASS]);
      INFO("grass: " << hind(ADULT, DIG2)[FT_GRASS]);

      CHECK(hind(ADULT, DIG1) < hind(ADULT, DIG2));
    }
  }

  SECTION("zero digestibility => zero energy") {
    const double ADULT = 100.0;
    const Digestibility ZERO(0.0);
    CHECK(GetDigestiveLimitIlliusGordon1992(ADULT, DT_HINDGUT)(ADULT, ZERO) ==
          0.0);
    CHECK(GetDigestiveLimitIlliusGordon1992(ADULT, DT_RUMINANT)(ADULT, ZERO) ==
          0.0);
  }
}

TEST_CASE("Fauna::GetForageDemands") {
  // constructor exceptions
  CHECK_THROWS(GetForageDemands(NULL, SEX_MALE));

  const Parameters params;
  Hft hft = create_hfts(1, params)[0];
  hft.foraging_limits.clear();
  hft.digestive_limit = DL_NONE;

  const int DAY = 0;
  HabitatForage avail;                            // available forage
  const ForageEnergyContent ENERGY_CONTENT(1.0);  // [MJ/kgDM]
  const double BODYMASS = hft.bodymass_female;    // [kg/ind]

  SECTION("Check some exceptions.") {
    // Create the object.
    GetForageDemands gfd(&hft, SEX_FEMALE);

    // Exception because not initialized.
    CHECK_THROWS(gfd(1.0));

    // exceptions during initialization
    CHECK_THROWS(gfd.init_today(DAY, avail, ENERGY_CONTENT,
                                -1));  // body mass
    CHECK_THROWS(gfd.init_today(-1,    // day
                                avail, ENERGY_CONTENT, BODYMASS));

    // initialize
    CHECK(!gfd.is_day_initialized(DAY));
    gfd.init_today(DAY, avail, ENERGY_CONTENT, BODYMASS);
    CHECK(gfd.is_day_initialized(DAY));

    // negative energy needs
    CHECK_THROWS(gfd(-1.0));

    // Negative eaten forage
    CHECK_THROWS(gfd.add_eaten(-1.0));

    // Eat more than possible.
    CHECK_THROWS(gfd.add_eaten(999999));
  }

  SECTION("Grazer with Fixed Fraction") {
    hft.diet_composer = DC_PURE_GRAZER;
    hft.digestive_limit = DL_FIXED_FRACTION;
    GetForageDemands gfd(&hft, SEX_FEMALE);  // create object
    const double DIG_FRAC = 0.03;  // max. intake as fraction of body mass
    hft.digestive_limit_fixed = DIG_FRAC;
    avail.grass.set_mass(999999);  // Lots of live grass (but nothing else).

    // We prescribe *lots* of hunger so that digestion *must* be the
    // limiting factor.
    const double ENERGY_DEMAND = 99999;

    // initialize
    gfd.init_today(DAY, avail, ENERGY_CONTENT, BODYMASS);

    // Lots of energy needs, but intake is limited by digestion.
    // Only grass shall be demanded.
    // The result must match the given fraction of body mass.
    const ForageMass init_demand = gfd(ENERGY_DEMAND);
    CHECK(init_demand[FT_GRASS] == Approx(BODYMASS * DIG_FRAC));
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++) {
      if (*ft != FT_GRASS) CHECK(init_demand[*ft] == 0.0);
    }
    // The demand may not change if we call it again.
    CHECK(gfd(ENERGY_DEMAND) == init_demand);

    // Now give something to eat, and the demand should become less.
    ForageMass EATEN;
    EATEN.set(FT_GRASS, 1.0);
    gfd.add_eaten(EATEN);
    CHECK(gfd(ENERGY_DEMAND) == init_demand - EATEN);
    // … and of course the result should stay the same.
    CHECK(gfd(ENERGY_DEMAND) == init_demand - EATEN);
  }
}

TEST_CASE("Fauna::GetNetEnergyContentDefault", "") {
  GetNetEnergyContentDefault ne_ruminant(DT_RUMINANT);
  GetNetEnergyContentDefault ne_hindgut(DT_HINDGUT);

  const Digestibility DIG1(0.5);
  const Digestibility DIG2(0.3);

  // higher digestibility ==> more energy
  CHECK(ne_ruminant(DIG1) > ne_ruminant(DIG2));
  CHECK(ne_hindgut(DIG1) > ne_hindgut(DIG2));

  // hindguts have lower efficiency
  CHECK(ne_ruminant(DIG1) > ne_hindgut(DIG1));

  // Check some absolute numbers
  {  // grass for ruminants
    const double ME = 15.0 * DIG1[FT_GRASS];
    CHECK(ne_ruminant(DIG1[FT_GRASS])[FT_GRASS] ==
          Approx(ME * (0.019 * ME + 0.503)));
  }
  {  // grass for hindguts
    const double ME = 15.0 * DIG1[FT_GRASS];
    CHECK(ne_hindgut(DIG1[FT_GRASS])[FT_GRASS] ==
          Approx(ME * (0.019 * ME + 0.503) * 0.93));
  }
}

TEST_CASE("Fauna::get_retention_time()") {
  CHECK_THROWS(get_retention_time(0));
  CHECK_THROWS(get_retention_time(-1));
  CHECK(get_retention_time(100) == Approx(45.276604));
}

TEST_CASE("Fauna::get_random_fraction", "") {
  for (int i = 0; i < 100; i++) {
    const double r = get_random_fraction();
    CHECK(r <= 1.0);
    CHECK(r >= 0.0);
  }
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
    CHECK(get_mort(1.0, new_bc) == Approx(0.0));
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

TEST_CASE("Fauna::GrassForage", "") {
  SECTION("Initialization") {
    CHECK(GrassForage().get_mass() == 0.0);
    CHECK(GrassForage().get_digestibility() == 0.0);
    CHECK(GrassForage().get_fpc() == 0.0);
  }

  SECTION("Exceptions") {
    CHECK_THROWS(GrassForage().set_fpc(1.2));
    CHECK_THROWS(GrassForage().set_fpc(-0.2));
    CHECK_THROWS(GrassForage().set_mass(-0.2));
    CHECK_THROWS(GrassForage().set_digestibility(-0.2));
    CHECK_THROWS(GrassForage().set_digestibility(1.2));

    GrassForage g;
    CHECK_THROWS(g.set_fpc(0.5));  // mass must be >0.0
    g.set_mass(1.0);
    CHECK_THROWS(g.get_fpc());     // illogical state
    CHECK_THROWS(g.set_fpc(0.0));  // fpc must be >0.0
  }

  SECTION("sward density") {
    CHECK(GrassForage().get_sward_density() == 0.0);

    GrassForage g;
    const double FPC = .234;
    const double MASS = 1256;
    g.set_mass(MASS);
    g.set_fpc(FPC);
    CHECK(g.get_sward_density() == Approx(MASS / FPC));
  }

  SECTION("merge") {
    // merge some arbitrary numbers
    GrassForage g1, g2;
    const double W1 = 956;
    const double W2 = 123;
    const double M1 = 23;
    const double M2 = 54;
    const double D1 = 0.342;
    const double D2 = 0.56;
    const double F1 = 0.76;
    const double F2 = 0.123;
    g1.set_mass(M1);
    g2.set_mass(M2);
    g1.set_digestibility(D1);
    g2.set_digestibility(D2);
    g1.set_fpc(F1);
    g2.set_fpc(F2);

    g1.merge(g2, W1, W2);
    CHECK(g1.get_mass() == Approx(average(M1, M2, W1, W2)));
    CHECK(g1.get_digestibility() == Approx(average(D1, D2, W1, W2)));
    CHECK(g1.get_fpc() == Approx(average(F1, F2, W1, W2)));
  }
}

TEST_CASE("Fauna::Habitat", "") {
  // Since Habitat is an abstract class, we use the simple
  // class DummyHabitat for testing the base class functionality.

  DummyHabitat habitat;

  SECTION("init_day()") {
    // init_day()
    CHECK_THROWS(habitat.init_day(-1));
    CHECK_THROWS(habitat.init_day(365));
    const int DAY = 34;
    habitat.init_day(DAY);
    CHECK(habitat.get_day_public() == DAY);
  }

  SECTION("output") {
    // initialized with zero output.
    const int COUNT = 22;            // day count (even number!)
    const ForageMass EATEN_AVG(54);  // eaten total per day

    habitat.init_day(23);

    // remove twice in the same day, but keeping sum to given value
    habitat.remove_eaten_forage(EATEN_AVG * 0.4);
    habitat.remove_eaten_forage(EATEN_AVG * 0.6);

    SECTION("check this day") {
      const HabitatData out = ((const Habitat&)habitat).get_todays_output();
      // Check eaten forage per day as a sample.
      CHECK(out.eaten_forage[FT_GRASS] == Approx(EATEN_AVG[FT_GRASS]));
    }

    SECTION("init next day") {
      habitat.init_day(24);
      const HabitatData out = ((const Habitat&)habitat).get_todays_output();
      // The values should be reset to zero.
      CHECK(out.eaten_forage[FT_GRASS] == 0.0);
    }
  }
}

TEST_CASE("Fauna::HabitatForage", "") {
  HabitatForage hf1 = HabitatForage();

  // Initialization
  REQUIRE(hf1.get_total().get_mass() == Approx(0.0));
  REQUIRE(hf1.get_total().get_digestibility() == Approx(0.0));

  SECTION("adding forage") {
    const double GRASSMASS = 10.0;        // dry matter [kgDM/km²]
    const double NMASS = GRASSMASS * .1;  // nitrogen [kgN/km²]
    hf1.grass.set_mass(GRASSMASS);
    hf1.grass.set_digestibility(0.5);
    hf1.grass.set_fpc(0.3);

    // Nitrogen
    CHECK_THROWS(hf1.grass.set_nitrogen_mass(GRASSMASS * 1.1));
    hf1.grass.set_nitrogen_mass(NMASS);
    CHECK(hf1.grass.get_nitrogen_mass() == NMASS);
    CHECK(hf1.get_nitrogen_content()[FT_GRASS] == Approx(NMASS / GRASSMASS));
    CHECK_THROWS(hf1.grass.set_mass(NMASS * .9));

    // Check value access
    REQUIRE(hf1.grass.get_mass() == GRASSMASS);
    CHECK(hf1.grass.get_mass() == hf1.get_mass()[FT_GRASS]);
    REQUIRE(hf1.get_total().get_mass() == GRASSMASS);
    CHECK(hf1.get_total().get_mass() == Approx(hf1.get_mass().sum()));
    REQUIRE(hf1.get_total().get_digestibility() == 0.5);
  }

  // The member function `merge()` is not tested here
  // because it is a nothing more than simple wrapper around
  // the merge functions of ForageBase and its child classes.
}

TEST_CASE("Fauna::HalfMaxIntake") {
  CHECK_THROWS(HalfMaxIntake(-1, 1));
  CHECK_THROWS(HalfMaxIntake(0, 1));
  CHECK_THROWS(HalfMaxIntake(1, 0));
  CHECK_THROWS(HalfMaxIntake(1, -1));

  const double V_HALF = 10.0;   // half-saturation density
  const double MAX_RATE = 200;  // maximum intake rate
  const HalfMaxIntake h(V_HALF, MAX_RATE);

  CHECK_THROWS(h.get_intake_rate(-1));
  CHECK(h.get_intake_rate(0.0) == 0.0);
  CHECK(h.get_intake_rate(10.0) == Approx(MAX_RATE * 10.0 / (V_HALF + 10.0)));
}

TEST_CASE("Fauna::HerbivoreBase", "") {
  // Since HerbivoreBase cannot be instantiated directly, we
  // test the relevant functionality in HerbivoreBaseDummy.

  // PREPARE SETTINGS
  Parameters params;
  REQUIRE(params.is_valid());
  Hft hft = create_hfts(1, params)[0];
  REQUIRE(hft.is_valid(params));

  // Let’s throw some exceptions
  CHECK_THROWS(HerbivoreBaseDummy(-1, 0.5, &hft,  // age_days
                                  SEX_MALE));
  CHECK_THROWS(HerbivoreBaseDummy(100, 0.5, NULL,  // hft== NULL
                                  SEX_MALE));
  CHECK_THROWS(HerbivoreBaseDummy(100, 1.1, &hft,  // body_conditon
                                  SEX_MALE));
  CHECK_THROWS(HerbivoreBaseDummy(100, -0.1, &hft,  // body_conditon
                                  SEX_MALE));

  SECTION("Body mass") {
    SECTION("Birth") {
      // call the birth constructor
      const HerbivoreBaseDummy birth(&hft, SEX_MALE);

      REQUIRE(&birth.get_hft() == &hft);
      REQUIRE(birth.get_age_days() == 0);
      REQUIRE(birth.get_age_years() == 0);

      const double lean_bodymass_birth =
          hft.bodymass_birth * (1.0 - hft.bodyfat_birth);
      const double pot_bodymass_birth =
          lean_bodymass_birth / (1.0 - hft.bodyfat_max);
      // body mass
      CHECK(birth.get_bodymass() == Approx(hft.bodymass_birth));
      CHECK(birth.get_potential_bodymass() == Approx(pot_bodymass_birth));
      CHECK(birth.get_lean_bodymass() == Approx(lean_bodymass_birth));
      // fat mass
      CHECK(birth.get_bodyfat() == Approx(hft.bodyfat_birth));
      CHECK(birth.get_max_fatmass() ==
            Approx(pot_bodymass_birth * hft.bodyfat_max));
    }

    SECTION("Pre-adult") {
      const double BODY_COND = 1.0;
      SECTION("pre-adult male") {
        const int AGE_YEARS = hft.maturity_age_phys_male / 2;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy male_young(AGE_DAYS, BODY_COND, &hft,
                                            SEX_MALE);
        REQUIRE(male_young.get_age_days() == AGE_DAYS);
        REQUIRE(male_young.get_age_years() == AGE_YEARS);
        CHECK(male_young.get_bodymass() < hft.bodymass_male);
        CHECK(male_young.get_bodymass() > hft.bodymass_birth);
        CHECK(male_young.get_fatmass() / male_young.get_max_fatmass() ==
              Approx(BODY_COND));
      }

      SECTION("pre-adult female") {
        const int AGE_YEARS = hft.maturity_age_phys_female / 2;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy female_young(AGE_DAYS, BODY_COND, &hft,
                                              SEX_FEMALE);
        REQUIRE(female_young.get_age_days() == AGE_DAYS);
        REQUIRE(female_young.get_age_years() == AGE_YEARS);
        CHECK(female_young.get_bodymass() < hft.bodymass_female);
        CHECK(female_young.get_bodymass() > hft.bodymass_birth);
        CHECK(female_young.get_fatmass() / female_young.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }

    SECTION("Adult with full fat") {
      const double BODY_COND = 1.0;  // optimal body condition
      SECTION("Adult male with full fat") {
        const int AGE_YEARS = hft.maturity_age_phys_male;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy male_adult(AGE_DAYS, BODY_COND, &hft,
                                            SEX_MALE);
        // AGE
        REQUIRE(male_adult.get_age_days() == AGE_DAYS);
        REQUIRE(male_adult.get_age_years() == AGE_YEARS);
        // BODY MASS
        CHECK(male_adult.get_bodymass() == Approx(hft.bodymass_male));
        CHECK(male_adult.get_potential_bodymass() == male_adult.get_bodymass());
        CHECK(male_adult.get_lean_bodymass() ==
              Approx(hft.bodymass_male * (1.0 - hft.bodyfat_max)));
        // FAT MASS
        CHECK(male_adult.get_max_fatmass() ==
              Approx(hft.bodyfat_max * hft.bodymass_male));
        CHECK(male_adult.get_bodyfat() == Approx(hft.bodyfat_max));
        CHECK(male_adult.get_fatmass() / male_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
      SECTION("Adult female with full fat") {
        const int AGE_YEARS = hft.maturity_age_phys_female;
        const int AGE_DAYS = AGE_YEARS * 365;
        const HerbivoreBaseDummy female_adult(hft.maturity_age_phys_male * 365,
                                              BODY_COND, &hft, SEX_FEMALE);
        // AGE
        REQUIRE(female_adult.get_age_days() == AGE_DAYS);
        REQUIRE(female_adult.get_age_years() == AGE_YEARS);
        // BODY MASS
        CHECK(female_adult.get_bodymass() == Approx(hft.bodymass_female));
        CHECK(female_adult.get_potential_bodymass() ==
              female_adult.get_bodymass());
        CHECK(female_adult.get_lean_bodymass() ==
              Approx(hft.bodymass_female * (1.0 - hft.bodyfat_max)));
        // FAT MASS
        CHECK(female_adult.get_max_fatmass() ==
              Approx(hft.bodyfat_max * hft.bodymass_female));
        CHECK(female_adult.get_bodyfat() == Approx(hft.bodyfat_max));
        CHECK(female_adult.get_fatmass() / female_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }

    SECTION("Adult with low fat") {
      const double BODY_COND = 0.3;  // poor body condition

      SECTION("Male") {
        const HerbivoreBaseDummy male_adult(hft.maturity_age_phys_male * 365,
                                            BODY_COND, &hft, SEX_MALE);
        // BODY MASS
        CHECK(male_adult.get_potential_bodymass() == Approx(hft.bodymass_male));
        CHECK(male_adult.get_lean_bodymass() + male_adult.get_max_fatmass() ==
              Approx(male_adult.get_potential_bodymass()));
        // FAT MASS
        CHECK(male_adult.get_max_fatmass() ==
              Approx(hft.bodyfat_max * hft.bodymass_male));
        CHECK(male_adult.get_fatmass() / male_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }

      SECTION("Female") {
        const HerbivoreBaseDummy female_adult(hft.maturity_age_phys_male * 365,
                                              BODY_COND, &hft, SEX_FEMALE);
        // BODY MASS
        CHECK(female_adult.get_potential_bodymass() ==
              Approx(hft.bodymass_female));
        CHECK(female_adult.get_lean_bodymass() +
                  female_adult.get_max_fatmass() ==
              Approx(female_adult.get_potential_bodymass()));
        // FAT MASS
        CHECK(female_adult.get_max_fatmass() ==
              Approx(hft.bodyfat_max * hft.bodymass_female));
        CHECK(female_adult.get_fatmass() / female_adult.get_max_fatmass() ==
              Approx(BODY_COND));
      }
    }
  }
}

TEST_CASE("Fauna::HerbivoreCohort", "") {
  // PREPARE SETTINGS
  Parameters params;
  REQUIRE(params.is_valid());
  Hft hft = create_hfts(1, params)[0];
  REQUIRE(hft.is_valid(params));

  // exceptions (only specific to HerbivoreCohort)
  // initial density negative
  CHECK_THROWS(HerbivoreCohort(10, 0.5, &hft, SEX_MALE, -1.0));

  const double BC = 0.5;  // body condition
  const int AGE = 3 * 365;
  const double DENS = 10.0;  // [ind/km²]

  // constructor (only test what is specific to HerbivoreCohort)
  REQUIRE(HerbivoreCohort(AGE, BC, &hft, SEX_MALE, DENS).get_ind_per_km2() ==
          Approx(DENS));

  SECTION("is_same_age()") {
    REQUIRE(AGE % 365 == 0);
    const HerbivoreCohort cohort1(AGE, BC, &hft, SEX_MALE, DENS);
    // very similar cohort
    CHECK(cohort1.is_same_age(HerbivoreCohort(AGE, BC, &hft, SEX_MALE, DENS)));
    // in the same year
    CHECK(cohort1.is_same_age(
        HerbivoreCohort(AGE + 364, BC, &hft, SEX_MALE, DENS)));
    // the other is younger
    CHECK(!cohort1.is_same_age(
        HerbivoreCohort(AGE - 364, BC, &hft, SEX_MALE, DENS)));
    // the other is much older
    CHECK(!cohort1.is_same_age(
        HerbivoreCohort(AGE + 366, BC, &hft, SEX_MALE, DENS)));
  }

  SECTION("merge") {
    HerbivoreCohort cohort(AGE, BC, &hft, SEX_MALE, DENS);

    SECTION("exceptions") {
      SECTION("wrong age") {  // wrong age
        HerbivoreCohort other(AGE + 365, BC, &hft, SEX_MALE, DENS);
        CHECK_THROWS(cohort.merge(other));
      }
      SECTION("wrong sex") {  // wrong sex
        HerbivoreCohort other(AGE, BC, &hft, SEX_FEMALE, DENS);
        CHECK_THROWS(cohort.merge(other));
      }
      SECTION("wrong HFT") {  // wrong HFT
        Hft hft2 = create_hfts(2, params)[1];
        REQUIRE(hft2 != hft);
        HerbivoreCohort other(AGE, BC, &hft2, SEX_MALE, DENS);
        CHECK_THROWS(cohort.merge(other));
      }
    }

    SECTION("merge whole cohort") {
      const double old_bodymass = cohort.get_bodymass();
      const double BC2 = BC + 0.1;  // more fat in the other cohort
      const double DENS2 = DENS * 1.5;
      HerbivoreCohort other(AGE, BC2, &hft, SEX_MALE, DENS2);
      cohort.merge(other);
      // The other cohort is gone
      CHECK(other.get_kg_per_km2() == 0.0);
      // More fat => more bodymass
      CHECK(cohort.get_bodymass() > old_bodymass);
      // That’s all we can test from the public methods...
    }
  }

  SECTION("mortality") {}
}

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
  CHECK_THROWS(HerbivoreIndividual(AGE, BC, &hft, SEX_MALE, -1.0));
  CHECK_THROWS(HerbivoreIndividual(AGE, BC, &hft, SEX_MALE, 0.0));
  CHECK_THROWS(HerbivoreIndividual(&hft, SEX_MALE, -1.0));
  CHECK_THROWS(HerbivoreIndividual(&hft, SEX_MALE, 0.0));

  // birth constructor
  REQUIRE(HerbivoreIndividual(&hft, SEX_MALE, AREA).get_area_km2() ==
          Approx(AREA));
  // establishment constructor
  REQUIRE(HerbivoreIndividual(AGE, BC, &hft, SEX_MALE, AREA).get_area_km2() ==
          Approx(AREA));

  SECTION("Mortality") {
    hft.mortality_factors.insert(MF_STARVATION_THRESHOLD);

    // create with zero fat reserves
    const double BC_DEAD = 0.0;  // body condition
    HerbivoreIndividual ind(AGE, BC_DEAD, &hft, SEX_MALE, AREA);

    // after one simulation day it should be dead
    double offspring_dump;   // ignored
    HabitatEnvironment env;  // ignored
    ind.simulate_day(0, env, offspring_dump);
    CHECK(ind.is_dead());
  }
  // NOTE: We cannot test mortality because it is a stochastic
  // event.
}

TEST_CASE("Fauna::Hft", "") {
  Hft hft = Hft();
  std::string msg;

  SECTION("not valid without name") {
    hft.name = "";
    CHECK_FALSE(hft.is_valid(Fauna::Parameters(), msg));
  }
}

TEST_CASE("Fauna::HftList", "") {
  HftList hftlist;

  // check initial size
  REQUIRE(hftlist.size() == 0);

  // invalid access
  CHECK_THROWS(hftlist[1]);
  CHECK_THROWS(hftlist[-1]);
  CHECK_THROWS(hftlist["abc"]);

  // add Hft without name
  Hft noname;
  noname.name = "";
  CHECK_THROWS(hftlist.insert(noname));

  // add some real HFTs
  Hft hft1;
  hft1.name = "hft1";
  hft1.is_included = true;
  REQUIRE_NOTHROW(hftlist.insert(hft1));
  REQUIRE(hftlist.size() == 1);
  REQUIRE(hftlist[0].name == "hft1");
  REQUIRE(hftlist.begin()->name == "hft1");

  Hft hft2;
  hft2.name = "hft2";
  hft2.is_included = false;
  REQUIRE_NOTHROW(hftlist.insert(hft2));
  REQUIRE(hftlist.size() == 2);
  REQUIRE_NOTHROW(hftlist[1]);

  // find elements
  CHECK(hftlist["hft2"].name == "hft2");
  CHECK(hftlist["hft1"].name == "hft1");
  CHECK(hftlist.contains("hft1"));
  CHECK(hftlist.contains("hft2"));
  CHECK_FALSE(hftlist.contains("abc"));

  // substitute element
  hft2.lifespan += 2;  // change a property outside list
  REQUIRE(hftlist[hft2.name].lifespan != hft2.lifespan);
  hftlist.insert(hft2);  // replace existing
  CHECK(hftlist[hft2.name].lifespan == hft2.lifespan);

  // remove excluded
  hftlist.remove_excluded();
  CHECK(hftlist.size() == 1);
  CHECK(hftlist.contains(hft1.name));        // hft1 included
  CHECK_FALSE(hftlist.contains(hft2.name));  // hft2 NOT included
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

TEST_CASE("Fauna::NitrogenInHerbivore") {
  NitrogenInHerbivore n;

  // Exceptions
  CHECK_THROWS(n.ingest(-.1));
  CHECK_THROWS(n.digest_today(-.1, 1.0));
  CHECK_THROWS(n.digest_today(0.0, 1.0));
  CHECK_THROWS(n.digest_today(1.0, -.1));

  // Initialization
  CHECK(n.get_excreta() == 0.0);
  CHECK(n.get_unavailable() == 0.0);

  SECTION("Ingestion-digestion cycle") {
    double ingested = 0.0;
    double soil = 0.0;   // nitrogen from excreta in the soil
    double total = 0.0;  // total nitrogen in ecoystem, as it should be
    const double RETENTION_TIME = 80;  // [hours]
    const double MASSDENS = 0;         // herbivore density [kg/km²]
    int hours = 0;                     // number of hours since first feeding
    for (int i = 0; i < 20; i++) {
      const double new_ingested = 1 + i % 2;  // just some positive numbers
      n.ingest(new_ingested);
      total += new_ingested;
      CHECK(total == soil + n.get_excreta() + n.get_unavailable());

      // every couple of feeding bouts, digest the nitrogen
      if (!(i % 3)) {
        hours += 24;
        n.digest_today(RETENTION_TIME, MASSDENS);
      }

      // put the excreta in the soil pool at some arbitrary interval
      if (!(i % 6)) {
        // Once the first feeding bout has passed the digestive tract,
        // there should be some excreta produced.
        INFO("hours = " << hours);
        if (hours > RETENTION_TIME) CHECK(n.get_excreta() > 0.0);

        soil += n.reset_excreta();
        CHECK(n.get_excreta() == 0);
      }
    }
  }

  SECTION("Tissue nitrogen") {
    const double MASSDENS = 10.0;       // [kg/km²]
    const double RETENTION_TIME = 1.0;  // hours
    const double INGESTED = MASSDENS;   // [kgN/km²]
    n.ingest(INGESTED);

    // Pass all nitrogen through the digestive tract.
    n.digest_today(RETENTION_TIME, MASSDENS);
    n.digest_today(RETENTION_TIME, MASSDENS);

    // Now, all nitrogen should be excreted, and only the tissue nitrogen
    // should be left in the unavailable pool.
    CHECK(n.get_unavailable() ==
          Approx(MASSDENS * NitrogenInHerbivore::N_CONTENT_IN_TISSUE));
    CHECK(n.get_unavailable() + n.get_excreta() == Approx(INGESTED));

    CHECK(n.reset_total() == Approx(INGESTED));
    CHECK(n.get_unavailable() == 0.0);
    CHECK(n.get_excreta() == 0.0);
  }
}

TEST_CASE("Fauna::Parameters", "") {
  // defaults must be valid.
  REQUIRE(Parameters().is_valid());
}

TEST_CASE("Fauna::parse_comma_separated_param", "") {
  CHECK(parse_comma_separated_param("").empty());
  CHECK(*parse_comma_separated_param("abc").begin() == "abc");
  CHECK(*parse_comma_separated_param("abc,def").begin() == "abc");
  CHECK(*(++parse_comma_separated_param("abc,def").begin()) == "def");
  CHECK(*parse_comma_separated_param("abc, def").begin() == "abc");
  CHECK(*(++parse_comma_separated_param("abc, def").begin()) == "def");
  CHECK(*parse_comma_separated_param("abc,    def  ").begin() == "abc");
  CHECK(*(++parse_comma_separated_param(" abc,    def").begin()) == "def");
}

TEST_CASE("Fauna::PeriodAverage") {
  CHECK_THROWS(PeriodAverage(-1));
  CHECK_THROWS(PeriodAverage(0));

  const int COUNT = 3;
  PeriodAverage pa(COUNT);

  CHECK_THROWS(pa.get_average());

  const double A = .1;
  const double B = .2;
  const double C = .4;
  const double D = .5;
  const double E = .6;

  pa.add_value(A);
  CHECK(pa.get_average() == A);

  pa.add_value(B);
  CHECK(pa.get_average() == Approx((A + B) / 2.0));

  pa.add_value(C);
  CHECK(pa.get_average() == Approx((A + B + C) / 3.0));

  pa.add_value(D);
  CHECK(pa.get_average() == Approx((B + C + D) / 3.0));

  pa.add_value(E);
  CHECK(pa.get_average() == Approx((C + D + E) / 3.0));
}

TEST_CASE("Fauna::ReprIlliusOconnor2000", "") {
  // NOTE: We are more tolerant with the Approx() function of
  // the CATCH framework (by adjusting Approx().epsilon().

  const double INC = 1.0;
  const int START = 100;
  const int LENGTH = 90;
  const double OPT = 1.0;  // optimal body condition

  const BreedingSeason season(START, LENGTH);

  // exceptions
  SECTION("exceptions") {
    CHECK_THROWS(ReprIlliusOconnor2000(season, -1.0));
    ReprIlliusOconnor2000 rep(season, INC);
    CHECK_THROWS(rep.get_offspring_density(-1, OPT));
    CHECK_THROWS(rep.get_offspring_density(365, OPT));
    CHECK_THROWS(rep.get_offspring_density(START, -0.1));
    CHECK_THROWS(rep.get_offspring_density(START, 1.1));
  }

  SECTION("higher annual increase makes more offspring") {
    const double INC2 = INC * 1.5;
    REQUIRE(INC2 > INC);
    ReprIlliusOconnor2000 rep1(season, INC);
    ReprIlliusOconnor2000 rep2(season, INC2);
    CHECK(rep1.get_offspring_density(START, OPT) <
          rep2.get_offspring_density(START, OPT));
    CHECK(rep1.get_offspring_density(START, OPT) < INC);
    CHECK(rep2.get_offspring_density(START, OPT) < INC2);
  }

  SECTION("better body condition makes more offspring") {
    const double BAD = OPT / 2.0;  // bad body condition
    ReprIlliusOconnor2000 rep(season, INC);
    CHECK(rep.get_offspring_density(START, BAD) <
          rep.get_offspring_density(START, OPT));
  }

  SECTION("one-day season length -> all offspring at once") {
    const double BAD = OPT / 2.0;  // bad body condition
    BreedingSeason season_short(START, 1);
    ReprIlliusOconnor2000 rep(season_short, INC);
    CHECK(rep.get_offspring_density(START, OPT) == Approx(INC).epsilon(0.05));
    CHECK(rep.get_offspring_density(START, BAD) < INC);

    SECTION("Check an absolute value for bad body condition") {
      CHECK(rep.get_offspring_density(START, BAD) ==
            Approx(INC / (1 + exp(-15.0 * (BAD - 0.3)))));
    }
  }

  SECTION("Sum of offspring over year must be max. annual increase") {
    ReprIlliusOconnor2000 rep(season, INC);
    // sum over whole year
    double sum_year = 0.0;
    for (int d = 0; d < 365; d++)
      sum_year += rep.get_offspring_density((START + d) % 364, OPT);
    CHECK(sum_year == Approx(INC).epsilon(0.05));

    // sum over the breeding season only
    double sum_season = 0.0;
    for (int d = START; d < START + LENGTH; d++)
      sum_season += rep.get_offspring_density(d, OPT);
    CHECK(sum_season == Approx(INC).epsilon(0.05));
  }
}

TEST_CASE("Fauna::SimulationUnit") {
  Hft HFT;
  CHECK_THROWS(SimulationUnit(std::auto_ptr<Habitat>(NULL),
                              std::auto_ptr<HftPopulationsMap>()));
  CHECK_THROWS(SimulationUnit(std::auto_ptr<Habitat>(new DummyHabitat()),
                              std::auto_ptr<HftPopulationsMap>(NULL)));
}

TEST_CASE("Fauna::Simulator", "") {
  Fauna::Parameters params;
  REQUIRE(params.is_valid());

  // prepare HFT list
  HftList hftlist = create_hfts(3, params);

  Simulator sim(params);

  SECTION("create_populations() for several HFTs") {
    std::auto_ptr<HftPopulationsMap> pops = sim.create_populations(hftlist);
    REQUIRE(pops.get() != NULL);
    CHECK(pops->size() == hftlist.size());
    // find all HFTs
    HftList::const_iterator itr_hft = hftlist.begin();
    while (itr_hft != hftlist.end()) {
      bool found_hft = false;
      HftPopulationsMap::const_iterator itr_pop = pops->begin();
      while (itr_pop != pops->end()) {
        if ((*itr_pop)->get_hft() == *itr_hft) found_hft = true;
        itr_pop++;
      }
      CHECK(found_hft);
      itr_hft++;
    }
  }

  SECTION("create_populations() for one HFT") {
    const Hft* phft = &hftlist[0];
    std::auto_ptr<HftPopulationsMap> pops = sim.create_populations(phft);
    REQUIRE(pops.get() != NULL);
    CHECK(pops->size() == 1);
    CHECK(&(*(pops->begin()))->get_hft() == phft);
  }

  SECTION("simulate_day()") {
    // Check simulate_day()
    std::auto_ptr<HftPopulationsMap> pops = sim.create_populations(hftlist);

    SimulationUnit simunit(
        std::auto_ptr<Habitat>(new DummyHabitat()),
        std::auto_ptr<HftPopulationsMap>(new HftPopulationsMap));
    CHECK_THROWS(sim.simulate_day(-1, simunit, true));
    CHECK_THROWS(sim.simulate_day(366, simunit, true));
    const bool do_herbivores = true;
    for (int d = 0; d < 365; d++) {
      sim.simulate_day(d, simunit, do_herbivores);

      // Check if day has been set correctly.
      CHECK(((DummyHabitat&)simunit.get_habitat()).get_day_public() == d);

      // Note: Various other things could be tested here.
      // But they are becoming more difficult to formulate as unit tests
      // because Simulator::simulate_day() and SimulateDay are on a high
      // level in the program hierarchy.
    }
  }
}

TEST_CASE("FaunaOut::CombinedData") {
  CombinedData c1, c2;
  CHECK(c1.datapoint_count == 0);

  // create some habitat data
  HabitatData hab1, hab2;
  hab1.available_forage.grass.set_mass(1.0);
  hab1.available_forage.grass.set_fpc(.5);
  hab2.available_forage.grass.set_mass(2.0);
  hab2.available_forage.grass.set_fpc(.5);

  // put habitat data into CombinedData
  c1.habitat_data = hab1;
  c2.habitat_data = hab2;

  // create some herbivore data

  const HftList hfts = create_hfts(2, Parameters());
  REQUIRE(hfts.size() == 2);

  HerbivoreData h1, h2;  // HFT 0
  HerbivoreData h3;      // HFT 1

  h1.inddens = 1.0;
  h2.inddens = 2.0;
  h3.inddens = 3.0;
  h1.expenditure = 1.0;
  h2.expenditure = 2.0;
  h3.expenditure = 3.0;

  // put the herbivore data into CombinedData

  c1.hft_data[&hfts[0]] = h1;
  // note: c2 has no entry for HFT 1
  c1.hft_data[&hfts[1]] = h3;
  c2.hft_data[&hfts[0]] = h2;

  SECTION("merge() with zero datapoint count") {
    c1.datapoint_count = 0;
    c2.datapoint_count = 1;

    c2.merge(c1);

    // c2 should be left unchanged

    CHECK(c2.datapoint_count == 1);
    CHECK(c2.hft_data.size() == 1);
    CHECK(c2.habitat_data.available_forage.grass.get_mass() == 2.0);
  }

  SECTION("merge() with equal datapoint counts") {
    // merge with equal weight
    c1.datapoint_count = c2.datapoint_count = 3;

    c1.merge(c2);

    CHECK(c1.datapoint_count == 6);

    CHECK(c1.habitat_data.available_forage.grass.get_mass() == Approx(1.5));
    // c1 now has data for both HFTs
    REQUIRE(c1.hft_data.size() == 2);
    // in HFT 0, two datapoints are merged
    CHECK(c1.hft_data[&hfts[0]].inddens ==
          Approx((1.0 + 2.0) / 2.0));  // normal average
    CHECK(
        c1.hft_data[&hfts[0]].expenditure ==
        Approx((1.0 * 1.0 + 2.0 * 2.0) / (1.0 + 2.0)));  // weighted by inddens

    // in HFT 1, one datapoint is merged with zero values
    CHECK(c1.hft_data[&hfts[1]].inddens ==
          Approx((0.0 + 3.0) / 2.0));  // normal average
    CHECK(c1.hft_data[&hfts[1]].expenditure ==
          Approx(3.0));  // weighted by inddens, but only one data point
  }

  SECTION("merge() with different data point counts") {
    c1.datapoint_count = 1;
    c2.datapoint_count = 2;

    c1.merge(c2);

    CHECK(c1.datapoint_count == 3);

    CHECK(c1.habitat_data.available_forage.grass.get_mass() ==
          Approx((1.0 * 1.0 + 2.0 * 2.0) / (3.0)));
    // c1 now has data for both HFTs
    REQUIRE(c1.hft_data.size() == 2);

    // in HFT 0, two datapoints are merged
    CHECK(c1.hft_data[&hfts[0]].inddens ==
          Approx((1.0 + 2.0 * 2.0) / 3.0));  // weighted by datapoint_count

    // weighted by inddens*datapoint_count:
    CHECK(c1.hft_data[&hfts[0]].expenditure ==
          Approx((1.0 + 2.0 * 2.0 * 2.0) / (1.0 + 2.0 * 2.0)));

    // in HFT 1, one datapoint is merged with zero values
    CHECK(c1.hft_data[&hfts[1]].inddens ==
          Approx((1.0 * 3.0 + 2.0 * 0.0) /
                 (1.0 + 2.0)));  // weighted by datapoint_count

    // weighted by inddens*datapoint_count, but since c2 has no data for
    // HFT 1, and since expenditure is individual based, c2 is simply not
    // included in the average.
    CHECK(c1.hft_data[&hfts[1]].expenditure == h3.expenditure);
  }

  SECTION("reset()") {
    c1.reset();
    CHECK(c1.datapoint_count == 0);
    CHECK(c1.hft_data.empty());
    CHECK(c1.habitat_data.available_forage.get_mass().sum() == 0.0);
  }
}

TEST_CASE("FaunaOut::HabitatData", "") {
  SECTION("Exceptions") {
    HabitatData d1, d2;
    CHECK_THROWS(d1.merge(d2, 0, 0));
    CHECK_THROWS(d1.merge(d2, -1, 1));
    CHECK_THROWS(d1.merge(d2, 1, -1));
  }

  // The values of the merge are not checked here because
  // they are given by Fauna::ForageValues<>::merge()
  // and Fauna::average().
}

TEST_CASE("FaunaOut::HerbivoreData", "") {
  SECTION("Exceptions") {
    HerbivoreData d1, d2;
    CHECK_THROWS(d1.merge(d2, 0, 0));
    CHECK_THROWS(d1.merge(d2, -1, 1));
    CHECK_THROWS(d1.merge(d2, 1, -1));
    CHECK_THROWS(HerbivoreData::create_datapoint(std::vector<HerbivoreData>()));
  }

  SECTION("create_datapoint()") {
    HerbivoreData d0, d1, d2, d3;
    REQUIRE(d0.inddens == 0.0);  // zero initialization

    // prepare some arbitrary data
    d1.inddens = 1;
    d2.inddens = 2;
    d3.inddens = 3;
    d1.expenditure = 1;
    d2.expenditure = 2;
    d3.expenditure = 3;
    d1.mortality[MF_BACKGROUND] = .1;
    d2.mortality[MF_BACKGROUND] = .2;
    d3.mortality[MF_BACKGROUND] = .3;
    d1.mortality[MF_LIFESPAN] = .5;

    // put them in a vector
    std::vector<HerbivoreData> vec;
    vec.push_back(d1);
    vec.push_back(d2);
    vec.push_back(d3);

    // create a datapoint
    HerbivoreData datapoint = HerbivoreData::create_datapoint(vec);

    // check if values are okay
    CHECK(datapoint.inddens == Approx(6.0));  // sum of all items
    // averages weighted by inddens:
    CHECK(datapoint.expenditure == Approx((1.0 + 2.0 * 2.0 + 3.0 * 3.0) / 6.0));
    CHECK(datapoint.mortality[MF_BACKGROUND] ==
          Approx((.1 + .2 * 2.0 + .3 * 3.0) / 6.0));
    // this mortality factor was only present in one item:
    CHECK(datapoint.mortality[MF_LIFESPAN] == Approx(.5));
  }

  SECTION("merge()") {
    HerbivoreData d1, d2;
    d1.inddens = 1;
    d2.inddens = 2;
    d1.expenditure = 1.0;
    d2.expenditure = 2.0;
    d1.mortality[MF_BACKGROUND] = .1;
    d2.mortality[MF_BACKGROUND] = .2;
    d1.mortality[MF_LIFESPAN] = .5;
    // no lifespan mortality in d2

    SECTION("equal weights") {
      d1.merge(d2, 1.0, 1.0);
      // simple average:
      CHECK(d1.inddens == Approx(1.5));
      CHECK(d1.mortality[MF_BACKGROUND] == Approx(.15));
      CHECK(d1.mortality[MF_LIFESPAN] == 0.0);  // was only in one datapoint
      // average weighted by inddens:
      CHECK(d1.expenditure == Approx((1.0 + 2.0 * 2.0) / 3.0));
    }

    SECTION("different weights") {
      d1.merge(d2, 1.0, 2.0);
      // simple average:
      CHECK(d1.inddens == Approx((1.0 + 2.0 * 2.0) / (1.0 + 2.0)));
      CHECK(d1.mortality[MF_BACKGROUND] ==
            Approx((.1 + 2.0 * .2) / (1.0 + 2.0)));
      CHECK(d1.mortality[MF_LIFESPAN] == 0.0);  // was only in one datapoint
      // average weighted by inddens:
      CHECK(d1.expenditure ==
            Approx((1.0 + 2.0 * 2.0 * 2.0) / (1.0 + 2.0 * 2.0)));
    }
  }
}

TEST_CASE("FaunaSim::LogisticGrass", "") {
  LogisticGrass::Parameters grass_settings;
  grass_settings.reserve = 2.0;  // just an arbitrary positive number
  grass_settings.init_mass = 1.0;
  grass_settings.saturation = 10 * grass_settings.init_mass;

  const int day = 1;  // day of the year

  SECTION("Grass initialization") {
    LogisticGrass grass(grass_settings);
    CHECK(grass.get_forage().get_mass() == Approx(grass_settings.init_mass));
    CHECK(grass.get_forage().get_digestibility() ==
          Approx(grass_settings.digestibility[0]));
    CHECK(grass.get_forage().get_fpc() == Approx(grass_settings.fpc));

    // exceptions
    CHECK_THROWS(grass.grow_daily(-1));
    CHECK_THROWS(grass.grow_daily(365));
  }

  // Let the grass grow for one day and compare before and after
  // Now we fill in new growth and decay values, so we need to remove the
  // default first.
  grass_settings.growth_monthly.clear();
  grass_settings.decay_monthly.clear();

  SECTION("No grass growth") {
    grass_settings.growth_monthly.push_back(0.0);
    grass_settings.decay_monthly.push_back(0.0);

    LogisticGrass grass(grass_settings);

    const GrassForage before = grass.get_forage();
    grass.grow_daily(day);
    const GrassForage after = grass.get_forage();

    CHECK(after.get_mass() == Approx(before.get_mass()));
  }

  SECTION("Positive grass growth") {
    grass_settings.growth_monthly.push_back(0.1);
    grass_settings.decay_monthly.push_back(0.0);

    LogisticGrass grass(grass_settings);

    // Let the grass grow for one day and check it’s greater
    INFO("grass_settings.growth_monthly == "
         << grass_settings.growth_monthly[0]);
    INFO("grass_settings.decay_monthly == " << grass_settings.decay_monthly[0]);

    const GrassForage before = grass.get_forage();
    grass.grow_daily(day);
    const GrassForage after = grass.get_forage();

    CHECK(after.get_mass() > before.get_mass());

    // Let it grow for very long and check that it reaches saturation
    for (int i = 0; i < 1000000; i++) grass.grow_daily(i % 365);
    CHECK(grass.get_forage().get_mass() == Approx(grass_settings.saturation));
  }

  SECTION("Negative grass growth") {
    // decay is greater than growth
    grass_settings.growth_monthly.push_back(0.1);
    grass_settings.decay_monthly.push_back(0.2);

    LogisticGrass grass(grass_settings);

    // Let the grass grow for one day and check it’s greater

    const GrassForage before = grass.get_forage();
    grass.grow_daily(day);
    const GrassForage after = grass.get_forage();

    CHECK(after.get_mass() < before.get_mass());
  }

  SECTION("Multi-months growth") {
    const double GROWTH = 0.001;

    // Growth in first month
    grass_settings.growth_monthly.push_back(GROWTH);
    // Growth in second month
    grass_settings.growth_monthly.push_back(0.0);

    // Decay is constant zero. It’s not tested explicitly, but we assume
    // that it works like growth.
    grass_settings.decay_monthly.push_back(0.0);

    LogisticGrass grass(grass_settings);

    int d = 0;
    // Let it grow for January (31 days)
    for (; d <= 31 - 1; d++) {
      const GrassForage before = grass.get_forage();
      grass.grow_daily(d);
      const GrassForage after = grass.get_forage();

      INFO("day == " << d);
      // Check that growth happened or maximum is reached
      if (after.get_mass() < grass_settings.saturation)
        CHECK(after.get_mass() > before.get_mass());
    }

    // Let it grow for February (28 days)
    for (; d <= 31 + 28 - 1; d++) {
      const GrassForage before = grass.get_forage();
      grass.grow_daily(d);
      const GrassForage after = grass.get_forage();

      INFO("day == " << d);
      // Check that growth happened or maximum is reached
      CHECK(after.get_mass() == Approx(before.get_mass()));
    }

    // Let it grow for March (31 days), now recycling the first value
    for (; d <= 31 + 28 + 31 - 1; d++) {
      const GrassForage before = grass.get_forage();
      grass.grow_daily(d);
      const GrassForage after = grass.get_forage();

      INFO("day == " << d);
      // Check that growth happened or maximum is reached
      if (after.get_mass() < grass_settings.saturation)
        CHECK(after.get_mass() > before.get_mass());
    }
  }
}

TEST_CASE("FaunaSim::SimpleHabitat", "") {
  SimpleHabitat::Parameters settings;
  settings.grass.init_mass = 1.0;
  settings.grass.saturation = 3.0;

  // create a habitat with some populations
  const Fauna::Parameters params;
  Simulator sim(params);
  SimpleHabitat habitat(settings);

  SECTION("Initialization") {
    CHECK(habitat.get_available_forage().grass.get_fpc() ==
          Approx(settings.grass.fpc));
    CHECK(habitat.get_available_forage().grass.get_mass() ==
          Approx(settings.grass.init_mass));
    CHECK(habitat.get_available_forage().grass.get_digestibility() ==
          Approx(settings.grass.digestibility[0]));
  }

  SECTION("Remove forage") {
    const HabitatForage avail = habitat.get_available_forage();

    SECTION("Remove some forage") {
      const ForageMass eaten = avail.get_mass() * 0.5;
      habitat.remove_eaten_forage(eaten);
      // check each forage type with Approx()
      for (ForageMass::const_iterator i = eaten.begin(); i != eaten.end(); i++)
        CHECK(habitat.get_available_forage().get_mass()[i->first] ==
              Approx(avail.get_mass()[i->first] - i->second));
    }

    SECTION("Remove all forage") {
      const ForageMass eaten = avail.get_mass();
      habitat.remove_eaten_forage(eaten);
      for (ForageMass::const_iterator i = eaten.begin(); i != eaten.end(); i++)
        CHECK(habitat.get_available_forage().get_mass()[i->first] ==
              Approx(0.0));
    }

    SECTION("Remove more forage than is available") {
      const ForageMass too_much = avail.get_mass() * 1.1;
      CHECK_THROWS(habitat.remove_eaten_forage(too_much));
    }
  }
}

TEST_CASE("FaunaSim::HabitatGroup", "") {
  // Make sure the group creates its habitats
  HabitatGroup group(1.0, 1.0);  // lon,lat
  group.reserve(5);
  for (int i = 1; i < 5; i++) {
    // create a simulation unit
    std::auto_ptr<Habitat> habitat(new DummyHabitat());
    std::auto_ptr<HftPopulationsMap> populations(new HftPopulationsMap());
    std::auto_ptr<SimulationUnit> simunit(
        new SimulationUnit(habitat, populations));
    // add it to the group
    group.add(simunit);
    // Check if it has been added properly
    CHECK(group.size() == i);
    CHECK(group.get_vector().size() == i);
  }
  // Make sure the references are pointing correctly to the objects
  const std::vector<SimulationUnit*> refs = group.get_vector();
  HabitatGroup::const_iterator itr = group.begin();
  int j = 0;
  while (itr != group.end()) {
    CHECK(refs[j] == *itr);
    j++;
    itr++;
  }
}

TEST_CASE("FaunaSim::HabitatGroupList", "") {
  // Make sure the group creates its habitats
  HabitatGroupList gl;
  gl.reserve(5);

  // add some habitat groups
  for (int i = 1; i < 5; i++) {
    HabitatGroup& group =
        gl.add(std::auto_ptr<HabitatGroup>(new HabitatGroup(i, i)));
    for (int j = 1; j < 4; j++) {
      // create a simulation unit
      std::auto_ptr<Habitat> habitat(new DummyHabitat());
      std::auto_ptr<HftPopulationsMap> populations(new HftPopulationsMap());
      std::auto_ptr<SimulationUnit> simunit(
          new SimulationUnit(habitat, populations));
      // add it to the group
      group.add(simunit);
    }
    CHECK(gl.size() == i);
  }
  // Don’t allow adding a group with same coordinates twice
  CHECK_THROWS(gl.add(std::auto_ptr<HabitatGroup>(new HabitatGroup(1, 1))));
}

// TEST_CASE("Fauna::is_first_day_of_month()", ""){
// 	CHECK_THROWS( is_first_day_of_month(-1) );
// 	CHECK_THROWS( is_first_day_of_month(365) );
// 	CHECK( is_first_day_of_month(0) );
// 	CHECK_FALSE( is_first_day_of_month(1) );
// 	CHECK_FALSE( is_first_day_of_month(44) );
// 	CHECK( is_first_day_of_month(-1+31) );
// 	CHECK( is_first_day_of_month(-1+31+28) );
// 	CHECK( is_first_day_of_month(-1+31+28+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31+30) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31+30+31) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31+30+31+30) );
// 	CHECK( is_first_day_of_month(-1+31+28+31+30+31+30+31+31+30+31+30+31) );
// }

//////////////////////////////////////////////////////////////
// REFERENCES
// Blaxter, Kenneth (1989). Energy Metabolism in Animals and Man. CUP Archive.
// 356 pp.
