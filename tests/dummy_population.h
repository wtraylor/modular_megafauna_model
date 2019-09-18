/**
 * \file
 * \brief Population implementation for unit tests.
 * \copyright ...
 * \date 2019
 */
#ifndef TESTS_DUMMY_POPULATION_H
#define TESTS_DUMMY_POPULATION_H

#include "dummy_herbivore.h"
#include "population_interface.h"

namespace Fauna {
class Hft;

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
    vec.push_back(DummyHerbivore(hft, hft->establishment.density));
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
}  // namespace Fauna

#endif  // TESTS_DUMMY_POPULATION_H
