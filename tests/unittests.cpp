//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Unit tests for megafauna herbivores.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 05-21-2017
//////////////////////////////////////////////////////////////////////////

#include <algorithm> // for std::max()
#include <cmath>   // for exp() and pow()
#include <memory>  // for std::auto_ptr
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
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
