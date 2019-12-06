/**
 * \file
 * \brief Function object to perform simulations in the herbivore model.
 * \copyright ...
 * \date 2019
 */
#include "simulate_day.h"
#include "feed_herbivores.h"
#include "habitat.h"
#include "herbivore_interface.h"
#include "population_interface.h"
#include "population_list.h"
#include "simulation_unit.h"

using namespace Fauna;

//============================================================
// SimulateDay
//============================================================

SimulateDay::SimulateDay(const int day_of_year, SimulationUnit& simulation_unit,
                         const FeedHerbivores& feed_herbivores)
    : day_of_year(day_of_year),
      excreted_nitrogen(0.0),
      environment(simulation_unit.get_habitat().get_environment()),
      feed_herbivores(feed_herbivores),
      forage_before_feeding(
          get_corrected_forage(simulation_unit.get_habitat())),
      herbivores(simulation_unit.get_populations().get_all_herbivores()),
      simulation_unit(simulation_unit) {}

void SimulateDay::create_offspring() {
  for (std::map<const Hft*, double>::iterator itr = total_offspring.begin();
       itr != total_offspring.end(); itr++) {
    const Hft* hft = itr->first;
    const double offspring = itr->second;
    if (offspring > 0.0)
      simulation_unit.get_populations().get(*hft).create_offspring(offspring);
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

void SimulateDay::operator()(const bool do_herbivores) {
  if (day_of_year < 0 || day_of_year >= 365)
    throw std::invalid_argument(
        "SimulateDay::operator()() "
        "Argument 'day_of_year' out of range");

  // pass the current date into the herbivore module
  simulation_unit.get_habitat().init_day(day_of_year);

  if (do_herbivores) {
    // Kill herbivore populations below the minimum density threshold here
    // so that simulate_herbivores() can take the nitrogen back before
    // the herbivore objects are removed from memory in purge_of_dead()
    // below.
    simulation_unit.get_populations().kill_nonviable();

    simulate_herbivores();

    // FEEDING
    HabitatForage available_forage = forage_before_feeding;
    feed_herbivores(available_forage, herbivores);
    // remove the eaten forage
    simulation_unit.get_habitat().remove_eaten_forage(
        forage_before_feeding.get_mass() - available_forage.get_mass());
  }

  simulation_unit.get_habitat().add_excreted_nitrogen(excreted_nitrogen);

  // Now we will change the populations, and the herbivore pointers could
  // become invalid. So we need to delete the pointers to be sure that
  // nobody uses those pointers.
  herbivores.clear();

  create_offspring();

  simulation_unit.get_populations().purge_of_dead();
}

void SimulateDay::simulate_herbivores() {
  // loop through all herbivores: simulate
  for (HerbivoreVector::iterator itr_h = herbivores.begin();
       itr_h != herbivores.end(); itr_h++) {
    HerbivoreInterface& herbivore = **itr_h;

    // If this herbivore is dead, just take all of its nitrogen and
    // skip it. The Population object will take care of releasing its
    // memory.
    if (herbivore.is_dead()) {
      excreted_nitrogen += herbivore.take_nitrogen_excreta();
      continue;
    }

    // ---------------------------------------------------------
    // HERBIVORE SIMULATION

    // Offspring by this one herbivore today
    // [ind/km²]
    double offspring = 0.0;

    // Let the herbivores do their simulation.
    herbivore.simulate_day(day_of_year, environment, offspring);

    // Gather the offspring.
    total_offspring[&herbivore.get_hft()] += offspring;

    // Gather nitrogen excreta.
    excreted_nitrogen += herbivore.take_nitrogen_excreta();
  }
}