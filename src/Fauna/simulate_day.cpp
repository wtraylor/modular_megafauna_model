// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Function object to perform simulations in the herbivore model.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "simulate_day.h"
#include "feed_herbivores.h"
#include "habitat.h"
#include "herbivore_interface.h"
#include "population_interface.h"
#include "simulation_unit.h"

using namespace Fauna;

//============================================================
// SimulateDay
//============================================================

SimulateDay::SimulateDay(const int day_of_year, SimulationUnit& simulation_unit,
                         const FeedHerbivores& feed_herbivores)
    : day_of_year(day_of_year),
      environment(simulation_unit.get_habitat().get_environment()),
      feed_herbivores(feed_herbivores),
      herbivores(get_herbivores(simulation_unit.get_populations())),
      simulation_unit(simulation_unit) {}

void SimulateDay::create_offspring() {
  for (const auto& itr : total_offspring) {
    PopulationInterface* pop = itr.first;
    const double offspring = itr.second;
    if (offspring > 0.0) pop->create_offspring(offspring);
  }
}

HabitatForage SimulateDay::get_corrected_forage(const Habitat& habitat) {
  // available forage in the habitat [kgDM/km²]
  HabitatForage available_forage = habitat.get_available_forage();
  static const double NEGLIGIBLE_FORAGE_MASS = 10000;  // [kgDM/km²] ٍ= 10 g/m²
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++)
    if (available_forage[*ft].get_mass() <= NEGLIGIBLE_FORAGE_MASS) {
      available_forage[*ft].set_nitrogen_mass(0.0);
      available_forage[*ft].set_mass(0.0);
    }
  return available_forage;
}

std::map<PopulationInterface*, HerbivoreVector> SimulateDay::get_herbivores(
    const PopulationList& pop_list) {
  std::map<PopulationInterface*, HerbivoreVector> result;
  for (auto& pop : pop_list) {
    assert(pop);
    result[pop.get()] = pop->get_list();
  }
  return result;
}

void SimulateDay::operator()(const bool do_herbivores,
                             const bool establish_as_needed) {
  if (day_of_year < 0 || day_of_year >= 365)
    throw std::invalid_argument(
        "SimulateDay::operator()() "
        "Argument 'day_of_year' out of range");

  // pass the current date into the herbivore module
  simulation_unit.get_habitat().init_day(day_of_year);

  if (do_herbivores) {
    // Kill herbivore populations below the minimum density threshold here
    // so that simulate_herbivores() can (potentially) return nutrients from
    // the dead bodies before the herbivore objects are removed from memory in
    // purge_of_dead() below.
    for (auto& pop : simulation_unit.get_populations()) pop->kill_nonviable();

    if (establish_as_needed) {
      for (auto& pop : simulation_unit.get_populations())
        if (pop->get_list().empty()) pop->establish();
      simulation_unit.set_initial_establishment_done();
    }

    simulate_herbivores();

    // Construct list of *all* herbivores.
    // FeedHerbivores expects a list of herbivore pointers, so we concatenate
    // the population-separated lists here. This is an unelegant and wasteful
    // solution...
    int total_count = 0;
    for (auto itr : herbivores) total_count += itr.second.size();
    HerbivoreVector all_herbivores;
    all_herbivores.reserve(total_count);
    for (auto itr : herbivores)
      all_herbivores.insert(all_herbivores.end(), itr.second.begin(),
                            itr.second.end());

    // FEEDING
    const auto forage_before_feeding =
        get_corrected_forage(simulation_unit.get_habitat());
    auto available_forage = forage_before_feeding;
    feed_herbivores(available_forage, all_herbivores);
    // remove the eaten forage
    simulation_unit.get_habitat().remove_eaten_forage(
        forage_before_feeding.get_mass() - available_forage.get_mass());
  }

  // Now we will change the populations, and the herbivore pointers could
  // become invalid. So we need to delete the pointers to be sure that
  // nobody uses those pointers.
  herbivores.clear();

  create_offspring();

  for (auto& pop : simulation_unit.get_populations()) pop->purge_of_dead();
}

void SimulateDay::simulate_herbivores() {
  // loop through all herbivores: simulate
  for (auto& itr : herbivores) {
    PopulationInterface* pop = itr.first;
    for (auto& herbivore : itr.second) {
      // If this herbivore is dead, just skip it. The Population object will
      // take care of releasing its memory.
      if (herbivore->is_dead()) {
        continue;
      }

      // ---------------------------------------------------------
      // HERBIVORE SIMULATION

      // Offspring by this one herbivore today
      // [ind/km²]
      double offspring = 0.0;

      // Let the herbivores do their simulation.
      herbivore->simulate_day(day_of_year, environment, offspring);

      // Gather the offspring.
      total_offspring[pop] += offspring;
    }
  }
}
