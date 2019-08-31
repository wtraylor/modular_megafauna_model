#include "population_list.h"
#include "herbivore.h"
#include "hft.h"
#include "population.h"

using namespace Fauna;

void PopulationList::add(PopulationInterface* new_pop) {
  if (new_pop == NULL)
    throw std::invalid_argument(
        "Fauna::PopulationList::add(): "
        "Parameter new_pop is NULL.");

  const Hft& hft = new_pop->get_hft();

  if (exists(hft))
    throw std::invalid_argument(
        "Fauna::PopulationList::add(): "
        "A population of HFT \"" +
        hft.name + "\" already exists.");

  list.emplace_back(new_pop);
}

void PopulationList::establish(const HftList& hftlist){
  // TODO
}

bool PopulationList::exists(const Hft& hft) const {
  for (const auto& itr : list)
    if (itr->get_hft() == hft) return true;
  return false;
}

PopulationInterface& PopulationList::get(const Hft& hft) {
  for (const auto& itr : list)
    if (itr->get_hft() == hft) return *itr;
  throw std::invalid_argument(
      "Fauna::PopulationList::get() "
      "There is no population for HFT '" +
      hft.name + "' in the list.");
}

HerbivoreVector PopulationList::get_all_herbivores() {
  std::vector<HerbivoreInterface*> result;
  result.reserve(1024);  // Reserve plenty of space for herbivore pointers.

  for (auto& pop : list) {
    const std::vector<HerbivoreInterface*> v = pop->get_list();
    result.reserve(result.size() + v.size());
    for (auto& h : v)
      if (!h->is_dead()) result.push_back(h);
  }

  return result;
}

ConstHerbivoreVector PopulationList::get_all_herbivores()
    const {
  std::vector<const HerbivoreInterface*> result;
  result.reserve(1024);  // Reserve plenty of space for herbivore pointers.

  for (const auto& pop : list) {
    const PopulationInterface& p = *pop;  // Specify *const* object.
    const ConstHerbivoreVector v = p.get_list();
    result.reserve(result.size() + v.size());
    for (const auto& h : v)
      if (!h->is_dead()) result.push_back(h);
  }
  return result;
}

void PopulationList::kill_nonviable(){
  for (auto& pop : list) {
    // If the population’s density is below minimum, mark all
    // herbivores as dead.
    const double min_ind_per_km2 = pop->get_hft().minimum_density_threshold *
                                   pop->get_hft().establishment_density;
    if (pop->get_ind_per_km2() < min_ind_per_km2) pop->kill_all();
  }
}

void PopulationList::purge_of_dead() {
  for (auto& itr : list) itr->purge_of_dead();
}
